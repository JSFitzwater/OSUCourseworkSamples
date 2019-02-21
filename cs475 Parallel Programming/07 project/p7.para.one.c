/*  Name:  James Fitzwater
 *  Email: fitzwatj@onid.oregonstate.edu
 *  Class: CS 475.400
 *  Assignment: Project #7
 */

#include <omp.h>
#include <stdio.h>
#include <stdlib.h>


#define SIZE 32768

#define NUMTRIES 10


// GLOBAL DECLARATIONS
struct s
{
    float value;
} ary[2*SIZE];

float arSums[1*SIZE];


void autoCorr( /* ... */ )
{

    float sum;

    #pragma omp parallel for private(sum)
    for( int shift = 0; shift < SIZE; shift++ )
    {
        sum = 0.;
        for( int i = 0; i < SIZE; i++ )
        {
            sum = sum + ( ary[i].value * ary[i + shift].value );
        }
        arSums[shift] = sum; // note the "fix #2" from false sharing if you are using OpenMP
    }
}


// MAIN()
int main()
{




//.....//

    int fpsz;
    float flt;
    FILE* fp;
    int i;

//.....//

    omp_set_num_threads( 1 );



    fp = fopen( "signal.txt", "r" );
    if( fp == NULL )
    {
        fprintf( stderr, "Cannot open file 'signal.txt'\n" );
        exit( 1 );
    }

    fscanf( fp, "%d", &fpsz );

/* define size based off script input */

        fpsz = SIZE;

/* ----------------------------------- */

    for( i = 0; i < fpsz; i++ )
    {
        fscanf( fp, "%f", &flt );
        ary[i].value = flt;
        ary[i+fpsz].value = flt; // duplicate the array
    }
    fclose( fp );


//.....//

    float kmps = 0;

    double time2 = 0.;

    for( unsigned long xx = 0; xx < NUMTRIES; xx++ )
    {
        double time0 = omp_get_wtime( );


        autoCorr( /*****/ );

        double time1 = omp_get_wtime( );

        time2 += (time1 - time0);

        kmps += (double)SIZE/(time1-time0)/1000.0;

    }

    printf("\nparallel-one,%6f,%6f,%10f", (time2/NUMTRIES),SIZE, (kmps/NUMTRIES));

//.....//

    return 0;
}



// ****************** //
// ****************** //
