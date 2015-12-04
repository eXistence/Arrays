#!/bin/sh

BUILD_DIR=build/gcc/debug
COMPILER="gcc"
BUILDSYSTEM="make"
BUILDTYPE="debug"

CMAKE_BUILD_TYPE="Debug"
CMAKE_GENERATOR="Sublime Text 2 - Unix Makefiles"

if [ "$#" -ne "3" -a "$#" -ne "2" ]; then
	echo "Usage run_cmake.sh <gcc|clang> <debug|release> <libstdc++|libc++>"
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

	if [ "$2" = "debug" ]; then
		BUILDTYPE="debug"
		CMAKE_BUILD_TYPE="Debug"
	fi

	if [ "$2" = "release" ]; then
		BUILDTYPE="release"
		CMAKE_BUILD_TYPE="Release"
	fi	
fi

BUILDDIR=build/$COMPILER/$BUILDTYPE
ROOTDIR=../../..

if [ "$1" = "clang" -a "$#" -ge "3" ]; then	
	echo "lets use clang with "$3
	CXX_FLAGS=-DCMAKE_CXX_FLAGS="-stdlib=$3"
	BUILDDIR=$BUILDDIR/$3
	ROOTDIR=../../../..
fi

mkdir -p $BUILDDIR
cd $BUILDDIR

echo "CXX_FLAGS="$CXX_FLAGS

cmake -G"$CMAKE_GENERATOR" -DCMAKE_BUILD_TYPE=$CMAKE_BUILD_TYPE -DCMAKE_CXX_FLAGS=" -stdlib=libc++ " $ROOTDIR
