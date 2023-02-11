#!/bin/bash

cd ..

rm -Rf glee
git clone https://github.com/Franticware/glee
cd glee
cmake -DCMAKE_TOOLCHAIN_FILE="../mingw32/mingw32-toolchain.cmake" \
-S. -B. \
-DCMAKE_BUILD_TYPE:STRING="MinSizeRel"
make
cd ..

SDL2VER=2.26.1
rm -Rf SDL2
rm -f SDL2-devel-$SDL2VER-mingw.tar.gz SDL2.dll
wget https://libsdl.org/release/SDL2-devel-$SDL2VER-mingw.tar.gz
tar -xvf SDL2-devel-$SDL2VER-mingw.tar.gz
mv SDL2-$SDL2VER SDL2
cp SDL2/i686-w64-mingw32/bin/SDL2.dll .

git clone https://github.com/g-truc/glm.git

: <<'END'
LIBJPEGVER=9e
rm -Rf jpeg
rm -f jpegsrc.v$LIBJPEGVER.tar.gz
wget https://www.ijg.org/files/jpegsrc.v$LIBJPEGVER.tar.gz
tar -xvf jpegsrc.v$LIBJPEGVER.tar.gz
mv jpeg-$LIBJPEGVER jpeg
cd jpeg
./configure --host=i686-w64-mingw32
make
cd ..
END

rm -Rf libjpeg-turbo
git clone https://github.com/libjpeg-turbo/libjpeg-turbo.git
cd libjpeg-turbo
cmake -DCMAKE_TOOLCHAIN_FILE="../mingw32/mingw32-toolchain.cmake" \
-S. -B. \
-DCMAKE_GNUtoMS:BOOL="0" -DCMAKE_BUILD_TYPE:STRING="MinSizeRel" -DCMAKE_INSTALL_PREFIX:PATH="." -DCMAKE_SYSTEM_PROCESSOR:STRING="" -DWITH_ARITH_DEC:BOOL="0" -DCMAKE_INSTALL_LIBDIR:PATH="lib" -DWITH_SIMD:BOOL="1" -DWITH_TURBOJPEG:BOOL="0" -DPKGNAME:STRING="libjpeg-turbo" -DBUILD:STRING="20221228" -DWITH_ARITH_ENC:BOOL="0" -DENABLE_SHARED:BOOL="0" 
make
cd ..

rm -Rf zlib
git clone https://github.com/madler/zlib
cd zlib
cmake -DCMAKE_TOOLCHAIN_FILE="../mingw32/mingw32-toolchain.cmake" \
-S. -B. \
-DLIBRARY_OUTPUT_PATH:PATH="" -DINSTALL_LIB_DIR:PATH="/usr/local/lib" -DCMAKE_BUILD_TYPE:STRING="MinSizeRel" -DCMAKE_INSTALL_PREFIX:PATH="/usr/local" -DINSTALL_MAN_DIR:PATH="/usr/local/share/man" -DINSTALL_PKGCONFIG_DIR:PATH="/usr/local/share/pkgconfig" -DEXECUTABLE_OUTPUT_PATH:PATH="" 
make
cd ..

rm -Rf libpng
git clone https://github.com/glennrp/libpng
cd libpng
cmake -DCMAKE_TOOLCHAIN_FILE="../mingw32/mingw32-toolchain.cmake" \
-S. -B. \
-DZLIB_INCLUDE_DIRS:PATH="../zlib" -DPNG_SHARED:BOOL="0" -DCMAKE_INSTALL_PREFIX:PATH="/usr/local" -DCMAKE_BUILD_TYPE:STRING="MinSizeRel" -DPNG_PREFIX:STRING="" -DPNG_BUILD_ZLIB:BOOL="1" 
make
cd ..
