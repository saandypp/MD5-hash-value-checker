/* 
 * Operating Systems (2INCO) Practical Assignment
 * Interprocess Communication
 *
 * Contains definitions which are commonly used by the farmer and the workers
 *
 */

#ifndef COMMON_H
#define COMMON_H

#include "uint128.h"

// Maximum size for any message in the tests
#define MAX_MESSAGE_LENGTH	6
 
// Data structure representing the request queue
typedef struct
{
    char                      start;
    char                      end;
    char                      job;
    uint128_t                 md5_value;
    int                       index;
} MQ_REQUEST_MESSAGE;

// Data structure representing the response queue
typedef struct
{
    char                      match[MAX_MESSAGE_LENGTH + 1];
    int                       index;
} MQ_RESPONSE_MESSAGE;


#endif

