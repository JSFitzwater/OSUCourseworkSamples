/*  Name:  James Fitzwater
 *  Email: fitzwatj@onid.oregonstate.edu
 *  Class: CS 344.400
 */

#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stddef.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 


/**  Resources:
 **
 **  Prof. Brewster's 'multiserver.c' from on-campus CS 344 
 **  lecture slides
 **
 **  Stevens, W. Richard & Stephen A. Rago. "Communicating
 **  with a Network Printer." Chap. 21, Sec. 5 in Advanced
 **  Programming in the Unix Environment. 3rd ed. Upper
 **  Saddle River, NJ: Addison-Wesley, 2013. 812-819.
 **/

/** Suraj Kurapati
    Network Programming Projec, USC CMPE-150, Spring 2004  
    github.com/sunaku/simple­ftp
    skurapti@ucsc.edu
**/


int main(int argc, char *argv[]) 
{
    int a,b,z,
        bindTry,
        bkgdFD,
        charsRead, 
        charsScribe,
        childPID,
        estabConnectFD,
        maxfd, 
        portNum, 
        selTry, 
        socktFD,
        xfd; 


    DIR* dirfd;
    FILE* filefd;
    struct dirent *dirfile;

    struct sockaddr_in servAddr,
                       clieAddr; 

           socklen_t   sizeClieInfo; 

    char buffer[72704]; 
    char buffer2[72704]; 
    char buffer3[72704]; 

    fd_set dset;
    fd_set rset;
    fd_set wset;

    int sfd[6];



    /*   Brewster: "check usage & args"
     *   % 0,prgm 1,port
     */

    if( argc != 2 )
    {
        fprintf(stderr,"Usage: otp_enc_d [port]\n");
        exit(1);
    }



    /*   Brewster: "set up address struct
     *   for this process (the server)"            
     */

    /*  Kerrisk, Michael. "ip(7)." 
     *  Linux Programmer's Manual. 
     *  http://man7.org/linux/man-pages/man7/ip.7.html
     *  
     * "When INADDR_ANY is specified in the [BIND(2)]
     *  call, the socket will be bound to all local 
     *  interfaces.  When LISTEN(2) is called on 
     *  an unbound socket, the socket is automatically 
     *  bound to a random free port with the local 
     *  address set to INADDR_ANY.  When CONNECT(2) 
     *  is called on an unbound socket, the socket 
     *  is automatically bound to a random free port 
     *  or to a usable shared port with the local 
     *  address set to INADDR_ANY."
     *  
     * "There are several special addresses: 
     *  INADDR_LOOPBACK (127.0.0.1) always refers 
     *  to the local host via the loopback device; 
     *  INADDR_ANY (0.0.0.0) means any address for 
     *  binding; INADDR_BROADCAST (255.255.255.255) 
     *  means any host and has the same effect on 
     *  bind as INADDR_ANY for historical reasons."
     */

    //  Brewster: "clear out address" 
    memset( (char*)&servAddr, '\0', sizeof(servAddr) );



    /*  REPLACE WITH 2 RAND-GEN PORT NUMS  *
     */

    //  Brewster: "get port number, convert 
    //  to an integer from a string" 
    portNum = atoi( argv[1]);


    //  Brewster: "create a network
    //  -capable socket"
    servAddr.sin_family = AF_INET;

    //  Brewster: "store port number"
    servAddr.sin_port = htons( portNum );


    //  Brewster: "copy in address...
    //  any address is allowed for 
    //  connection to this process"
    servAddr.sin_addr.s_addr = INADDR_ANY;
  

    //  Brewster: "convert machine name
    //  to a special form of address"
    //  (localhost eq 127.0.0.1)
    //servAddr = gethostbyname( "localhost" );



    /*   Brewster: "set up socket"
     */

    //  Brewster: "create socket"
    socktFD = socket(AF_INET,SOCK_STREAM,0);

    if(socktFD < 0){
		// fprintf(stderr,"OTP_ENC_D: ERROR opening socket\n"); 
        exit(2);
	}



    /*   Brewster: "enable socket 
     *   to begin listening"  
     */

    //  Brewster: "connect socket to port" 
    bindTry = bind(socktFD, (struct sockaddr *)&servAddr,
               sizeof(servAddr));

    if(bindTry < 0 ){
		// fprintf(stderr,"OTP_ENC_D: ERROR binding\n"); 
        exit(1);
	}

    //  Brewster: "flip socket on...
    //  it can now receive up to 
    //  5 connections"
    listen( socktFD, 5 );

    while (1) 
    {



        /*  INT SELECT( A, B, C, D, E )
         *   • A: int     fd: 1 + num available
         *   • B: fd_set* in/read bitmask
         *   • C: fd_set* out/write bitmask
         *   • D: fd_set* error bitmask
         *   • E: struct  timeval*  timeout length
         *
         *  FD_SETSIZE, "max number of file descriptors"
         *  FD_ZERO,  "set all bits to 0"
         *  FD_SET,   "set one specific bit to 1"
         *  FD_ISSET, "determine if a specific bit is set to 1"
         *  FD_CLR,   "set one specific bit to 0"
         *
         *  RET VALS
         *    •  -1 error
         *    •   0 no action/timeout
         *    •  >0 count of fds worked 
         */



        /*   Brewster: "accept a connection,
         *   blocking if one is not available
         *   until one connects"   
         */ 

        //  Brewster: "get size of address
        //  for client that will connect" 
        sizeClieInfo = sizeof(clieAddr);



        //  Brewster: "accept [client
        //  to address]"
        estabConnectFD = accept(socktFD, 
                         (struct sockaddr *)&clieAddr, 
                         &sizeClieInfo);
         



        if(estabConnectFD < 0 ){
            // fprintf(stderr,"OTP_ENC_D: ERROR on accept\n"); 
            exit(1);
        }


        // bkgdFD = dup2(estabConnectFD,bkgdFD);
        // close( estabConnectFD );


        /*   Brewster: "get message from client" 
         */



        if((childPID = fork()) == 0)
        {

            // Kurapati (server.c & service.c): 
            // "child doesn't need this socket"
            close(socktFD);

            /*
             *  SSIZE_T RECV(sox file descrip,
             *               ptr to data pkg,
             *               bytes to catch,
             *               config flags);
             *
             *  SSIZE_T RECV(int sockfd,
             *               void *msg,
             *               size_t message_size,
             *               int flags);
             */

            /*
             * "• Data may arrive in odd size bundles
             *
             *  • recv() or read() will return exactly
             *    amount of data that has already arrived
             *
             *  • more data may be coming as long as the
             *    return value is greater than 0
             *
             *  • recv() and read() will block if the
             *    connection is open but no data is available
             *
             *   % gcc -o client client.c
             *   % gcc -o server server.c
             *   % ./server 51717 &
             *   % ./client localhost 51717 "
             */


            //  Brewster: "buffer clear-out for reuse"
            memset(buffer, '\0', sizeof(buffer)); 

            //  Brewster: "read data from 
            //  socket, leaving \0 at end"
            charsRead = recv( estabConnectFD, buffer, 
                              sizeof(buffer) - 1, 0 );

            if(charsRead < 0 ){
                // fprintf(stderr,"OTP_ENC_D: ERROR reading from socket\n"); 
                exit(1);
            }




            //  recv ID from client
            // printf("OTP_ENC_D: recvd from client: \"%s\"\n",buffer);

            // security: audit client ID
            if(strcmp("otp_enc",buffer) != 0){
                charsScribe = send( estabConnectFD, "otp_enc_d", 9, 0);
                if(charsScribe < 0 ){
                    // fprintf(stderr,"OTP_ENC_D: ERROR writing to socket\n"); 
                    exit(1);
                }
                printf("OTP_ENC_D: prohibited client\n");
                close( estabConnectFD );
                continue;
            }else{



                /*   Brewster: "send return
                 *   Success back to client"  
                 */


                /*
                 *  ssize_t send(sox file descrip,
                 *               ptr to data pkg,
                 *               bytes to send,
                 *               config flags);
                 *
                 *  ssize_t send(int sockfd,
                 *               void *msg,
                 *               size_t message_size,
                 *               int flags);
                 */


      
               // //  send cipher back to otp_enc
               // charsScribe = send( estabConnectFD, buffer,
               //                     strlen(buffer), 0 );



                //  test - send ID to client
                charsScribe = send( estabConnectFD, "otp_enc_d", 9, 0);
     
                if(charsScribe < 0 ){
                    // fprintf(stderr,"OTP_ENC_D: ERROR writing to socket\n"); 
                    exit(1);
                }


                // recv plaintext file from client

                //  Brewster: "buffer clear-out for reuse"
                memset(buffer, '\0', sizeof(buffer)); 

                //  Brewster: "read data from 
                //  socket, leaving \0 at end"
                charsRead = recv( estabConnectFD, buffer, 
                                  sizeof(buffer) - 1, 0 );

                if(charsRead < 0 ){
                   //  fprintf(stderr,"OTP_ENC_D: ERROR reading from socket\n"); 
                    exit(1);
                }

                // plaintext, send acknowledgement
                charsScribe = send( estabConnectFD, "recvd", 5, 0);
                if(charsScribe < 0 ){
                    // fprintf(stderr,"OTP_ENC_D: ERROR writing to socket\n"); 
                    exit(1);
                }



                // recv key file from client

                //  Brewster: "buffer clear-out for reuse"
                memset(buffer2, '\0', sizeof(buffer)); 

                //  Brewster: "read data from 
                //  socket, leaving \0 at end"
                charsRead = recv( estabConnectFD, buffer2, 
                                  sizeof(buffer2) - 1, 0 );

                if(charsRead < 0 ){
                    // fprintf(stderr,"OTP_ENC_D: ERROR reading from socket\n"); 
                    exit(1);
                }





                // key, send acknowledgement
                charsScribe = send( estabConnectFD, "recvd", 5, 0);
                if(charsScribe < 0 ){
                   //  fprintf(stderr,"OTP_ENC_D: ERROR writing to socket\n"); 
                    exit(1);
                }





                // do cipher
                z = 0;
                while(buffer[z]){
                   
                    if( buffer[z] == 32 )
                    {
                        b = buffer[z];
                    }
                    else {
                    /*
                        H    E    L    L    O   
                        72   69   76   76   79
                        G    K    K    Y    E  
                        71   75   75   89   69
                        --- ---  ---  --- ---
                        143  144  151  165  148
                   - 65
                        78   79   86   100  83
                   - 26
                                       74
                        N    O    V    J    S
                        78   79   86   74   83

                   + 65
                        143  144  151  139  148
                    */

                    /*
                           a = x + y
                           b = a - 65
                        if b > 90
                           b = b - 26
                        ===============
                           a = b + 65
                           x = a - y
                        if x < 65
                           x = x + 26
                    */

                        a = buffer[z] + buffer2[z];

                        b = a - 65;

                        if( b > 90 )
                        {
                            b = b - 26;
                        }
                    }

                    buffer3[z] = b;
                    z++;
                }

                buffer3[z] = '\n';

                //z++;

                // send cipher
                charsScribe = send( estabConnectFD, buffer3, (z-1), 0);
                if(charsScribe < 0 ){
                    // fprintf(stderr,"OTP_ENC_D: ERROR writing to socket\n"); 
                    exit(1);
                }



                /*   Brewster: "close sockets" 
                 */

                //  Brewster: "close socket established w/ client"
                close( estabConnectFD );

            }

        }
    }


    //  Brewster: "close listening socket"
    close( socktFD );


    exit(EXIT_SUCCESS);
}