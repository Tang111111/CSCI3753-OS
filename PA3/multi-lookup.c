#include "multi-lookup.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "util.h"
#include "queue.h"





void* RequestThread(void *arg){

    struct ThreadFile * request;
    request=(struct ThreadFile *) arg;
    int filePosition;
    char hostname[MAX_DOMAIN_NAME_LENGTH];
    char *allocSpace;
    FILE* servicedfp=request->serviced; //File pointer for the requester_log.txt
    FILE* inputFilePointer=NULL; //File pointer for the five input files

    pthread_mutex_t *buffer_mutex=request->buffer_mutex; //mutex for the shared buffer
    pthread_mutex_t *service_mutex=request->service_mutex; //mutex for the requester_log.txt
    pthread_mutex_t *position_mutex=request->position_mutex; //mutex for later counting the position in the while loop
    queue *shared_buffer=request->buffer; 

    pthread_cond_t *c_cons=request->c_cons;  //condition variable for comsumer(resolver)
    pthread_cond_t *c_prod=request->c_prod;  //condition variable for producer(requester)

    int thread_id = pthread_self();

    int endOfFile = request->totalFile;// pointer pointing to the end of file
    
    int numberservicedFile=0;  //counting how many files have been serviced

    while(*(request->filePosition)<endOfFile){

        pthread_mutex_lock(position_mutex); //set a mutex here to allow only one thread to add its filePosition once
        filePosition=*(request->filePosition);
        filePosition++;
        *(request->filePosition)=filePosition;
        if(filePosition>endOfFile){
            break;  //have serviced all the files
        }
        pthread_mutex_unlock(position_mutex);

        inputFilePointer=request->requesterFile[filePosition-1];

            while(fscanf(inputFilePointer, INPUTFS, hostname)>0){ //thread read the assigned file
            allocSpace=malloc(MAX_DOMAIN_NAME_LENGTH);
            strncpy(allocSpace,hostname,MAX_DOMAIN_NAME_LENGTH);
            pthread_mutex_lock(buffer_mutex); //enter the critical section for writing to the shared buffer


            while(queue_is_full(shared_buffer)){ 
                pthread_cond_wait(c_prod,buffer_mutex); //suspend the current thread,unlock the mutex until get signaled
            }
            
            queue_push(shared_buffer,allocSpace);
            pthread_mutex_unlock(buffer_mutex); //leave the critical section
            pthread_cond_signal (c_cons); //signal the cousumer CV that shared buffer is not empty
        }
        numberservicedFile++;
        
    }
    pthread_mutex_lock(service_mutex);
    fprintf(servicedfp, "Thread %d serviced %d files \n", thread_id, numberservicedFile);  //write result to the requester_log.txt
    pthread_mutex_unlock(service_mutex);
    return NULL;

}

void* ResolverThread(void *arg){
    struct ThreadFile * requireData;
    requireData=(struct ThreadFile *) arg;

    char *hostname;
    char *ipAddress;

    FILE *resultsfp=requireData->results; //File pointer to the results.txt

    queue* shared_buffer=requireData->buffer; //shared buffer

    pthread_mutex_t *buffer_mutex=requireData->buffer_mutex; //mutex for shared buffer
    pthread_mutex_t *output_mutex=requireData->output_mutex; //mutex for the results.txt

    pthread_cond_t *c_cons=requireData->c_cons;
    pthread_cond_t *c_prod=requireData->c_prod;

    while(!queue_is_empty(shared_buffer)||*(requireData->flag)) { //when requester threads have been terminated and the shared buffer is empty, jump out of the loop
        pthread_mutex_lock (buffer_mutex); //enter the critical section for reading the shared buffer
        while(queue_is_empty(shared_buffer)){
            pthread_cond_wait(c_cons,buffer_mutex); //suspend the current thread,unlock the mutex until get signaled
        }

        hostname=queue_pop(shared_buffer);

        pthread_mutex_unlock (buffer_mutex); //leave the critical section

        pthread_cond_signal (c_prod); //signal the producer that shared is not full

        ipAddress=malloc(MAX_IP_LENGTH);

        if(dnslookup(hostname,ipAddress,MAX_IP_LENGTH)==UTIL_FAILURE){
            printf("The hostname is %s\n",hostname);
            strncpy(ipAddress, "", MAX_IP_LENGTH);
        }
        pthread_mutex_lock(output_mutex);
        fprintf(resultsfp, "%s,%s\n", hostname, ipAddress);  //write results to the results.txt
        pthread_mutex_unlock(output_mutex);  
        free(hostname);
        free(ipAddress);
        
}
    return NULL;


}


