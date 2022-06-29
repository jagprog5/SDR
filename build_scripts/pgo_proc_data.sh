#!/bin/bash
# used for pgo in the CMakeLists.txt

# if not using clang then this file doesn't exists
# if using clang but for some strange reason the file isn't generated, then the compilation step later will fail
ls default*.profraw 2>/dev/null
if [ "$?" != 0 ]; then
    exit 0
fi

llvm-profdata merge -output default.profdata default*.profraw
