#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include "includes/functions.h"



int main(int argc , char * argv[]){

    if (argc > 3 || argc < 2) {
        printf("usage : \nfor hosting (first connecting user) : ./chat <port> \nfor client (other users connecting to the host ) : ./chat <host-ip> <port>\n");
        return -1 ;
        }

    //client code start
    if (argc == 3 ) { 
        int ip = verify_and_parse_ip(argv[1]) ;
        int port = verify_and_parse_port(argv[2]) ;

        int connectionFD = tcp_socket_connect(ip , port );
        printf("[*] Connected Sucsessfully, you can start typing (CTRL + C to close the program)\n");

        int exitStatus = recvAndSendUsingGivenSocket(connectionFD);
        return exitStatus ;
    }

    //server code start 
    if (argc == 2 ) {
        int port = verify_and_parse_port(argv[1]) ;
        int serverFD = tcp_socket_listen(port) ; //queue up to BACKLOG connections

        char serverIP[16] ;
        getLoaclIpAddrString(serverIP );
        printf("[*] server LAN adress is :  %s , share this for other LAN clients to let them connect .\n", serverIP);

        AcceptedConnection connection = acceptIncomingConnectionOnListeningSocket(serverFD);
        if (connection.acceptedConnectionFD == -1 )  {
            perror("a client tried to connect but a problem accepting the connection occurred : ");
        }  else {
            printf("[+] A client with IP : %s has connected ! \n" , connection.addrIpString);
        }
        
        int exitStatus = recvAndSendUsingGivenSocket(connection.acceptedConnectionFD) ;
        return exitStatus;
    }




    return 0 ;
}