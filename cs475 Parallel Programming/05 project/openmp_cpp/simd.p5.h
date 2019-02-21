#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <ctime>
#include <sys/time.h>
#include <sys/resource.h>

#include <omp.h>

#ifndef SIMD_H
#define SIMD_H

// SSE stands for Streaming SIMD Extensions

#define SSE_WIDTH	4

#define ALIGNED		__attribute__((aligned(16)))

void    testOps( );

void	MyMul(    float *, float *,  float *, int );
float	MyMulSum( float *, float *, int );

void	SimdMul(  float *, float *,  float *, int );
float	SimdMulSum( float *, float *, int );

float	ReductMyMulSum(   float *, float *, int );
float	ReductSimdMulSum( float *, float *, int );

extern float a[];
extern float b[];
extern float c[];


#endif		// SIMD_H
