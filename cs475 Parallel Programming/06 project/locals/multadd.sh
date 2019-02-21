#!/bin/bash

for locsz in 1 2 4 8 16 32 64 128 256 512 1024
do
	## -w --> suppress ISO warning
	g++ -DLOCAL_SIZE=$locsz -o multadd multadd.cpp C:/cs475/p6/nvidia/OpenCL.lib -lm -fopenmp -w
	./multadd
done