#!/bin/bash

set -e

cd $(dirname $0)
cd ..

if [ -d ./build/jbm/ ]; then
    echo 'already have font'
    exit 0
fi

wget 'https://download.jetbrains.com/fonts/JetBrainsMono-2.304.zip'
unzip JetBrainsMono-2.304.zip -d ./jbm/
