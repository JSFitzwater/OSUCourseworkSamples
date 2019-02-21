/*  Name:  James Fitzwater
 *  Email: fitzwatj@onid.oregonstate.edu
 *  Class: CS 475.400
 *  Assignment: Project #5
 */

#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

#define SSE_WIDTH	4
#define ALIGNED		__attribute__((aligned(16)))



#define NUMTRIES 10


// GLOBAL DECLARATIONS
float a[ARYSZ];
float b[ARYSZ];
float c[ARYSZ];


// MY MULT
void MyMul( float *a, float *b,   float *c,   int len )
{
	int myLimit = ( len/SSE_WIDTH ) * SSE_WIDTH;

	for( int x = myLimit; x < len; x++ )
	{
		c[x] = a[x] * b[x];
	}
}


// MY MULT'N'SUM
float MyMulSum( float *a, float *b, int len )
{
	float sum[4] = { 0., 0., 0., 0. };
	int limit = ( len/SSE_WIDTH ) * SSE_WIDTH;

	for( int x = limit; x < len; x++ )
	{
		sum[x-limit] += a[x] * b[x];
	}

	return sum[0] + sum[1] + sum[2] + sum[3];
}


// SIMD MULT
void SimdMul( float *a, float *b,   float *c,   int len )
{
	int limit = ( len/SSE_WIDTH ) * SSE_WIDTH;

/**
	__asm
	(
		".att_syntax\n\t"
		"movq    -24(%rbp), %rbx\n\t"		// a
		"movq    -32(%rbp), %rcx\n\t"		// b
		"movq    -40(%rbp), %rdx\n\t"		// c
	);

	for( int i = 0; i < limit; i += SSE_WIDTH )
	{
		__asm
		(
			".att_syntax\n\t"
			"movups	(%rbx), %xmm0\n\t"	// load the first sse register
			"movups	(%rcx), %xmm1\n\t"	// load the second sse register
			"mulps	%xmm1, %xmm0\n\t"	// do the multiply
			"movups	%xmm0, (%rdx)\n\t"	// store the result
			"addq $16, %rbx\n\t"
			"addq $16, %rcx\n\t"
			"addq $16, %rdx\n\t"
		);
	}
**/

    omp_set_num_threads( 1 );

    #pragma omp simd
	for( int i = limit; i < len; i++ )
	{
		c[i] = a[i] * b[i];
	}
}


// SIMD MULT'N"SUM
float SimdMulSum( float *a, float *b, int len )
{
	float sum[4] = { 0., 0., 0., 0. };
	int limit = ( len/SSE_WIDTH ) * SSE_WIDTH;

/**
	__asm
	(
		".att_syntax\n\t"
		"movq    -40(%rbp), %rbx\n\t"		// a
		"movq    -48(%rbp), %rcx\n\t"		// b
		"leaq    -32(%rbp), %rdx\n\t"		// &sum[0]
		"movups	 (%rdx), %xmm2\n\t"		// 4 copies of 0. in xmm2
	);

	for( int i = 0; i < limit; i += SSE_WIDTH )
	{
		__asm
		(
			".att_syntax\n\t"
			"movups	(%rbx), %xmm0\n\t"	// load the first sse register
			"movups	(%rcx), %xmm1\n\t"	// load the second sse register
			"mulps	%xmm1, %xmm0\n\t"	// do the multiply
			"addps	%xmm0, %xmm2\n\t"	// do the add
			"addq $16, %rbx\n\t"
			"addq $16, %rcx\n\t"
		);
	}

	__asm
	(
		".att_syntax\n\t"
		"movups	 %xmm2, (%rdx)\n\t"	// copy the sums back to sum[ ]
	);
**/

    omp_set_num_threads( 1 );

    #pragma omp simd reduction(+:sum)
	for( int i = limit; i < len; i++ )
	{
		sum[i-limit] += a[i] * b[i];
	}

	return sum[0] + sum[1] + sum[2] + sum[3];
}


