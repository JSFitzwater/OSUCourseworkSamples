#!/bin/bash

## MODIFIED USING PROJECT NOTES HANDOUT SUPPLEMENT

# chunk size
for ary in 1000 1600 3200 6400 10000 16000 32000 64000 100000 160000 320000 640000 1000000 1600000 3200000 6400000 1000000 1600000 3200000
do
    ## g++ -DARYSZ=$ary -c simd.p5.cpp -o simd.p5.o
    ## g++ -DARYSZ=$ary -o proj5 proj5.cpp simd.p5.o -lm -fopenmp
    ## rm simd.p5.o
    g++ -DARYSZ=$ary simd.p5.cpp -o proj5 -lm -fopenmp
    ./proj5
done
