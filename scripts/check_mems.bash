#!/bin/bash

cd $(dirname $0)
cd ..

echo "Checking for standard memory functions, no output is good:"

function do_grep() {
    grep -rn '\smalloc(' src/lib/
    grep -rn '\srealloc(' src/lib/
    grep -rn '\sfree(' src/lib/
}

do_grep | grep -v src/lib/include/internal_helpers.h | grep -v src/lib/src/internal_helpers.c

exit 0