// TEST OPERATIONS
void testOps()
{
    double time0, time1,
           time2, time3,
           timeSIMDSSEMULT,
           timeSIMDSSEMULTSUM,
           timeMYMUL,
           timeMYMULSUM;

    double megaMults    = 0.;
    double avgMegaMults = 0.;
    double maxMegaMults = 0.;
    double sumMegaMults = 0.;

    double mmsSIMDSSEMULT = 0.;
    double mmsSIMDSSEMULTSUM = 0.;
    double mmsMYMUL = 0.;
    double mmsMYMULSUM = 0.;

    unsigned long len = ARYSZ;

    float d;


    for( unsigned long yy = 0; yy < ARYSZ; yy++ ){
        a[yy] = 0.;
        b[yy] = 0.;
        c[yy] = 0.;
    }


    for( unsigned long xx = 0; xx < 4; xx++ )
    {
        time2 = 0.;
        sumMegaMults = 0.;

        for( unsigned long t = 0; t < NUMTRIES; t++ )
        {
            switch( xx )
            {
                case 0:
                {
                    time0 = omp_get_wtime( );

                    // --
                    SimdMul( a, b, c, len );
                    // --

                    time1 = omp_get_wtime( );
                    time2 = time2 + (time1-time0);
                    sumMegaMults = sumMegaMults + ( (double)ARYSZ/(time1-time0)/1000000. );

                    if( t == (NUMTRIES - 1) ){
                        timeSIMDSSEMULT = time2/NUMTRIES;
                        mmsSIMDSSEMULT = sumMegaMults/NUMTRIES;

                        printf( "\n- %d - SIMD SSE MULT -\n    time: %6f\n    mms: %6f", ARYSZ, timeSIMDSSEMULT, mmsSIMDSSEMULT);
                    }
                    break;
                }
                case 1:
                {
                    time0 = omp_get_wtime( );

                    // --
                    d = SimdMulSum( a, b, len );
                    // --

                    time1 = omp_get_wtime( );
                    time2 = time2 + (time1-time0);
                    sumMegaMults = sumMegaMults + ( (double)ARYSZ/(time1-time0)/1000000. );



                    if( t == (NUMTRIES - 1) ){
                        timeSIMDSSEMULTSUM = time2/NUMTRIES;
                        mmsSIMDSSEMULTSUM = sumMegaMults/NUMTRIES;

                        printf( "\n- %d - SIMD SSE MULT'n'SUM -\n    time: %6f\n    mms: %6f", ARYSZ, timeSIMDSSEMULTSUM, mmsSIMDSSEMULTSUM);
                    }
                    break;
                }
                case 2:
                {
                    time0 = omp_get_wtime( );

                    // --
                    MyMul( a, b, c, len );
                    // --

                    time1 = omp_get_wtime( );
                    time2 = time2 + (time1-time0);
                    sumMegaMults = sumMegaMults + ( (double)ARYSZ/(time1-time0)/1000000. );

                    if( t == (NUMTRIES - 1) ){
                        timeMYMUL = time2/NUMTRIES;
                        mmsMYMUL = sumMegaMults/NUMTRIES;

                        printf( "\n- %d - MY MULT -\n    time: %6f\n    mms: %6f", ARYSZ, timeMYMUL, mmsMYMUL);
                    }
                    break;
                }
                case 3:
                {
                    time0 = omp_get_wtime( );

                    // --
                    d = MyMulSum( a, b, len );
                    // --

                    time1 = omp_get_wtime( );
                    time2 = time2 + (time1-time0);
                    sumMegaMults = sumMegaMults + ( (double)ARYSZ/(time1-time0)/1000000. );

                    if( t == (NUMTRIES - 1) ){
                        timeMYMULSUM = time2/NUMTRIES;
                        mmsMYMULSUM = sumMegaMults/NUMTRIES;

                        printf( "\n- %d - MY MULT'n'SUM -\n    time: %6f\n    mms: %6f", ARYSZ, timeMYMULSUM, mmsMYMULSUM);
                    }
                    break;
                }
                default:
                {
                    printf("\n\tDEFAULT INPUT ERROR\n");
                }
            }
        }
    }

    printf("\n\nSpeedup Mult - Time: %6f", (timeMYMUL/timeSIMDSSEMULT));
    printf("\nSpeedup Mult - MMs: %6f", (mmsSIMDSSEMULT/mmsMYMUL));
    printf("\nSpeedup Mult'n'Sum - Time: %6f", (timeMYMULSUM/timeSIMDSSEMULTSUM));
    printf("\nSpeedup Mult'n'Sum - MMs: %6f\n", (mmsSIMDSSEMULTSUM/mmsMYMULSUM));
}


// MAIN()
int main()
{
    testOps();


    return 0;
}
