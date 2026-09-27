#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#define MAX_BUF_SIZE 2048
#define BACKLOG 100
#define MAX_NUMBER_OF_THREADS 100

typedef struct {
    int acceptedConnectionFD ;
    struct sockaddr_in addr ;
    char addrIpString[16];
} AcceptedConnection ;

AcceptedConnection accumulatedConnections[MAX_NUMBER_OF_THREADS] ;
int connectionsCpt = 0 ;
pthread_mutex_t locker = PTHREAD_MUTEX_INITIALIZER ;

int verify_and_parse_ip(char *ip_argument);
int verify_and_parse_port(char *port_argument);
int tcp_socket_connect(int ip, int port);
int tcp_socket_listen(int port);
void getLoaclIpAddrString(char * ipBuf);
int recvAndSendUsingGivenSocketClientSide(int connectionFD);
int recvAndBroadcastUsingGivenSocketServerSideThread(AcceptedConnection acceptedConnection);
AcceptedConnection acceptIncomingConnectionOnListeningSocket(int listeningSockedFD);
void * handleClient(void * acceptedConnectionPtr);
//============================================================
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <ifaddrs.h>

//returns the port as an int
int verify_and_parse_port(char * port_argument){
    long parsed_port ;
    char * endptr ;
    parsed_port = strtol(port_argument,&endptr,10);                 // instead of int port = atoi(argv[1]) to make more verifications 
    
    if (*endptr != '\0') {
        printf("Invalid Charachter in port input : %c \n" , *endptr);  
        exit(EXIT_FAILURE);
    } else if ((parsed_port < 1) || (parsed_port > 65535)) {
        printf("Invalid port value - Ports range 1-65535 \n");
        exit(EXIT_FAILURE);
    }

    return (int) parsed_port ;
}

//returns the ip as an int but ready to us direcly inside addr.sin_addr 
int verify_and_parse_ip(char * ip_argument){
    int state ;
    int ip ;
    if((state = inet_pton(AF_INET , ip_argument , &ip)) == 0 ){
        printf("Invalid Ip adress - Please enter a value in this form : 0-255.0-255.0-255.0-255 \n");
        exit(EXIT_FAILURE);
    }else if (state == -1) {
        perror("[-] Error parsing the Ip adress ");
        exit(EXIT_FAILURE);
    }
    return ip ;
}

//creates a ipv4 socket and and connect it to the given ip and port (port in little endien - ip in big endien)
int tcp_socket_connect(int ip , int port){
    int connection_fd ;
    if((connection_fd = socket(AF_INET, SOCK_STREAM , 0)) == -1 ){
        perror("[-] Error Creating the socket ");   //this one prints the msg u have, checks the value of errno (a var from errno.h) then find the string explanation linked to that value and prints it
        exit(EXIT_FAILURE);
    }
    printf("[*] Created connection socket \n");

    struct sockaddr_in adr ;
    adr.sin_family = AF_INET ;
    adr.sin_port = htons(port);
    adr.sin_addr.s_addr = ip ;

    if (connect(connection_fd , (struct sockaddr *)&adr , sizeof(adr) ) == -1 ) {
        perror("[-] Error establishing the connection ");
        close(connection_fd);
        exit(EXIT_FAILURE);
    }
    return connection_fd ;
}

//creates an ipv4 socket and let it listen at all interfaces on a given port with the REUSEADDR opt on (port in little endien)
int tcp_socket_listen(int port){
    int listenerSocket_fd ;
    if ((listenerSocket_fd = socket(AF_INET , SOCK_STREAM , 0)) == -1 ){
        perror("[-] Error Creating the socket ");   //this one prints the msg u have, checks the value of errno (a var from errno.h) then find the string explanation linked to that value and prints it
        exit(EXIT_FAILURE);
    }
    printf("[*] Created listener socket \n");
    
    int opt = 1 ;
    if (setsockopt(listenerSocket_fd , SOL_SOCKET , SO_REUSEADDR , &opt , sizeof(opt)) == -1 ){
        perror("[-] Error Setting the socket parameters ");
        close(listenerSocket_fd);
        exit(EXIT_FAILURE);
    }
    printf("[*] Set sockopt done correctly \n");

    struct sockaddr_in adr ;
    adr.sin_family = AF_INET ;
    adr.sin_port = htons(port);
    adr.sin_addr.s_addr = htonl(INADDR_ANY) ; //or inet_pton(AF_INET , "0.0.0.0" , &adr.sin_addr);


    if (bind(listenerSocket_fd , (struct sockaddr *)&adr , sizeof(adr) ) == -1) {
        perror("[-] Error Binding the adr/port to socket ");
        close(listenerSocket_fd);
        exit(EXIT_FAILURE);
    }
    printf("[*] Bound the socket to the adr correctly\n");

    if (listen(listenerSocket_fd , BACKLOG) == -1) {
        perror("[-] Listen failed starting ");
        close(listenerSocket_fd);
        exit(EXIT_FAILURE);
    }
    return listenerSocket_fd ;
}

void getLoaclIpAddrString(char * ipBuf){
    struct ifaddrs *ifa , *ifaIter;
    int state = getifaddrs(&ifa) ;
    if (state == -1 ){
        perror("[-] Error getting the local adress ip for display : ");
        ipBuf[0] = '\0' ;
        return ;
    }
    for (ifaIter = ifa ; ifaIter != NULL ; ifaIter = ifaIter->ifa_next){
        if (ifaIter->ifa_addr == NULL) continue; 
        if (strcmp(ifaIter->ifa_name, "lo") == 0) continue;
        if (ifaIter->ifa_addr->sa_family == AF_INET) {
            inet_ntop(AF_INET , &(*((struct sockaddr_in *)(((*ifaIter).ifa_addr)))).sin_addr , ipBuf , INET_ADDRSTRLEN ) ; //hardest fucking line ever 
            freeifaddrs(ifa);
            return ;
        }
    }
    freeifaddrs(ifa);
    return ;
}


