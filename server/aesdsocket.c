//Program : Aesdsocket program for assignment 5 part 1
//Author : Amogh Sadhu
/* =========================================================================
   1. HEADER INCLUDES
   ========================================================================= */
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <syslog.h>
#include <arpa/inet.h> 
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>
#include <time.h>

#include "linkeListLib.h"

/* =========================================================================
   2. PREPROCESSOR DEFINES & MACROS
   ========================================================================= */
#define PORT 9000
#define BUFFER_SIZE 1024

/* =========================================================================
   3. TYPE DEFINITIONS
   ========================================================================= */
typedef struct threadData
{
    pthread_t threadId;
    bool threadCompleteFlag;
    pthread_mutex_t* threadMutex;
    int accpetedSocket;
    struct sockaddr_in socketAddress;
}threadData_t;

/* =========================================================================
   4. GLOBAL VARIABLES
   ========================================================================= */
volatile sig_atomic_t keep_running  = 1;     //will edit this in signal handling part

pthread_mutex_t threadMutex;

const char* fileName = "/var/tmp/aesdsocketdata";

/* =========================================================================
   5. STATIC VARIABLES
   ========================================================================= */


/* =========================================================================
   6. STATIC FUNCTION PROTOTYPES
   ========================================================================= */
static void signalHandlerShared(int sig);
static void* threadfunc(void * threadData);
static Node* checkJoinAndPruneThreads(Node* head);
static void* timerThreadFunc(void* arg);

/* =========================================================================
   7. MAIN FUNCTION
   ========================================================================= */
