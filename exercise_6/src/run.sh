#!/bin/bash

THREADS="1 2 4 8 12 16 24 32 40 48"

for N in $THREADS
	do
		numactl --interleave=all ./presum 16777216 $N >> out_interleave.txt;
	done
