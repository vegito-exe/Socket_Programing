#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h> //for socket functions
#include <netinet/in.h> //for the types like sockaddr_in and conversions like htons , 
#include <arpa/inet.h> //for diff ip conversions  like inet_addr()
#include <unistd.h> //for close() to work

#define MAX_BUF_SIZE 2056


int main(int argc , char * argv[]){

    if (argc != 2) {
        printf("To use this program : %s <Port>\n", argv[0]);
        return -1;
    }
    
    //=================================================== port validation START
    long parsed_port ;
    char * endptr ;
    parsed_port = strtol(argv[1],&endptr,10);                 // instead of int port = atoi(argv[1]) to make more verifications 

    if (*endptr != '\0') {
        printf("Invalid Charachter in input : %c \n" , *endptr);  
        return -1;
    } else if ((parsed_port < 1) || (parsed_port > 65535)) {
        printf("Invalid port value - Ports range 1-65535 \n");
        return -1;
    }

    int port = (int) parsed_port ;
    printf("[*] Creating a server at port %d ...\n" , port) ;
    //=================================================== port validation END
    //=================================================== Creation of the listening socket START

    int listenerSocket_fd ;
    if ((listenerSocket_fd = socket(AF_INET , SOCK_STREAM , 0)) == -1 ){
        perror("[-] Error Creating the socket ");   //this one prints the msg u have, checks the value of errno (a var from errno.h) then find the string explanation linked to that value and prints it
        return -1 ;
    }
    printf("[*] Created listener socket \n");

    int opt = 1 ;
    if (setsockopt(listenerSocket_fd , SOL_SOCKET , SO_REUSEADDR , &opt , sizeof(opt)) == -1 ){
        perror("[-] Error Setting the socket parameters ");
        close(listenerSocket_fd);
        return -1 ;
    }
    printf("[*] Set sockopt done correctly \n");

    struct sockaddr_in adr ;
    adr.sin_family = AF_INET ;
    adr.sin_port = htons(port);
    adr.sin_addr.s_addr = htonl(INADDR_ANY) ; //or inet_pton(AF_INET , "0.0.0.0" , &adr.sin_addr);


    if (bind(listenerSocket_fd , (struct sockaddr *)&adr , sizeof(adr) ) == -1) {
        perror("[-] Error Binding the adr/port to socket ");
        close(listenerSocket_fd);
        return -1;
    }
    printf("[*] Bound the socket to the adr correctly\n");

    if (listen(listenerSocket_fd , 3) == -1) {
        perror("[-] Listen failed starting ");
        close(listenerSocket_fd);
        return -1 ;
    }
    printf("[*] Socket starting listening for requests \n");
    //=================================================== Creation of the listening socket END
    //=================================================== connection Start

    int connection_fd ;
    struct sockaddr_in client_adr ;
    socklen_t client_adr_length = sizeof(client_adr);
    if ((connection_fd = accept(listenerSocket_fd , (struct sockaddr *)&client_adr , &client_adr_length)) == -1 ){
        perror("[-] Error accepting the client requesting socket");
        close(listenerSocket_fd);
        return -1 ; 
    }
    printf("[*] Connected ! Waiting for incoming messages and startig the chat ,\n");

    while (1){
        ssize_t num_of_recved_bytes ;
        char buf[MAX_BUF_SIZE] ;
        if ( (num_of_recved_bytes = recv(connection_fd , buf , MAX_BUF_SIZE - 1 , 0 )) == -1 ){
            perror("[-] Error receiving the message ");
            close(connection_fd);
            close(listenerSocket_fd);
            return -1 ;
        } else if (num_of_recved_bytes == 0) {
            printf("Connection closed . exiting ...\n");
            close(connection_fd);
            close(listenerSocket_fd);
            return 0 ;   
        }
        buf[num_of_recved_bytes] = '\0';
        fwrite(buf ,sizeof(char) , num_of_recved_bytes , stdout);
    }
    //=================================================== connection END
    return 0 ;
}