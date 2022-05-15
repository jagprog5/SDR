#!/bin/bash

# http://logan.tw/posts/2015/04/28/check-code-coverage-with-clang-and-lcov/
# this file is used for code coverage when using clang

exec llvm-cov gcov "$@"