int main(int argc, char* argv[]){

    //Setting up the signals
    struct sigaction sa;
    memset(&sa,0,sizeof(sa));
    sa.sa_handler = signalHandlerShared;
    sa.sa_flags = 0;

    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    
    // Socket related declarations
    int socketFd, newSocket;
    struct sockaddr_in address;
    struct sockaddr_in addressClient;

    // Filling the address structure for server
    address.sin_port = htons(PORT); 
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;

    socklen_t socketLength = sizeof(address);
    socklen_t socketLengthClient = sizeof(addressClient);

    //Setting up the mutex and  thread
    pthread_t timerThread;
    pthread_mutex_init(&threadMutex, NULL);
    Node* head = NULL;

    remove(fileName);

    openlog("aesdsocket", LOG_PID, LOG_USER);

    if((socketFd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        return -1;
    }

    int opt = 1;
    if(setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        return -1;  
    }

    if(bind(socketFd, (struct sockaddr*)&address, socketLength) < 0){
        return -1;
    }

    if(listen(socketFd, 3) < 0){
        return -1;
    }

    if (argc > 1 && strcmp(argv[1], "-d") == 0) {
    pid_t pid = fork();
    if (pid < 0) return -1; // Fork failed
    if (pid > 0) exit(0);   // Parent exits, child continues
    }

    //Timer thread Implementation
    if((pthread_create(&timerThread, NULL, timerThreadFunc, NULL)) != 0){
            printf("Failed to create timer \n");
        }

    while(keep_running){        // While loop to continuously waiting for connections
        socketLengthClient = sizeof(addressClient);    //update the length  

        if((newSocket = accept(socketFd, (struct sockaddr*)&addressClient, &socketLengthClient)) < 0){
            if(keep_running == 0) break;
            else continue;
        }
        syslog(LOG_INFO, "Accepted connection from %s",inet_ntoa(addressClient.sin_addr));

        // Starting thead from here
        threadData_t* threadData;
        threadData = (threadData_t*)malloc(sizeof(threadData_t));
        threadData->accpetedSocket = newSocket;
        threadData->socketAddress = addressClient;
        threadData->threadMutex = &threadMutex;
        threadData->threadCompleteFlag = false;

        if((pthread_create(&(threadData->threadId), NULL, threadfunc, (void*)threadData)) == 0){
            head = insertAtBeginning(head, (void*)threadData);
        }
        else {
            free(threadData);
            close(newSocket);
        }
        head = checkJoinAndPruneThreads(head);
    }

    shutdown(socketFd,SHUT_RDWR);

    //Timer thread joining 
    pthread_join(timerThread, NULL);

    // Final sweep: Join every thread regardless of the flag
    while(head != NULL) {
        threadData_t* data = (threadData_t*)head->data;
        pthread_join(data->threadId,NULL);
        free(data);
        head = deleteNodeAtBeginning(head);
    }

    pthread_mutex_destroy(&threadMutex);
    remove(fileName);

    closelog();
return 0;
}

/* =========================================================================
   8. FUNCTION IMPLEMENTATIONS
   ========================================================================= */
//Signal handler
static void signalHandlerShared(int sig){
    switch(sig){
        case SIGINT:
            syslog(LOG_INFO, "Caught signal, exiting");
            keep_running = 0;
            break;

        case SIGTERM:
            syslog(LOG_INFO, "Caught signal, exiting");
            keep_running = 0;
            break;
    }
}

//Thread function which accepts void and takes void* as arg
static void* threadfunc(void * threadData){
    char *buffer = NULL;

    //file related declarations
    FILE *fp;    

    threadData_t* tInfo = (threadData_t*)threadData;
    int newSocket  = tInfo->accpetedSocket;
    struct sockaddr_in addressClient = tInfo->socketAddress;

    buffer = (char*)malloc(BUFFER_SIZE*sizeof(char));       //allocate the memory
    if(buffer == NULL){
        tInfo->threadCompleteFlag = 1;
        pthread_exit(NULL);
    }

    pthread_mutex_lock(tInfo->threadMutex);     //Lock the mutex here     
    fp = fopen(fileName, "a");
    if(fp != NULL){
        ssize_t bytesRecived;
        while((bytesRecived = recv(newSocket, buffer,BUFFER_SIZE, 0)) > 0){

            fwrite(buffer, sizeof(char), bytesRecived, fp); 
            if(memchr(buffer, '\n', bytesRecived) != NULL){
                break;
            }  
        }          
    fclose(fp);
    }
    pthread_mutex_unlock(tInfo->threadMutex);     //Unlock the mutex here 

    pthread_mutex_lock(tInfo->threadMutex);
    fp = fopen(fileName, "r");
    if(fp != NULL){
        int readBytes;
        while((readBytes = (fread(buffer, sizeof(char), BUFFER_SIZE, fp))) > 0){
            send(newSocket, buffer, readBytes, 0);
        }
    fclose(fp);
    }
    pthread_mutex_unlock(tInfo->threadMutex);     //Unlock the mutex here 

    shutdown(newSocket, SHUT_RDWR); 
    syslog(LOG_INFO, "Closed connection from %s", inet_ntoa(addressClient.sin_addr));
    free(buffer);
    tInfo->threadCompleteFlag = 1;

return NULL;
}

static Node* checkJoinAndPruneThreads(Node* head){
    Node* currentNode = head;
    Node* prev = NULL;

    while(currentNode != NULL){
        threadData_t* tInfo = (threadData_t*)currentNode->data;
        if(tInfo->threadCompleteFlag == 1){

            //Join the thread if complete
            pthread_join(tInfo->threadId, NULL);
            free(tInfo);

            Node* nodeToRemove = currentNode;
            if(prev == NULL){
                //Remove the head
                head = currentNode->next;
                currentNode = head;
            }
            else{
                prev->next = currentNode->next;
                currentNode = currentNode->next;
            }
        free(nodeToRemove);    
        }
        else{
            prev = currentNode;
            currentNode = currentNode->next;    
        }
    }
return head;
}

static void* timerThreadFunc(void* arg){
    while (keep_running)
    {
        for(int i= 0; i<10 && keep_running == 1; i++){
            sleep(1);
        }

        if(!keep_running) break;

        time_t rawTime;
        struct tm *info;
        char buffer[100];
        char finalBuffer[150];

        time(&rawTime);  
        info = localtime(&rawTime);   

        strftime(buffer,sizeof(buffer), "%a, %d %b %Y %T %z", info);
        int len = snprintf(finalBuffer, sizeof(finalBuffer), "timestamp:%s\n",buffer);

        pthread_mutex_lock(&threadMutex);
        FILE *fp = fopen(fileName, "a");
        if(fp != NULL){
            fwrite(finalBuffer, 1, len, fp);
            fclose(fp);
        }
        pthread_mutex_unlock(&threadMutex);
    }
return NULL;
}