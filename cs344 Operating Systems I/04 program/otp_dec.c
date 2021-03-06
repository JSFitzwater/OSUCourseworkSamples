/*  Name:  James Fitzwater
 *  Email: fitzwatj@onid.oregonstate.edu
 *  Class: CS 344.400
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h> 
#include <signal.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stddef.h>
#include <dirent.h>
#include <time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>


/* Gay, Warren: Chapter 14 code: mkaddr.c */
/* Gay, Warren: Listing 14.1: 
 * "The bindacpt.c Source Module for Connecting and Accepting" */
/**  http://www.linuxhowtos.org/data/6/client.c  **/
/**  referer: http://www.linuxhowtos.org/C_C++/socket.htm  **/

/** Suraj Kurapati
    Network Programming Projec, USC CMPE-150, Spring 2004  
    github.com/sunaku/simple­ftp
    skurapti@ucsc.edu
**/


int auditLine(char *ln, char *str)
{
    int n1;
    int matchRes = -1;

    char *c1 = ln;
    char *c2 = str;

    n1 = 0;
    if (c1[0] == c2[0])
    {
        matchRes = 1;
    }

    n1 = 1;
    while(c1[n1])
    {
        if (c1[n1] == ':' || c2[n1] == ':')
        {
            return matchRes;
        }
        if (c1[n1] != c2[n1])
        {
            matchRes = -1;
        }
        n1++;
    }

    return matchRes;
}


