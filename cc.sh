#! /bin/sh

CFLAGS="-Wall -g -c"
LFLAGS="-lrt -lm"

echo "compiling ..."
gcc $CFLAGS farmer.c || exit
gcc $CFLAGS worker.c || exit
gcc $CFLAGS md5s.c || exit
gcc $CFLAGS md5s_main.c || exit

echo "linking ..."
gcc -o farmer farmer.o $LFLAGS || exit
gcc -o worker worker.o md5s.o $LFLAGS || exit
gcc -o md5s md5s_main.o md5s.o $LFLAGS || exit

echo "creating expected output ..."
grep UINT128 settings.h | sed "s/.*'\(.*\)'.*/'\1'/" > expected_output.txt

