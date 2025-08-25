#!/bin/bash

cd $(dirname $0)

cd ..
rm $(find build -name *.gcda)

exit 0