int main(int argc, char *argv[])
{

    int foo, sockfd, portno, n, x, z;
    int testsize1;
    int testsize2;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    DIR* dirfd;
    FILE* filefd;
    struct dirent *dirfile;

    char buffer2[72704];
    char buffer3[72704];

    int filesize, filesize2;


    if (argc < 4) {
        fprintf(stderr,"OTP_DEC: arg syntax: otp_dec [filename] [filename] [port]\n", argv[0]);
       exit(0);
    }

    // audit file contents

    // open Directory
    if((dirfd = opendir(".")) == NULL)
    {
        fprintf(stderr,"OTP_DEC: opendir failure");
    }else{
        x=2;

        // while reading files...
        while(dirfile = readdir(dirfd)){


            // if cmd line argument...
            if(strcmp(argv[1],dirfile->d_name) == 0){

                // open plaintext file
                if((filefd = fopen(dirfile->d_name, "r")) != NULL){


                    // Kurapati (server.c & service.c): 
                    // "determine file size"
                    fseek(filefd, 0, SEEK_END);
                    testsize1 = ftell(filefd);

                    // Kurapati (server.c & service.c): 
                    // "read contents into buffer"
                    memset( buffer2, '\0', 72704 );
                    fread(buffer2, sizeof(char), testsize1, filefd);


                    // audit plaintext file contents
                    z=0;
                    while(buffer2[z]){
                        if(buffer2[z] > 90){
                            fprintf(stderr,"OTP_DEC: ERROR: plaintext file: prohibited character(s)\n");
                        }
                        if(buffer2[z]  < 65 
                        && buffer2[z] != 32){
                            fprintf(stderr,"OTP_DEC: ERROR: plaintext file: prohibited character(s)\n");
                        }
                        z++;
                    }
                    fclose(filefd);
                }
            }


            // if cmd line argument...
            if(strcmp(argv[2],dirfile->d_name) == 0){

                // open key file
                if((filefd = fopen(dirfile->d_name, "r")) != NULL){


                    // Kurapati (server.c & service.c): 
                    // "determine file size"
                    fseek(filefd, 0, SEEK_END);
                    testsize2 = ftell(filefd);

                    // Kurapati (server.c & service.c): 
                    // "read contents into buffer"
                    memset( buffer3, '\0', 72704 );
                    fread(buffer3, sizeof(char), testsize2, filefd);


                    // audit keyfile file contents
                    z=0;
                    while(buffer3[z]){
                        if(buffer3[z] > 90){
                            fprintf(stderr,"OTP_DEC: ERROR: key file: prohibited character(s)\n");
                        }
                        if(buffer3[z]  < 65 
                        && buffer3[z] != 32){
                            fprintf(stderr,"OTP_DEC: ERROR: key file: prohibited character(s)\n");
                        }
                        z++;
                    }
                    fclose(filefd);
                }
            }


        }
        closedir(dirfd);

        //test size
        if(testsize1 > testsize2){
            fprintf(stderr,"OTP_ENC: ERROR: key/plaintext size mismatch\n");
            exit(1);
        }
    }


    // setup connection

    portno = atoi(argv[3]);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    // if (sockfd < 0){ 
        // fprintf(stderr,"OTP_DEC: ERROR opening socket\n");
    // }

    server = gethostbyname("localhost");
    if (server == NULL) {
        // fprintf(stderr,"OTP_DEC: ERROR, no such host\n");
        exit(0);
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);

    if (connect(sockfd,(struct sockaddr *) &serv_addr,
        sizeof(serv_addr)) < 0){ 
        int foo=0; // fprintf(stderr,"OTP_DEC: ERROR connecting\n");
    }


    // socket security: guarantee connected to OTP_ENC_D

    // send this ID, OTP_ENC
    char* buffer = "otp_dec";

    n = send(sockfd,buffer,strlen(buffer),0);
    // if (n < 0){ 
         // fprintf(stderr,"OTP_DEC: ERROR writing to socket\n");
    // }
    if(n < strlen(buffer) ){
        foo=0; // fprintf(stdout,"OTP_DEC: WARNING: Not all data written to socket!\n");
    }

    // recv partner's ID
    memset( buffer2, '\0', 72704 );
    n = recv( sockfd, buffer2, sizeof(buffer2) - 1, 0 );
    // if (n < 0){ 
         // fprintf(stderr,"OTP_DEC: ERROR reading from socket\n");
    // }

    // connection check: OTP_ENC_D
    if(strcmp("otp_dec_d",buffer2) != 0){
        // fprintf(stdout,"OTP_DEC: prohibited server\n");
        close( sockfd );
    }else{

        //  Brewster: "print contents"
        // fprintf(stdout,"OTP_DEC: recvd from server: \"%s\"\n",buffer2);

        // open Directory
        if((dirfd = opendir(".")) == NULL)
        {
            fprintf(stderr,"OTP_DEC: opendir failure");
        }else{

            x=1;

            while( x < 3){

                if((filefd = fopen(argv[x], "r")) != NULL){

                    // Kurapati (server.c & service.c): 
                    // "determine file size"
                    fseek(filefd, 0, SEEK_END);
                    if(x == 1){
                        filesize2 = ftell(filefd);
                    }
                    filesize = filesize2;

                    rewind(filefd);

                    // Kurapati (server.c & service.c): 
                    // "read contents into buffer"
                    memset( buffer2, '\0', 72704 );
                    fread(buffer2, sizeof(char), filesize, filefd);

                    fclose(filefd);

                    // send buffer
                    n = send(sockfd,buffer2,filesize,0);
                    // if (n < 0){ 
                         // fprintf(stderr,"OTP_DEC: ERROR writing to socket\n");
                    // }
                    if(n < strlen(buffer2) ){
                        foo=0; // fprintf(stdout,"OTP_DEC: WARNING: Not all data written to socket!\n");
                    }

                    // recv acknowledgement
                    memset( buffer2, '\0', 72704 );
                    n = recv( sockfd, buffer2, sizeof(buffer2) - 1, 0 );
                    // if (n < 0){ 
                         // fprintf(stderr,"OTP_DEC: ERROR reading from socket\n");
                    // }

                    // audit acknowledgement
                    if(strcmp("recvd",buffer2) != 0){
                        foo=0; // fprintf(stdout,"OTP_DEC: didn't get \'recvd\', got \'%s\'\n",buffer2);
                    }else{
                        foo=0; // fprintf(stdout,"OTP_DEC: got recvd signal.\n");
                    }
                }

                x++;
            }

            // recv cipher file contents
            memset( buffer2, '\0', 72704 );
            n = recv( sockfd, buffer2, sizeof(buffer2) - 1, 0 );
            // if (n < 0){ 
                 // fprintf(stderr,"OTP_DEC: ERROR reading from socket\n");
            // }

            // acknowledge reception
            n = send( sockfd, "recvd", 5, 0);
            // if(n < 0 ){
                // fprintf(stderr,"OTP_DEC: ERROR writing to socket\n"); exit(1);
            // }
            if(n < strlen(buffer) ){
                foo=0; // printf("OTP_DEC: WARNING, Not all data written to socket!\n");
            }

            fflush(stdout);

            // print to STDOUT cipher contents
            fprintf(stdout,"%s\n",buffer2);

            // close directory
            closedir(dirfd);

            // Brewster: "close socket"
            close( sockfd );
        }
    }
    return 0;
}
