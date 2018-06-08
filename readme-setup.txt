How to setup piranha new for 64bit using mingw64/MSYS2.
and Eclipse

precondition:
git installed
Eclipse with CDT installed
set MINGW_HOME to your installation of MSYS2\MINGW (the mingw64 or mingw32 directory. For Piranha the mingw64 should be used) This is an application that needs 64bits


piranha:
pull in latest version using git to local location
Repository: https://github.com/bluescarni/piranha.git

for speeding up build in Eclipse set MAKEFLAGS in project properties Environment to -j 8 (for 8 cores)

CMAKE: // should we ue the version that comes for MSYS2? could resolve the issues with some variables not found
install latest version from
https://cmake.org/

to use clang with cmake and 64bit host set 
in compiler selection (-T option)
v141_clang_c2,host=x64

set source dirctory to where the local copy of the repository is i.e. the highest level CMakeList.txt file (in the piranha folder).
set the build directory to a sibling level e.g "build" (for eclipse)
The problem is the git installation. It has sh.exe in the path which collides with the generation of MSYS make filed for eclipse CDT.

No longer true: (Neon.3) choose UNIX makefile for Eclipse. Currently Msys2 doesn't seem to be recognized by Eclipse and the toolchain automatically set up. Needs more investigation. Is there a trick todo it?
in Neon.3 it seems to recognize MINGW_HOME and sets up the toolchain.
run configure.
All kinds of error messages will show up. see below for waht to install and how.


-- using the cmake version from MYSYS2 to generate MSYS Makefiles works with make in MYSY-wingw64 console
cd tests; make -j8
make test

MSYS2:
++++++
install MSYS2 and and follow its update process until done.
http://msys2.github.io/

https://sourceforge.net/projects/msys2/

set MINGW_HOME to your installation of MSYS2\MINGW (the mingw64 or mingw32 diretory. For Piranha the mingw64 should be used) This is an application that needs 64bits

For development we also need additonal packages (currently MSYS2 doesn't seem to have a graphic install tool)
We need packages:
	base-devel
	mingw-w64-x86_64-toolchain
        mingw-w64-x86_64-boost
	
	install with (using MSYS2 console/always for installs/updates):
	
	pacman --needed -Syu <packagename> 
	
	until done. The prompts will tell if something is todo.


- Default install for many tools/libraries will be usr/local which on Windows 7 translates in a standart installation for MSYS2 to "C:\msys64\usr\local".

bzip2 / get using pacman
+++++
get the source code from
http://www.bzip.org/


Boost: //get using pacman
++++++
http://www.boost.org/
Look for getting started guide
on Windows
not all libraries to be compiled are needed. 
download and expand into a separate directory and build
using bjam (in plain windows for VC 15 Norton removes b2.exe after bootstrap, claims to have a bad reputation (in boost 1.60)
bjam is bit identical to bjam(?)

-a: rebuild independent of current status, -j8 run 8jobs in parallel 
-enter the proper path that points to the bzip2 library source as Boost.build variable using -s BZIP2_SOURCE=...

./bjam.exe -a -j8 cxxflags="-std=c++11" toolset=gcc variant=release,debug link=shared threading=multi address-model=64 -s BZIP2_SOURCE==D:/Dev/bzip2/bzip2-1.0.6 --with-serialization --with-iostreams --with-test --with-filesystem --with-system --with-timer --with-chrono --with-python --with-regex stage

how to build the Python libs?? do I have to set a Boost.Build variable ??? like bzip2

To clean:
./bjam --clean-all toolset=msvc,gcc --build-type=complete stage

GMP:
++++
https://gmplib.org/
	./configure
	make
	make check
	make install
should buil everything and install in usr/local


MPFR:
+++++
?? do we need this?


This description is for Eeclipse Oxygen (4.7.0)
// for mp++
install msys2/mingw64 
gmp and mpfr are part of this installation
Flint is not has to be compiled separately
Mp++ uses "Catch" as testing frame work. currently 1.9.7 see https://github.com/philsquared/Catch


see above

run cmake with srcroot (i.e. where the base CmakeList.txt file is) and scroot-build on the same level
i.e.  ..abc/srcroot
      ..abc/srcroot-build

during configuration use the gmp mpfr python boost libraries that are under mingw64/  (consistency)

generate for eclipse CDT- MinGW Makefiles

in eclipse: FIle-Import-general-Existing projects into workspace       from srcroot-build

in eclipse package explorer top level of project (here mp++@mppp-build) right click select build-targets->build.. (shift-F9) and select what to run
or better open Project explorer. This will show Build targets

For the projects go to "Build Targets" and for targets "all" and "clean" right-click and edit target. Under section "Build Command" add -j8 to the "Build command line".
Don't use "Builder Settings" build will fail without additional settings.

 
 
===========================================================
For windows:
We need mpir, mpfr and mp++

git@github.com:wbhart/mpir.git
https://github.com/BrianGladman/mpfr.git
There is a dependency of the location of mpir in realtion to mpfr. See corresponding readmes

git@github.com:bluescarni/mppp.git

mpir and mpfr provide pre-set VS projects. 
in mpir change code-generator setting to /MD (from /MT) for lib_mpir
Build mpir lib_mpir for the processor wanted (for i7 its skylark)

in mpir change code-generator setting to /MD (from /MT) for lib_mpir
build mpfr lib-mpfr  

run cmake for mp++ and configure for VS2017
set an install directory (CMAKE_INSTALL_PREFIX), One has to run the INSTALL project otherwise some files are missing. If not set it will try to install in "Progrem Files"
Build all
RUN_TESTS

set CMAKE_PREFIX_PATH to mp++ install directory (in order to find some cmake files that configure Mp++)
For clang c2 chose -T v141_clang_c2 to select he clang toolset

set it to D:\Dev\Astronomy\mp++;D:\Dev\boost\boost_1_67_0;D:\Dev\boost\boost_1_67_0\stage;D:\Dev\mpir\dll\x64\Release;D:\Dev\mpfr\dll\x64\Release