int main(int argc,char *argv[]){  

    struct timeval begin, end;
    gettimeofday(&begin, NULL);


    int requester_threads=atoi(argv[1]);
    int resolver_threads=atoi(argv[2]);
    int numberOfFile=argc-5;

    int unfinishedFlag=1; //set 0 when request threads have all completed


    pthread_mutex_t buffer_mutex;  //mutex for shared buffer
    pthread_mutex_t output_mutex;  //mutex for writing results.txt
    pthread_mutex_t service_mutex; //mutex for writing serviced.txt
    pthread_mutex_t position_mutex; //mutex for counting current position of the file pointer

    pthread_mutex_init(&buffer_mutex,NULL);
    pthread_mutex_init(&output_mutex,NULL);
    pthread_mutex_init(&service_mutex,NULL);
    pthread_mutex_init(&position_mutex,NULL);

    pthread_cond_t c_cons; //CV for consumer
    pthread_cond_t c_prod; //CV for producer

    pthread_cond_init(&c_cons,NULL);
    pthread_cond_init(&c_prod,NULL);

    if(argc<6){
        fprintf(stderr, "Not enough arguments, you should provide at least 6 arguments");
        return EXIT_FAILURE;
    }
    if(numberOfFile>MAX_INPUT_FILES){
        fprintf(stderr, "Too many files: %d\n", (argc - 5));
        return EXIT_FAILURE;
    }
    if(requester_threads>MAX_REQUESTOR_THREADS){
        fprintf(stderr, "Too many requester threads, it should not be greater than: %d\n", MAX_REQUESTOR_THREADS);
        return EXIT_FAILURE;
    }
    if(resolver_threads>MAX_RESOLVER_THREADS){
        fprintf(stderr, "Too many resolver threads, it should not be greater than: %d\n", MAX_RESOLVER_THREADS);
        return EXIT_FAILURE;
    }


    /*Create requesr thread pool*/

    struct ThreadFile request;
    pthread_t requestThreads[requester_threads];  //requester thread
    int thread_create_code;
    FILE* servicedFilePointer = NULL;  //File pointer for requester_log.txt

    servicedFilePointer= fopen(argv[3], "w");
    if (!servicedFilePointer)
    {
        perror("Error opening output file.\n");
        return EXIT_FAILURE;
    }

    request.serviced=servicedFilePointer;
    
    FILE* readfile=NULL;
    int errorFile=0;  //count the number of wrong input files
    int counter=0; //count the number of valid files
    for(int i=0;i<numberOfFile;i++){
        readfile=fopen(argv[i+5],"r"); //read input file to file pointers of request threads
        
        if(readfile==NULL){
            printf("The worng file path is %s\n",argv[i+5] );
            perror("Error opening input file,might be caused by the wrong file path \n");
            errorFile++;
        }
        else{
            request.requesterFile[counter++]=readfile;
        }
        
    }

    request.service_mutex=&service_mutex;
    request.buffer_mutex=&buffer_mutex;
    request.output_mutex=&output_mutex;
    request.position_mutex=&position_mutex;
    request.c_cons=&c_cons;
    request.c_prod=&c_prod;

    int filePosition=0;
    int totalFile=numberOfFile-errorFile;

    request.filePosition=&filePosition;
    request.totalFile=totalFile;



    queue shared_buffer;

    if(queue_init(&shared_buffer, QUEUEMAXSIZE) == QUEUE_FAILURE){ //queue max size is 20
        fprintf(stderr, "error: queue_init failed!\n");
    }

    request.buffer=&shared_buffer;

    
    for(int i=0;i<requester_threads;i++){
        thread_create_code=pthread_create(&requestThreads[i],NULL,RequestThread,&request);
        if(thread_create_code){
            printf("ERROR; return code from pthread_create() is %d\n", thread_create_code);
            exit(EXIT_FAILURE);
        }
    }


    /*Create resolve thread pool*/

    struct ThreadFile resolver;
    pthread_t resolverThreads[resolver_threads];
    int thread_create_code2;
    FILE* resultsFilePointer=NULL;  //File pointer for results.txt

    resultsFilePointer = fopen(argv[4], "w");
    if (!resultsFilePointer)
    {
        perror("Error opening output file.\n");
        return EXIT_FAILURE;
    }

    resolver.results=resultsFilePointer;
    resolver.buffer=&shared_buffer;
    resolver.service_mutex=&service_mutex;
    resolver.buffer_mutex=&buffer_mutex;
    resolver.output_mutex=&output_mutex;
    resolver.c_cons=&c_cons;
    resolver.c_prod=&c_prod;
    resolver.flag=&unfinishedFlag;

    for(int i=0;i<resolver_threads;i++){
        thread_create_code2=pthread_create(&resolverThreads[i],NULL,ResolverThread,&resolver);
        if (thread_create_code2){
            printf("ERROR; return code from pthread_create() is %d\n", thread_create_code2);
            exit(EXIT_FAILURE);
        }
    }
    

    for (int i=0; i<requester_threads; i++)
    {
        pthread_join(requestThreads[i], NULL);
    }

    unfinishedFlag=0; //set the flag that all the request threads has finished

    for (int i=0; i<resolver_threads; i++)
    {
        pthread_join(resolverThreads[i], NULL);
    }

    fclose(servicedFilePointer);
    fclose(resultsFilePointer);


    for(int i=0;i<counter;i++){
        fclose(request.requesterFile[i]);
    }

    pthread_mutex_destroy(&buffer_mutex);
    pthread_mutex_destroy(&output_mutex);
    pthread_mutex_destroy(&service_mutex);

    pthread_cond_destroy(&c_prod);
    pthread_cond_destroy(&c_cons);

    queue_cleanup(&shared_buffer);

    gettimeofday(&end, NULL);
    long long time = 1000000 * (end.tv_sec - begin.tv_sec) + (end.tv_usec - begin.tv_usec);
    printf("The program took %lld milliseconds to run the program.\n", time);
    printf("Program exiting succesfully.\n");
    return 0;

    
}
