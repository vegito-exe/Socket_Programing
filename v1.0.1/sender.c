#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include "functions.h"


int main (int argc , char * argv[]){

    if (argc != 3){
        printf("To use this program : %s <IPv4-Adress> <Port>\n", argv[0]);
        return -1;
    }

    //=================================================== port validation START
    int port = verify_and_parse_port(argv[2]);
    //=================================================== IP validation START
    int ip = verify_and_parse_ip(argv[1]);
    printf("[*] Connecting to %s:%d\n",argv[1],port);
    //=================================================== Starting the connection socket 
    int connection_fd = tcp_socket_connect(ip , port ) ;
    printf("Connected ! you can start sending your message right now , press Ctrl + C to close the connection if you want .\n");
    //=================================================== the sending (communication) loop
    char buf[MAX_BUF_SIZE] ;
    while (1)
    {
        fgets(buf , MAX_BUF_SIZE , stdin ) ;
        if(send(connection_fd , buf , strlen(buf),0) == -1){
            perror("[-] Error sending the msg ");
            close(connection_fd);
            return -1;
        }
    }
    
    return 0 ;
}