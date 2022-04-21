# MD5-hash-value-checker
Given a MD5 hash value, the application generates all possible strings and calculates their MD5 hash values, until it finds a match.
<br>
<br>

## Compile & run

Compile the application using following commands: 

    $ gcc -Wall -o farmer farmer.c -lrt
    $ gcc -Wall -o worker worker.c md5s.c -lrt -lm

After this, run the program using the following command:

    $ ./farmer
