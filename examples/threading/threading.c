#include "threading.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>


pthread_mutex_t threadMutex;
// Optional: use these functions to add debug or error prints to your application
#define DEBUG_LOG(msg,...)
//#define DEBUG_LOG(msg,...) printf("threading: " msg "\n" , ##__VA_ARGS__)
#define ERROR_LOG(msg,...) printf("threading ERROR: " msg "\n" , ##__VA_ARGS__)

void* threadfunc(void* thread_param)
{
    struct thread_data* thread_args = (struct thread_data*)thread_param;
    
    usleep(thread_args->wait_to_obtain_ms);

    pthread_mutex_lock(thread_args->threadMutex);

    usleep(thread_args->wait_to_release_ms);

    pthread_mutex_unlock(thread_args->threadMutex);

    thread_args->thread_complete_success = true;

    // TODO: wait, obtain mutex, wait, release mutex as described by thread_data structure
    // hint: use a cast like the one below to obtain thread arguments from your parameter
    //struct thread_data* thread_func_args = (struct thread_data *) thread_param;
    return thread_param;
}


bool start_thread_obtaining_mutex(pthread_t *thread, pthread_mutex_t *mutex,int wait_to_obtain_ms, int wait_to_release_ms)
{
    struct thread_data* threadArgs = malloc(sizeof(struct thread_data));
    if (threadArgs == NULL){
       return false;
    }

    threadArgs->threadMutex = mutex;
    threadArgs->wait_to_obtain_ms = wait_to_obtain_ms;
    threadArgs->wait_to_release_ms = wait_to_release_ms;
    threadArgs->thread_complete_success = false;
       
    int res = pthread_create(thread, NULL, threadfunc, threadArgs);

    if(res != 0){
        free(threadArgs);
        return false; 
    }
    /**
     * TODO: allocate memory for thread_data, setup mutex and wait arguments, pass thread_data to created thread
     * using threadfunc() as entry point.
     *
     * return true if successful.
     *
     * See implementation details in threading.h file comment block
     */
    return true;
}

