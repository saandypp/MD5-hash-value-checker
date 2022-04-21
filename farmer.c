/* 
 * Operating Systems  (2INCO)  Practical Assignment
 * Interprocess Communication
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>    
#include <unistd.h>    // For execlp
#include <mqueue.h>    // For mq


#include "settings.h"  // Fefinition of work
#include "common.h"

#define STUDENT_NAME        "Sandy"

static char                 mq_request[80];
static char                 mq_response[80];

static void create_children(pid_t childrenPIDs[])
{
  int i;
  /* Start children. */
  for (i = 0; i < NROF_WORKERS; i++)
  {
    pid_t processID = fork(); // Fork
    if (processID < 0)
    {
      // If fork is unsuccesful
      perror("fork() failed");
      abort();
    }
    else if (processID == 0)
    {
      // Launch workers using execlp() with the two messege queue arguments
      execlp("./worker", "worker", mq_request, mq_response, NULL);
      perror("execlp() failed");
    }
    // Save children pids in order to see when they are finished
    childrenPIDs[i] = processID;
  }
}

// This function waits for all the children to finish
static void wait_all_children(pid_t childrenPIDs[])
{
  int i;
  for (i = 0; i < NROF_WORKERS; i++)
  {
    waitpid(childrenPIDs[i],NULL,0);
  }
}

int main (int argc, char * argv[])
{
    if (argc != 1)
    {
        fprintf (stderr, "%s: invalid arguments\n", argv[0]);
    }

    // TODO:
    //  * create the message queues (see message_queue_test() in
    //    interprocess_basic.c)
    mqd_t               mq_fd_request;
    mqd_t               mq_fd_response;
    MQ_REQUEST_MESSAGE  req;
    MQ_RESPONSE_MESSAGE rsp;
    struct mq_attr      attr;

    sprintf (mq_request, "/mq_request_%s_%d", STUDENT_NAME, getpid());
    sprintf (mq_response, "/mq_response_%s_%d", STUDENT_NAME, getpid());

    attr.mq_maxmsg  = MQ_MAX_MESSAGES;
    attr.mq_msgsize = sizeof (MQ_REQUEST_MESSAGE);
    mq_fd_request = mq_open (mq_request, O_WRONLY | O_CREAT | O_EXCL, 0600, &attr);

    attr.mq_maxmsg  = MQ_MAX_MESSAGES;
    attr.mq_msgsize = sizeof (MQ_RESPONSE_MESSAGE);
    mq_fd_response = mq_open (mq_response, O_RDONLY | O_CREAT | O_EXCL, 0600, &attr);
    //  * create the child processes (see process_test() and
    //    message_queue_test())
    pid_t childrenPIDs[NROF_WORKERS];//child pids array
    create_children(childrenPIDs);//creating children
    //  * do the farming
    int sent_msg = 0;                       // A counter to keep the sent messeges
    int received_msg = 0;                   // A counter to keep received messeges
    char start_char = ALPHABET_START_CHAR;  // start_char will be used to iterate over when assigning jobs
    char end_char = ALPHABET_END_CHAR;      // Final letter of the alphabet
    int md5_list_index = 0;                 // Index of the hash values

    // matching_strings is an array where the matching strings will be saved such that they can be printed in the correct order
    int numElements = sizeof(md5_list) / sizeof(md5_list[0]);
    char matching_strings[numElements][MAX_MESSAGE_LENGTH + MAX_MESSAGE_LENGTH + 1];

    while (sent_msg < JOBS_NROF || received_msg < numElements)
    {
        // Get attributes for request queue
        mq_getattr(mq_fd_request, &attr);
        
        // while queue is not full && there are still jobs to be done
        while (attr.mq_curmsgs < attr.mq_maxmsg && sent_msg < JOBS_NROF)
        {
            //send messages
            req.job = start_char;
            req.end = end_char;
            req.start = ALPHABET_START_CHAR;
            req.md5_value = md5_list[md5_list_index];
            req.index = md5_list_index;
            mq_send (mq_fd_request, (char *) &req, sizeof (req), 0);
 
            sent_msg++;         // Every time a message is send, this is incremented
            start_char++;       // Every time a message is send, the next job needs to be assigned, so increment through the alphabet

            // If we have iterated through all jobs for one hash value, then move on to next hash value
            if ((int)start_char > (int)end_char)
            {
                md5_list_index++;
                start_char = ALPHABET_START_CHAR;
            }
            mq_getattr(mq_fd_request, &attr);
        }
        // Get attributes for response queue
        mq_getattr(mq_fd_response, &attr);
        
        // While there are still jobs computed by the worker to be printed out
        while (attr.mq_curmsgs > 0)
        {
            // Receive finished job from worker
            mq_receive (mq_fd_response, (char *) &rsp, sizeof (rsp), NULL);
            received_msg++;

            // Put the password in the correct order, such that it can be printed out in the correct order later
            strcpy(matching_strings[rsp.index], rsp.match);
            
            mq_getattr(mq_fd_response, &attr);
        }
    }

    // print all matching strings; they are already in the correct order in matching_strings.
    int j;
    for (j = 0; j < numElements; j++)
    {
      if (strlen(matching_strings[j]) > MAX_MESSAGE_LENGTH) {
        matching_strings[j][MAX_MESSAGE_LENGTH] = '\0';
      }
      printf("'%s'\n", matching_strings[j]);
    }

    //  * wait until the chilren have been stopped (see process_test())
    int i;
    for (i = 0; i < NROF_WORKERS; i++)
    {
      req.job = '!';//send special value ! for the workers to break and exit
      mq_send (mq_fd_request, (char *) &req, sizeof (req), 0);
    }

    wait_all_children(childrenPIDs);

    //  * close and unlink the message queues (see message_queue_test())
    mq_close (mq_fd_response);
    mq_close (mq_fd_request);
    mq_unlink (mq_request);
    mq_unlink (mq_response);
    
    return (0);
}
