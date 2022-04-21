/* 
 * Operating Systems  (2INCO)  Practical Assignment
 * Interprocess Communication
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>      // for perror()
#include <unistd.h>     // for getpid()
#include <mqueue.h>     // for mq-stuff
#include <time.h>       // for time()
#include <complex.h>

#include "common.h"
#include "md5s.h"

static void rsleep (int t);

static char *mq_request;
static char *mq_response;

int main (int argc, char * argv[])
{
    // First we check if the correct amount of arguments are supplied. 
    // argv[0] is the name of the program, argv[1] should be the request queue, and argv[2] should be the response queue
    // Therefore, argc = 3 should hold 
    if (argc != 3) {
        int i; 
        fprintf (stderr, "%s: %d arguments:\n", argv[0], argc);
        for (i = 1l; i < argc; i++) {
            fprintf(stderr, "     '%s'\n", argv[i]);
        }
        exit(1);
    }
    // If argc = 3, then we can parse the arguments.

    // Type declarations
    mqd_t               mq_fd_request;
    mqd_t               mq_fd_response;
    MQ_REQUEST_MESSAGE  req;
    MQ_RESPONSE_MESSAGE rsp;

    // Gets the message queue names from the arguments.
    mq_request = argv[1];
    mq_response = argv[2];

    // Now we need to open the request (read only) and response queue (write only)
    mq_fd_request = mq_open(mq_request, O_RDONLY);
    mq_fd_response = mq_open(mq_response, O_WRONLY);

    // Repeatingly (until there are no more tasks to do): 
    while(true) { 

        // Read from the request queue the new job to do
        mq_receive (mq_fd_request, (char *) &req, sizeof (req), NULL);
        
        // If we get the special value !, then the farmer tells the worker there are no more jobs to be done. 
        if(req.job == '!') {
            break;
        }

        // Wait a random amount of time after getting a request
        rsleep(10000);

        // Now we can start with the bruteforce. First we need some declarations for the start and end of the alphabet
        char start_alphabet = req.start;             
        char end_alphabet = req.end;
        char password_nojob[MAX_MESSAGE_LENGTH - 1];            // password_nojob is a string that will later be used in generating a new password to try.
        for (int i = 0; i < MAX_MESSAGE_LENGTH - 1; i++) {      // So password_nojob starts out as 'aaaaa'.
            password_nojob[i] = start_alphabet;
        }
        
        int try_for_hash = 1;                                   // Will be used as a boolean: if password has been found or if all possible configurations have been
                                                                //    tried for the given hash, change to 0 such that a new hash value can be received.
        int current_length = 0;                                 // Length of the helper word password_nojob. Starts of as 0 since in the beginning the password we 
                                                                //    want to check consists only of the job value, which is not included in password_nojob.

        // Will go on until either correct password has been found, or this job cannot find the password.
        while (try_for_hash) {
            char password[current_length + 2];                  // Password is the actual password that will be used to calculate the MD5 value.
            password[0] = req.job;                              // First value of the password is always the job value.
            for (int i = 1; i < current_length + 1; i++) {      
                password[i] = password_nojob[i - 1];            // Uses the password_nojob string in order to construct a password to calculate the MD5 value.
            }
            
            password[current_length + 1] = '\0';                // Strings in C always end with this value

            // Calculate the MD5 hash for the generated password
            uint128_t try_hash;
            try_hash = md5s(password, current_length + 1);

//--------------------------------------------------------------------------------------------------------------------------------------------------------------
            // Fix for not checking certain passwords. Read pdf file as to why this section is included (line 104-128). 
            // No further comments will be given in this section, since a similar procedure is going on right above and below line 104-128. 
            char password_a[current_length + 3];
            password_a[0] = req.job;
            for (int i = 1; i < current_length + 1; i++) {
                password_a[i] = password_nojob[i - 1];
            }
            password_a[current_length + 1] = start_alphabet;    // The start alphabet character is skipped in the generating of a new password_nojob, so here we 
                                                                //   manually try this password.
            password_a[current_length + 2] = '\0';
            uint128_t try_hash_a;
            try_hash_a = md5s(password_a, current_length + 2);

            if (try_hash_a == req.md5_value) {

                for (int i = 0; i < MAX_MESSAGE_LENGTH; i++) {
                    rsp.match[i] = password_a[i];
                }
                rsp.index = req.index;
                
                mq_send(mq_fd_response, (char *) &rsp, sizeof (rsp), 0);
                break;
            }
//--------------------------------------------------------------------------------------------------------------------------------------------------------------

            // Compare generated password with hash obtained from farmer and take appropriate action (send or do not send).
            if (try_hash == req.md5_value) {
                for (int i = 0; i < MAX_MESSAGE_LENGTH; i++) {  // the correctly calculated password is used to form rsp.match of the response queue data structure.
                    rsp.match[i] = password[i];
                }
                rsp.index = req.index;
                
                // Send the correctly found password to the response queue
                mq_send(mq_fd_response, (char *) &rsp, sizeof (rsp), 0);

                try_for_hash = 0;
                break;
            }

            if (current_length == 0) {
                current_length++;
            }

            // Generate a new password_nojob
            for (int i = current_length - 1; i >= 0; i--) {
                // If the helper word password_nojob is not yet at the end of the alphabet, increment
                if ((int) password_nojob[i] < (int) end_alphabet) {              
                    password_nojob[i] = (char) ((int) password_nojob[i] + 1);
                    break;

                // If the helper word password_nojob is at the end of the alphabet, reset it.
                } else if ((int) password_nojob[i] == (int) end_alphabet) {     
                    password_nojob[i] = start_alphabet;
                }
                // Increase the length of the helper word password_nojob by 1 if all configurations of previous length have been checked.
                if((int) password_nojob[0] == (int) start_alphabet && i == 0) { 
                    current_length++;
                    break;
                }
            }

            // If we go over the max possible length, then break; apparently for this job and hash, a correct password can not be found, so receive a new job. 
            if (current_length + 1 > MAX_MESSAGE_LENGTH) {
                try_for_hash = 0;
                break;
            }
        }
    }
    // Close the message queues.
    mq_close(mq_fd_request);
    mq_close(mq_fd_response);

    // Unlink message queues.
    mq_unlink(mq_request);
    mq_unlink(mq_response);

    return (0);
}

/*
 * rsleep(int t)
 *
 * The calling thread will be suspended for a random amount of time
 * between 0 and t microseconds
 * At the first call, the random generator is seeded with the current time
 */
static void rsleep (int t)
{
    static bool first_call = true;
    
    if (first_call == true)
    {
        srandom (time (NULL) % getpid ());
        first_call = false;
    }
    usleep (random() % t);
}
