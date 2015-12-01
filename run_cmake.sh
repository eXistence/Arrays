#!/bin/sh

BUILD_DIR=build/gcc/debug
COMPILER="gcc"
BUILDSYSTEM="make"
BUILDTYPE="debug"

CMAKE_BUILD_TYPE="Debug"
CMAKE_GENERATOR="Sublime Text 2 - Unix Makefiles"

if [ "$#" -ne "2" ]; then
	echo "Usage run_cmake.sh <gcc|clang> <debug|release>"
	exit 1
fi	

if [ "$#" -ge "1" ]; then	

	if [ "$1" = "gcc" ]; then
		export CXX="g++"
		export CC="gcc"
		COMPILER="gcc"
	fi

	if [ "$1" = "clang" ]; then
		export CXX="clang++"
		export CC="clang"		
		COMPILER="clang"
	fi	
fi

if [ "$#" -ge "2" ]; then	

	if [ "$3" = "debug" ]; then
		BUILDTYPE="debug"
		CMAKE_BUILD_TYPE="Debug"
	fi

	if [ "$3" = "release" ]; then
		BUILDTYPE="release"
		CMAKE_BUILD_TYPE="Release"
	fi	
fi

BUILDDIR=build/$COMPILER/$BUILDTYPE

mkdir -p $BUILDDIR
cd $BUILDDIR

cmake -G"$CMAKE_GENERATOR" -DCMAKE_BUILD_TYPE=$CMAKE_BUILD_TYPE ../../..
