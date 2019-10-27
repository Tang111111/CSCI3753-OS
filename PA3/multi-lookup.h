#ifndef MULTI_LOOKUP_H
#define MULTI_LOOKUP_H


#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>

#include "queue.h"

#define MAX_FILE_NUMBER 10
//#define MAX_BUFFER 20
#define MAX_DOMAIN_NAME_LENGTH 1025


//#define MAX_DOMAIN_NAME_LENGTH 1025
#define MAX_IP_LENGTH 46
#define MAX_INPUT_FILES  10
#define MAX_RESOLVER_THREADS 10
#define MAX_REQUESTOR_THREADS 10
#define INPUTFS "%1025s"


struct ThreadFile
{
    FILE* requesterFile[MAX_FILE_NUMBER]; //File pointer for input files
    FILE* results; //File pointer for results.txt
    FILE* serviced; //File pointer for requester_log.txt
    int *filePosition; //file pointer when reading the input files
    int totalFile; //total number of input files
    int *flag; //indicate whether all requester threads has been finished

    pthread_mutex_t* service_mutex; //mutex when writing to the requester_log.txt
    pthread_mutex_t* buffer_mutex;  //mutex when dealing with the shared buffer
    pthread_mutex_t* output_mutex;  //mutex when writing to results.txt
    pthread_mutex_t* position_mutex; //mutex for counting the current filr poistion
    pthread_cond_t* c_cons;  //CV for consumer
    pthread_cond_t* c_prod;  //CV for producer
    queue *buffer;  //shared buffer
};

void* RequestThread(void *arg);
void* ResolverThread(void *arg);


#endif