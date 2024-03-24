#!/bin/bash
rm -rf OpenMRac
rm -rf OpenMRac-data
rm -rf release
rm *.exe
rm *.dat
read -p "Press enter to continue, Ctrl+C to quit"
mkdir release
mkdir release/openmrac
cp readme.txt release/openmrac || exit
git clone https://github.com/Franticware/OpenMRac-data.git
cd OpenMRac-data
make
cp openmrac.dat ../release/openmrac || exit
cd ..
git clone https://github.com/Franticware/OpenMRac.git --branch dos-3dfx --recursive
cd OpenMRac/src/djgpp/
./prereq.sh
cd ..
make -f Makefile.djgpp
cp openmrac.exe set3dfx.exe ../../release/openmrac || exit
cd ../../release
7z a openmrac-$(date +'%Y%m%d')-dos openmrac
cd ../OpenMRac
git log -1 | cat
cd ..
