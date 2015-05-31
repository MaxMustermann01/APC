#
# Exercise 3 - Makefile
# AUTHORS  Sven Dorkenwald
#          Günther Schindler
#

CC=@g++
CFLAGS = -O3 -lm -Wall -Wextra 
LDLIBS = -pthread

# Optimization Flags:
# -O1    : Optimizing compilation takes somewhat more time, and a lot more memory for a 
#          large function.
# -02    : Optimize even more.
# -03    : Optimize yet more. -O3 turns on all optimizations specified by -O2 and more
# -Ofast : Disregard strict standards compliance. -Ofast enables all -O3 optimizations. It 
#         also enables optimizations that are not valid for all standard-compliant programs.

#CFLAGS += -O1

ll presum.cpp:
	$(CC) $(CFLAGS) $(LDLIBS) presum.cpp \
					time_measurement.c \
					-o presum

clean :
	rm -f presum
