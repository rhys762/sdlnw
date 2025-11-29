#!/bin/bash
#

cd $(dirname $0)
cd ..

meson setup build/ -Dwerror=true -Db_coverage=true --buildtype=debugoptimized

exit 0
