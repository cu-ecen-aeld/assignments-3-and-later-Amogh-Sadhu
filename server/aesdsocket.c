//Program : Aesdsocket program for assignment 5 part 1
//Author : Amogh Sadhu
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <syslog.h>
#include <arpa/inet.h> 
#include <signal.h>
#include <unistd.h>

#define PORT 9000
#define BUFFER_SIZE 1024

void signalHandlerShared(int sig);

volatile sig_atomic_t keep_running  = 1;     //will edit this in signal handling part

//Signal handler
void signalHandlerShared(int sig){
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


int main(int argc, char* argv[]){

    //Setting up the signals
    struct sigaction sa;
    memset(&sa,0,sizeof(sa));
    sa.sa_handler = signalHandlerShared;
    sa.sa_flags = 0;

    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    //file related declarations
    FILE *fp;
    const char* fileName = "/var/tmp/aesdsocketdata";    
    remove(fileName);

    // Socket related declarations
    int socketFd, newSocket;
    char *buffer = NULL;
    ssize_t bytesRecived;
    struct sockaddr_in address;
    struct sockaddr_in addressClient;

    // Filling the address structure for server
    address.sin_port = htons(PORT); 
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;

    socklen_t socketLength = sizeof(address);
    socklen_t socketLengthClient = sizeof(addressClient);

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

    buffer = (char*)malloc(BUFFER_SIZE*sizeof(char));       //allocate the memory
    while(keep_running){        // While loop to continuously waiting for connections
        socketLengthClient = sizeof(addressClient);    //update the length
        
        if(buffer == NULL) return -1;

        if((newSocket = accept(socketFd, (struct sockaddr*)&addressClient, &socketLengthClient)) < 0){
            if(keep_running == 0) break;
            else continue;
        }
        syslog(LOG_INFO, "Accepted connection from %s",inet_ntoa(addressClient.sin_addr));
 
        if((fp = fopen(fileName, "a")) == NULL){
            break;
        }
        while((bytesRecived = recv(newSocket, buffer,BUFFER_SIZE, 0)) > 0){

            fwrite(buffer, sizeof(char), bytesRecived, fp); 
            if(memchr(buffer, '\n', bytesRecived) != NULL){
                break;
            }            
            // char* newLine = memchr(buffer, '\n', bytesRecived);
            // if(newLine != NULL){
            //     int packetSize = newLine - buffer + 1;
            //     fwrite(buffer, sizeof(char), packetSize, fp);  
            //     break;       
            // }
            // else{
            //     fwrite(buffer, sizeof(char), bytesRecived, fp); 
            // }
        }
        fclose(fp);

        if((fp = fopen(fileName, "r")) == NULL){
            break;
        }

        int readBytes;
        while((readBytes = (fread(buffer, sizeof(char), BUFFER_SIZE, fp))) > 0){
            send(newSocket, buffer, readBytes, 0);
        }
        fclose(fp);

        shutdown(newSocket, SHUT_RDWR); 
        syslog(LOG_INFO, "Closed connection from %s", inet_ntoa(addressClient.sin_addr));
    }
    free(buffer);           //free the memory
    shutdown(socketFd,SHUT_RDWR);
    if(remove(fileName) != 0){
        return -1;
    }
    closelog();
return 0;
}
