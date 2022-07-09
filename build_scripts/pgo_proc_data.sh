#!/bin/bash
# the first arg is the compiler
# the second arg is the compiler version

COMPILER="$1"
if [ "$COMPILER" != "Clang" ] ; then
      exit 1 # only clang needs this processing step
fi
COMPILER_MAJOR_VERSION=`echo "$2" | cut -d. -f1`

# if for some strange reason the file isn't generated, then the compilation step later will fail
ls default*.profraw 2>/dev/null
if [ "$?" != 0 ]; then
    exit 0
fi

LLVM_PROFDATA="llvm-profdata-$CLANG_MAJOR_VERSION"
command -v "$LLVM_PROFDATA" > /dev/null
if [ $? -ne 0 ] ; then
      # fallback
      LLVM_PROFDATA="llvm-profdata"
fi

"$LLVM_PROFDATA" merge -output default.profdata default*.profraw
