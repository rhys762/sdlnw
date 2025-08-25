#!/bin/bash
# run tests and report coverage

cd $(dirname $0)
cd ..
PROJECT_ROOT=$(pwd)

cd $PROJECT_ROOT/build
ninja coverage-html

exit 0
