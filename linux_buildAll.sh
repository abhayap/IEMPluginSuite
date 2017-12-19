#!/bin/sh
cd ${0#/*}
for d in */; do
    echo "Compiling $d for Linux..."
    if [ -d "$PWD/${d}Builds/LinuxMakefile" ]; then
        make CONFIG=Release AR=gcc-ar -j$(nproc) -k -C "$PWD/${d}Builds/LinuxMakefile"
        echo "done..."
    else
        echo "no makefile found, moving on..."
    fi
done
echo "all done!"
