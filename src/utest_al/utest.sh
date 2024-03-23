CFLAGS="-I.. -std=gnu++11 -nostdlib -m32 -ffreestanding -O2 -Wall -Wextra -fno-exceptions -DNDEBUG "

LFLAGS="-s -lm -no-pie"

CC=i586-pc-msdosdjgpp-gcc
CXX=i586-pc-msdosdjgpp-g++
LINK=i586-pc-msdosdjgpp-g++
EXE2COFF=~/djgpp/i586-pc-msdosdjgpp/bin/exe2coff
INTERM=interm
TARGET=utest_al.exe

rm $TARGET
rm *.djgpp.o
rm $INTERM.exe
rm $INTERM

$CXX -c $CFLAGS -o main.djgpp.o main.cpp || exit
$CXX -c $CFLAGS -o minial.djgpp.o ../minial.cpp || exit
$CXX -c $CFLAGS -o minial_gus.djgpp.o ../minial_gus.cpp || exit
$CXX -c $CFLAGS -o minial_sb.djgpp.o ../minial_sb.cpp || exit
$CXX -c $CFLAGS -o gus.djgpp.o ../gus.cpp || exit
$CXX -c $CFLAGS -o sb.djgpp.o ../sb.cpp || exit

$LINK -o $INTERM.exe  $LFLAGS main.djgpp.o minial.djgpp.o minial_gus.djgpp.o minial_sb.djgpp.o gus.djgpp.o sb.djgpp.o || exit
$EXE2COFF $INTERM.exe || exit
cat ../CWSDSTUB.EXE $INTERM > $TARGET

#dosbox-x -c "mount C ." -c "C:" -c "utest_al.exe sb"
dosbox-x -c "mount C ." -c "C:" -c "utest_al.exe gus"

#dosbox-x -c "mount C ." -c "C:" -c "echo SB:" -c "utest_al.exe sb" -c "echo GUS:" -c "utest_al.exe gus"
