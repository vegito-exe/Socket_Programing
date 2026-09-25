#include <stdio.h>
#include <sys/socket.h> //for socket functions
#include <netinet/in.h> //for the types like sockaddr_in and conversions like htons , 
#include <arpa/inet.h> //for diff ip conversions  like inet_addr()
#include <unistd.h> //for close() to work
#include "functions.h"


int main(int argc , char * argv[]){

    if (argc != 2) {
        printf("To use this program : %s <Port>\n", argv[0]);
        return -1;
    }
    
    //=================================================== port validation START
    int port = verify_and_parse_port(argv[1]);
    printf("[*] Creating a server at port %d ...\n" , port) ;
    //=================================================== port validation END
    //=================================================== Creation of the listening socket START
    int listenerSocket_fd = tcp_socket_listen(port);
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
            printf("[*] Connection closed . exiting ...\n");
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