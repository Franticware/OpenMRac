#!/bin/bash

TOOLCHAIN_FILE=djgpp-toolchain.cmake

LIBJPEGVER=9e
rm -Rf jpeg
rm -f jpegsrc.v$LIBJPEGVER.tar.gz
wget https://www.ijg.org/files/jpegsrc.v$LIBJPEGVER.tar.gz
tar -xvf jpegsrc.v$LIBJPEGVER.tar.gz
mv jpeg-$LIBJPEGVER jpeg
cd jpeg
CC=i586-pc-msdosdjgpp-gcc ./configure
make
cd ..

rm -Rf zlib
git clone https://github.com/madler/zlib
cd zlib
CC=i586-pc-msdosdjgpp-gcc ./configure
make
cd ..

rm -Rf libpng
git clone https://github.com/glennrp/libpng
cd libpng
cp ../zlib/zlib.h ../zlib/zconf.h .
cmake -DCMAKE_TOOLCHAIN_FILE=../$TOOLCHAIN_FILE \
-S. -B. \
-DZLIB_INCLUDE_DIRS:PATH="../zlib" -DPNG_SHARED:BOOL="0" -DCMAKE_INSTALL_PREFIX:PATH="/usr/local" -DCMAKE_BUILD_TYPE:STRING="MinSizeRel" -DPNG_PREFIX:STRING="" -DPNG_BUILD_ZLIB:BOOL="1" 
make
cd ..
