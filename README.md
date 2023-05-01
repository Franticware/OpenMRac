# OpenMRac

![Screenshot](media/openmrac-3dfx-0.png)

OpenMRac is a split-screen racing game. It is a tweaked source release of [MultiRacer](https://www.franticware.com/multiracer).

Programming was done by Vojtěch Salajka.
Porting to big endian architectures for Amiga-like OSes was done by [Szilárd Biró](https://github.com/BSzili).

⚠️ Beware! The source code is old and messy, plus most comments are in Czech 😁

Creating forks and porting to additional platforms is encouraged, but these typically will not be merged back to the main repo. The same applies to mods.

Franticware claims rights to the name "MultiRacer" which should not be used by other parties for their products or ports. That is the reason for changing the title to OpenMRac, to which no such restrictions apply.

Game data files are in a separate repository under a different license: https://github.com/Franticware/OpenMRac-data

# DOS 3dfx Version

This branch contains DOS/DJGPP port of OpenMRac. It was tested working on DOS with 3dfx Voodoo 1, 486DX2 66 MHz, where it runs very slowly with audio stuttering, but seems stable. The screenshots were taken on real hardware.

It was also tested in DOSBox-X, where it is unstable. When it runs, the audio is OK.

![Screenshot](media/openmrac-3dfx-1.png)
