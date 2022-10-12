#!/bin/bash

name=sample
asm=$name.asm
obj=$name.o
lpath=-L/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/lib
lib=-lSystem

abort() {
    cat $1 && rm $1 && exit
}

make && ./comp "$@" > $asm || abort $asm
cat $asm
as $asm -o $obj || exit
rm $asm
ld $obj -o $name $lib $lpath
rm $obj
./$name
echo $?
rm $name
