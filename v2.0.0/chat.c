#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include "includes/functions.h"




int main(int argc , char * argv[]){

    if (argc > 3 || argc < 2) {
        printf("usage : \nfor hosting (first connecting user) : ./chat <port> \nfor client (other users connecting to the host ) : ./chat <host-ip> <port>\n");
        return -1 ;
        }

    if (argc == 3 ) { //client
        int ip = verify_and_parse_ip(argv[1]) ;
        int port = verify_and_parse_port(argv[2]) ;
        int connectionFD = tcp_socket_connect(ip , port );
        printf("[*] Connected Sucsessfully, you can start typing (CTRL + C to close the program)\n");
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
                    return -1 ;
                } else if (num_of_recved_bytes == 0) {
                    printf("[*] Connection closed . exiting ...\n");
                    close(connectionFD);
                    return 0 ;   
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
                    return -1;
                }
            }
        }

    }
    //client code end 
    //server code start 
    if (argc == 2 ) {
        int port = verify_and_parse_port(argv[1]) ;
        int serverFD = tcp_socket_listen(port) ;

        struct sockaddr_in addr ;
        
        
        socklen_t addrSize = sizeof(addr) ;
        int connectionFD = accept(serverFD , (struct sockaddr *)&addr , &addrSize );
        if (connectionFD == -1){
            perror("[-] Error accepting the connection socket : ");
            close(serverFD);
            return -1 ;
        }
        char senderIP[16];
        inet_ntop(AF_INET , &addr.sin_addr.s_addr , senderIP, INET_ADDRSTRLEN );
        printf("[+] A client with IP : %s has connected ! \n" , senderIP);

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
                    return -1 ;
                } else if (num_of_recved_bytes == 0) {
                    printf("[*] Connection closed . exiting ...\n");
                    close(connectionFD);
                    return 0 ;   
                }
                buf[num_of_recved_bytes] = '\0';
                printf("%s : %s",senderIP,buf);
            } 
            if (FD_ISSET(STDIN_FILENO , &readFDs)){//ready to send to socket (thats means the user pressed some keystrokes then pressed ENTER , which mean we have something waiting inside the kernel buffer that we can consume using fgets or anything )
                char buf[MAX_BUF_SIZE] ;
                char msg[] = "Server : " ;
                fgets(buf , MAX_BUF_SIZE , stdin );
                if(send(connectionFD , msg , strlen(msg),0) == -1){
                    perror("[-] Error sending the msg ");
                    close(connectionFD);
                    return -1;
                }
                if(send(connectionFD , buf , strlen(buf),0) == -1){
                    perror("[-] Error sending the msg ");
                    close(connectionFD);
                    return -1;
                }
            }
        }
        
    }




    return 0 ;
}