#!/bin/bash
# used for pgo in the CMakeLists.txt
# the first arg is the CMAKE_CXX_COMPILER_VERSION. This makes sure the correct llvm-profdata is being used

# if not using clang then this file doesn't exists
# if using clang but for some strange reason the file isn't generated, then the compilation step later will fail
ls default*.profraw 2>/dev/null
if [ "$?" != 0 ]; then
    exit 0
fi

# llvm-profdata is finicky for the version of clang that is being used
# this makes the pipeline more robust
CLANG_MAJOR_VERSION=`echo "$1" | cut -d. -f1` # get major number from version
if [ -z "$CLANG_MAJOR_VERSION" ] ; then
      LLVM_PROFDATA="llvm-profdata"
else
      LLVM_PROFDATA="llvm-profdata-$CLANG_MAJOR_VERSION"
fi

"$LLVM_PROFDATA" merge -output default.profdata default*.profraw
