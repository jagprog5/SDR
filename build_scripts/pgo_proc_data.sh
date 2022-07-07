#!/bin/bash
# used for pgo in the CMakeLists.txt
# the arg is the CMAKE_CXX_COMPILER_VERSION. it prevents a version conflict if multiple llvm-profdata's are installed

# if not using clang then this file doesn't exists
# if using clang but for some strange reason the file isn't generated, then the compilation step later will fail
ls default*.profraw 2>/dev/null
if [ "$?" != 0 ]; then
    exit 0
fi

CLANG_MAJOR_VERSION=`echo "$1" | cut -d. -f1` # get major number from version
if [ -z "$CLANG_MAJOR_VERSION" ] ; then
      LLVM_PROFDATA="llvm-profdata"
else
      LLVM_PROFDATA="llvm-profdata-$CLANG_MAJOR_VERSION"
      command -v "$LLVM_PROFDATA" > /dev/null
      if [ $? -ne 0 ] ; then
            # fallback
            LLVM_PROFDATA="llvm-profdata"
      fi
fi

"$LLVM_PROFDATA" merge -output default.profdata default*.profraw
