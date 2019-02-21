/*  Name:  James Fitzwater
 *  Email: fitzwatj@onid.oregonstate.edu
 *  Class: CS 475.400
 *  Assignment: Project #5
 */


#include <iostream>
#include<string>
#include<cstdlib>
#include<cstdio>
#include <stdio.h>
#include <stdlib.h>

#include"simd.p5.h"



#define NUMTRIES 4


float a[ARYSZ];
float b[ARYSZ];
float c[ARYSZ];


void testOps()
{

    clock_t clockBegin,
            clockEnd,
            clockDiff;

    time_t timeBegin,
           timeEnd,
           timeDiff;

    double time0,
           time1,
           time2;

    double megaMults    = 0.;
    double avgMegaMults = 0.;
    double maxMegaMults = 0.;
    double sumMegaMults = 0.;

    omp_set_num_threads( omp_get_max_threads() );

    unsigned long len = ARYSZ;

    float d;


    for( unsigned long yy = 0; yy < ARYSZ; yy++ ){
        a[yy] = 0.;
        b[yy] = 0.;
        c[yy] = 0.;
    }


    for( unsigned long xx = 0; xx < 6; xx++ )
    {

        for( unsigned long t = 0; t < NUMTRIES; t++ )
        {

            clockBegin = clock();
            timeBegin = time(0);
            time0 = omp_get_wtime( );

            switch( xx )
            {
                case 0:
                {

                    SimdMul( a, b, c, len );
                    if( t == (NUMTRIES - 1) ){
                        std::cout << "\n- SIMD SSE MULT " << ARYSZ <<" -" << std::endl;
                    }
                    break;
                }
                case 1:
                {

                    d = SimdMulSum( a, b, len );
                    if( t == (NUMTRIES - 1) ){
                        std::cout << "\n- SIMD SSE MULT'n'SUM " << ARYSZ <<" - sum: " << d << std::endl;
                    }
                    break;
                }
                case 2:
                {

                    MyMul( a, b, c, len );
                    if( t == (NUMTRIES - 1) ){
                        std::cout << "\n- MY MULT " << ARYSZ <<" -" << std::endl;
                    }
                    break;
                }
                case 3:
                {

                    d = MyMulSum( a, b, len );
                    if( t == (NUMTRIES - 1) ){
                        std::cout << "\n- MY MULT'n'SUM " << ARYSZ <<" - sum: " << d << std::endl;
                    }
                    break;
                }
                case 4:
                {

                    d = ReductMyMulSum( a, b, len );
                    if( t == (NUMTRIES - 1) ){
                        std::cout << "\n- REDUCT MY MULT " << ARYSZ <<" - sum: " << d << std::endl;
                    }
                    break;
                }
                case 5:
                {

                    d = ReductSimdMulSum( a, b, len );
                    if( t == (NUMTRIES - 1) ){
                        std::cout << "\n- REDUCT SIMD MULT'n'SUM " << ARYSZ <<" - sum: " << d << std::endl;
                    }
                    break;
                }
                default:
                {
                    std::cout << "\n\tDEFAULT INPUT ERROR\n" << std::endl;
                }
            }

            clockEnd = clock();
            timeEnd = time(0);
            time1 = omp_get_wtime( );

            timeDiff = (timeBegin - timeEnd);
            clockDiff = (clockEnd - clockBegin);
            time2= (time1-time0);

            megaMults = (double)ARYSZ/(time1-time0)/1000000.;

            sumMegaMults += megaMults;

            if( megaMults > maxMegaMults )
            {
                maxMegaMults = megaMults;
            }
        }


        avgMegaMults = sumMegaMults/(double)NUMTRIES;

        std::cout << "   Array Size = ";
        printf( "%d", ARYSZ );
        std::cout << "\n" << std::endl;

        std::cout << "   Peak Performance = ";
        printf( "%8.2lf", maxMegaMults );
        std::cout << " MegaMults/Sec" << std::endl;

        std::cout << "Average Performance = ";
        printf( "%8.2lf", avgMegaMults );
        std::cout << " MegaMults/Sec" << std::endl;

        std::cout << "Clock diff: ";
        printf("%d;",clockDiff,((double)clockDiff)/CLOCKS_PER_SEC);
        std::cout << "\nTime diff: " << timeDiff << std::endl;
        std::cout << "\nTime OMP diff: " << time2 << std::endl;
    }


/**
    for( unsigned long yy = 0; yy < ARYSZ; yy++ ){
        free(a[yy]);
        free(b[yy]);
        free(c[yy]);
    }
**/
/**
    free(a);
    free(b);
    free(c);
**/
}

void MyMul( float *a, float *b,   float *c,   int len )
{
	int myLimit = ( len/SSE_WIDTH ) * SSE_WIDTH;

	for( int x = myLimit; x < len; x++ )
	{
		c[x] = a[x] * b[x];
	}
}


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


void SimdMul( float *a, float *b,   float *c,   int len )
{
	int limit = ( len/SSE_WIDTH ) * SSE_WIDTH;
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

	for( int i = limit; i < len; i++ )
	{
		c[i] = a[i] * b[i];
	}
}


float SimdMulSum( float *a, float *b, int len )
{
	float sum[4] = { 0., 0., 0., 0. };
	int limit = ( len/SSE_WIDTH ) * SSE_WIDTH;

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

	for( int i = limit; i < len; i++ )
	{
		sum[i-limit] += a[i] * b[i];
	}

	return sum[0] + sum[1] + sum[2] + sum[3];
}


/**
void ReductMyMul( float *a, float *b,   float *c,   int len )
{
	int myLimit = ( len/SSE_WIDTH ) * SSE_WIDTH;

	for( int x = myLimit; x < len; x++ )
	{
		c[x] = a[x] * b[x];
	}
}
**/


/**
void ReductSimdMul( float *a, float *b,   float *c,   int len )
{
	int limit = ( len/SSE_WIDTH ) * SSE_WIDTH;
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

	for( int i = limit; i < len; i++ )
	{
		c[i] = a[i] * b[i];
	}
}
**/


float ReductMyMulSum( float *a, float *b, int len )
{
	float sum[4] = { 0., 0., 0., 0. };
	int limit = ( len/SSE_WIDTH ) * SSE_WIDTH;

    #pragma omp parallel for reduction(+:sum)
	for( int x = limit; x < len; x++ )
	{
		sum[x-limit] += a[x] * b[x];
	}

	return sum[0] + sum[1] + sum[2] + sum[3];
}


float ReductSimdMulSum( float *a, float *b, int len )
{
	float sum[4] = { 0., 0., 0., 0. };
	int limit = ( len/SSE_WIDTH ) * SSE_WIDTH;

	__asm
	(
		".att_syntax\n\t"
		"movq    -40(%rbp), %rbx\n\t"		// a
		"movq    -48(%rbp), %rcx\n\t"		// b
		"leaq    -32(%rbp), %rdx\n\t"		// &sum[0]
		"movups	 (%rdx), %xmm2\n\t"		// 4 copies of 0. in xmm2
	);

/**
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

    #pragma omp parallel for reduction(+:sum)
	for( int i = limit; i < len; i++ )
	{
		sum[i-limit] += a[i] * b[i];
	}

	return sum[0] + sum[1] + sum[2] + sum[3];
}





int main()
{
    testOps();


    return 0;
}
