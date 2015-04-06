#!/bin/bash

if [ $# -ne 1 ]; then
	echo "path to CC65 expected"
	exit 1
fi

if [ ! -d build ]; then
	mkdir build
fi

cp $1/libsrc/runtime/* build/
cp $1/libsrc/common/* build/

# replace files
cp fgets.c build

for f in build/*.c; do
	cc65 -t none -O -I ../include $f -o $f.s
done

for f in build/*.s; do
	ca65 $f -o $f.o
done

ar65 a rt.lib build/*.o

rm -rf build

exit 0

