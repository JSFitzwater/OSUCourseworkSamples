
/*  Name:  James Fitzwater
 *  Email: fitzwatj@onid.oregonstate.edu
 *
 *  Assignment No. 4
 *  Class: CS 344.400 
 *  Date: 
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <ctype.h>  
#include <fcntl.h>

// 65 - 90   upper
// 97 - 122  lower



char* main(int argc, char* argv[]){

    int x,numc,seed;
    FILE* keyfile;
    char* keygenfile[2056];

    // prep rand() op
    seed = time(0);
    srand(seed);

    // get num of characters
       numc = atoi(argv[1]);

    // create/open file w/ write flag
//     keyfile = fopen( argv[3], "w" );

    // generate random characters &
    // write-out characters to file
    x=0;
    while(x < numc+1){
        if(x <= numc){
            printf("%c",((rand() % (90-65)) + 65));
        }else{
            printf("\n");
        }
        x++;
    }
      

    // close file - done
//     fclose(keyfile);

    return 0;
}