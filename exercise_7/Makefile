#
# Exercise 7 - Makefile
# AUTHORS  Sven Dorkenwald
#          Günther Schindler
#

CC=gcc
CFLAGS = -Wall
CFLAGS += -Wno-switch-enum
LDLIBS = -pthread
LDLIBS += -lm

# Optimization Flags:
# -O1    : Optimizing compilation takes somewhat more time, and a lot more memory for a 
#          large function.
# -02    : Optimize even more.
# -03    : Optimize yet more. -O3 turns on all optimizations specified by -O2 and more
# -Ofast : Disregard strict standards compliance. -Ofast enables all -O3 optimizations. It 
#         also enables optimizations that are not valid for all standard-compliant programs.

CFLAGS += -O3

count :
	$(CC) $(CFLAGS) 		src/main.c \
					src/rb_tree.c \
					src/time_measurement.c \
					-o rbtree \
					$(LDLIBS)

clean :
	rm -f rb_tree
