#!/bin/bash


gcc p7.para.one.c -o p7paraone -lm -fopenmp
./p7paraone

gcc p7.para.n.c -o p7paratwo -lm -fopenmp
./p7paratwo

gcc p7.simd.c -o p7simd -lm -fopenmp
./p7simd

g++ -o p7gpu p7.gpu.cpp C:/cs475/p7/nvidia/OpenCL.lib -lm -fopenmp -w
./p7gpu

g++ -o p7sinewaveprint p7.sine.wave.print.cpp C:/cs475/p7/nvidia/OpenCL.lib -lm -fopenmp -w
./p7sinewaveprint
