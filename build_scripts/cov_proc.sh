#!/bin/bash
# the first arg is the compiler
# the second arg is the compiler version

DIR="$(dirname "$(readlink -f "$0")")" # the directory that this script is in
COMPILER="$1"
COMPILER_MAJOR_VERSION=`echo "$2" | cut -d. -f1`

if [ "$COMPILER" == "Clang" ] ; then
    GCOV_TOOL="$DIR/llvm-gcov.sh COMPILER_MAJOR_VERSION"
else
    GCOV_TOOL="gcov-$COMPILER_MAJOR_VERSION"
    command -v "$GCOV_TOOL" > /dev/null
    if [ $? -ne 0 ] ; then
        # fallback
        GCOV_TOOL="gcov"
    fi
fi

lcov --gcov-tool "$GCOV_TOOL" --capture --directory "$DIR/.." --base-directory "$DIR/../include" --no-external --output-file "$DIR/../build/coverage.info"
