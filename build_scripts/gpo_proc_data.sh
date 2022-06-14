#!/bin/bash

# used for gpo. used in the cmakelists
# for g++ this is a no-op

ls default*.profraw && llvm-profdata merge -output default.profdata default*.profraw
