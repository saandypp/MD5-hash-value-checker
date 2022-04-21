#
#	Makefile for  (2INC0)  Interprocess Communication
#
#	(c) TUe 2016-2020, Joris Geurts
#       (c) TUe 2021, Richard Verhoeven
#

BINARIES = worker farmer md5s

CC = gcc
CFLAGS = -Wall -g -c
LDLIBS = -lrt -lm

all:	$(BINARIES) expected_output.txt

clean:
	rm -f *.o $(BINARIES)

worker: worker.o md5s.o

farmer: farmer.o

md5s: md5s.o md5s_main.o

worker.o: worker.c common.h

farmer.o: farmer.c settings.h common.h

expected_output.txt: settings.h
	grep UINT128 settings.h | sed "s/.*'\(.*\)'.*/'\1'/" > expected_output.txt