int recvAndSendUsingGivenSocketClientSide(int connectionFD){
        fd_set currentFDs , readFDs ;
        FD_ZERO(&currentFDs) ;
        FD_SET(STDIN_FILENO, &currentFDs);
        FD_SET(connectionFD ,&currentFDs);
        
        while (1)
        {
            FD_ZERO(&readFDs);
            readFDs = currentFDs ;
            select(connectionFD+1 , &readFDs , NULL , NULL , NULL );
            if (FD_ISSET(connectionFD , &readFDs)){ //ready to read from socket
                ssize_t num_of_recved_bytes ;
                char buf[MAX_BUF_SIZE] ;
                if ( (num_of_recved_bytes = recv(connectionFD , buf , MAX_BUF_SIZE - 1 , 0 )) == -1 ){
                    perror("[-] Error receiving the message ");
                    close(connectionFD);
                    return -1 ; // error 
                } else if (num_of_recved_bytes == 0) {
                    printf("[*] Connection closed . exiting ...\n");
                    close(connectionFD);
                    return 0 ;  //not error
                }
                buf[num_of_recved_bytes] = '\0';
                fwrite(buf ,sizeof(char) , num_of_recved_bytes , stdout);
            } 
            if (FD_ISSET(STDIN_FILENO , &readFDs)){//ready to send to socket (thats means the user pressed some keystrokes then pressed ENTER , which mean we have something waiting inside the kernel buffer that we can consume using fgets or anything )
                char buf[MAX_BUF_SIZE] ;
                fgets(buf , MAX_BUF_SIZE , stdin );
                if(send(connectionFD , buf , strlen(buf),0) == -1){
                    perror("[-] Error sending the msg ");
                    close(connectionFD);
                    return -1; //error
                }
            }
        }
}

//when done from this function (error happened or connection closed ) remove it from the accumilator , and clsoe the thread
int recvAndBroadcastUsingGivenSocketServerSideThread(AcceptedConnection acceptedConnection){
        int connectionFD = acceptedConnection.acceptedConnectionFD ;
        while (1) {   
            ssize_t num_of_recved_bytes ;
            char buf[MAX_BUF_SIZE] ;
            if ( (num_of_recved_bytes = recv(connectionFD , buf , MAX_BUF_SIZE - 1 , 0 )) == -1 ){
                perror("[-] Error receiving the message ");
                close(connectionFD);
                return -1 ; // error 
            } else if (num_of_recved_bytes == 0) {
                printf("[*] Connection closed . exiting ...\n");
                close(connectionFD);
                return 0;  //not error
            }
            buf[num_of_recved_bytes] = '\0';
            char msgWithIP[MAX_BUF_SIZE + 24] ;
            sprintf(msgWithIP , "%s : %s",acceptedConnection.addrIpString , buf );
            printf("%s",msgWithIP);

            //broadcast part

            pthread_mutex_lock(&locker);
            for (int i = 0 ; i < connectionsCpt ; i++){
                if (accumulatedConnections[i].acceptedConnectionFD == acceptedConnection.acceptedConnectionFD)  continue;
                 
                if(send(accumulatedConnections[i].acceptedConnectionFD , msgWithIP , strlen(msgWithIP),0) == -1){
                perror("[-] Error sending the msg : ");
                close(connectionFD);
                return -1; 
                }
            }
            pthread_mutex_unlock(&locker);
            
        }
}

AcceptedConnection acceptIncomingConnectionOnListeningSocket(int listeningSockedFD){
    AcceptedConnection result ;
    strcpy(result.addrIpString , "NO_IP"); // intialization
    struct sockaddr_in addr ;
    socklen_t addrSize = sizeof(addr) ;
    result.acceptedConnectionFD = accept(listeningSockedFD , (struct sockaddr *)&addr , &addrSize );
    if (result.acceptedConnectionFD == -1){
        perror("[-] Error accepting a connection socket : ");
        return result;
    }
    inet_ntop(AF_INET , &result.addr.sin_addr.s_addr , result.addrIpString, INET_ADDRSTRLEN );
    return result ;
}

void * handleClient(void * acceptedConnectionPtr){ // what this should do ?? adds the accepted connection to the array and remove it at the end, recv messages from the client print them normally , BUT , when  trying to send , it should broadcast the message to all other connecting sockets , which are found in accumulatedConnections array , if the connection gets closed somehow, update that array and remove the FD from it , i think thats it 
    AcceptedConnection acceptedConnection = *(AcceptedConnection *)(acceptedConnectionPtr) ;

    pthread_mutex_lock(&locker);
    accumulatedConnections[connectionsCpt] = acceptedConnection;
    connectionsCpt++ ; 
    pthread_mutex_unlock(&locker) ;

    recvAndBroadcastUsingGivenSocketServerSideThread(acceptedConnection);

    pthread_mutex_lock(&locker);
    for (int i = 0 ; i < connectionsCpt ; i++){
        if (i == acceptedConnection.acceptedConnectionFD){
            accumulatedConnections[i] = accumulatedConnections[connectionsCpt - 1];
            break ;
        }
    }
    pthread_mutex_unlock(&locker);
    
    printf("[*] The connection from user with IP : %s was closed \n",acceptedConnection.addrIpString);
    pthread_exit(NULL);
}

#endif /* FUNCTIONS_H */
