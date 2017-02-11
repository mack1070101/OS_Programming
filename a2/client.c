/*******************************************************************
 * CMPUT 379 Assignment 2
 * Due:
 *
 * Group: Mackenzie Bligh & Justin Barclay
 * CCIDs: bligh & jbarclay
 * *****************************************************************
 * Notes: this section should be removed
 *   */

/*  Imports*/
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <strings.h>
#define	MY_PORT	2222

int main(int argc, char *argv[]){
    int s, number;
    struct sockaddr_in server;
    struct hostent *host;

    host = gethostbyname("localhost");

    if(host == NULL){
        perror("Client: cannot get host description");
        exit(-1);
    }
    while(1){
        // Create new socket
        s = socket(AF_INET, SOCK_STREAM, 0);

        // Check for failed socket
        if (s < 0) {
            perror ("Client: cannot open socket");
            exit (-2);
        }
        bzero(&server, sizeof(server)); // zeroize socket struct
        // Copy server address into socket struct
        bcopy (host->h_addr_list[0], & (server.sin_addr), host->h_length);

        server.sin_family = host->h_addrtype;
        server.sin_port = htons (MY_PORT);

        if(connect (s, (struct sockaddr*) & server, sizeof (server))){
            perror("Client: cannot connect to server");
            exit(-3);
        }

        read(s, &number, sizeof (number)); //Read socket
        close(s); // Close socket

        // Probably get rid of this bullshit
        fprintf(stderr, "Process %d gets number %d\n", getpid (),
                ntohl (number));
        sleep (2);

    }
    return 0;
}
