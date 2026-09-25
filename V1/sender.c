#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>


#define MAX_BUF_SIZE 2056


int main (int argc , char * argv[]){

    if (argc != 3){
        printf("To use this program : %s <IPv4-Adress> <Port>\n", argv[0]);
        return -1;
    }

    //=================================================== port validation START
    long parsed_port ;
    char * endptr ;
    parsed_port = strtol(argv[2],&endptr,10);                 // instead of int port = atoi(argv[1]) to make more verifications 

    if (*endptr != '\0') {
        printf("Invalid Charachter in port input : %c \n" , *endptr);  
        return -1;
    } else if ((parsed_port < 1) || (parsed_port > 65535)) {
        printf("Invalid port value - Ports range 1-65535 \n");
        return -1;
    }

    int port = (int) parsed_port ;
    //=================================================== port validation END
    //=================================================== IP validation START
    int state ;
    int ip ;
    if((state = inet_pton(AF_INET , argv[1] , &ip)) == 0 ){
        printf("Invalid Ip adress - Please enter a value in this form : 0-255.0-255.0-255.0-255 \n");
        return -1 ;
    }else if (state == -1) {
        perror("[-] Error parsing the Ip adress ");
        return -1 ;
    }
    printf("[*] Connecting to %s:%d\n",argv[1],port);
    //=================================================== IP validation END
    //=================================================== Starting the connection socket 
    int connection_fd; 
    if((connection_fd = socket(AF_INET, SOCK_STREAM , 0)) == -1 ){
        perror("[-] Error Creating the socket ");   //this one prints the msg u have, checks the value of errno (a var from errno.h) then find the string explanation linked to that value and prints it
        return -1 ;
    }
    printf("[*] Created connection socket \n");

    struct sockaddr_in adr ;
    adr.sin_family = AF_INET ;
    adr.sin_port = htons(port);
    adr.sin_addr.s_addr = ip ;

    if (connect(connection_fd , (struct sockaddr *)&adr , sizeof(adr) ) == -1 ) {
        perror("[-] Error establishing the connection ");
        close(connection_fd);
        return -1 ;
    }
    printf("Connected ! you can start sending your message right now , press Ctrl + C to close the connection if you want .\n");
    
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