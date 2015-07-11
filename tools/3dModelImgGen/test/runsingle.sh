#!/bin/bash
cd ..
if [ ! -d "result" ];
then
   mkdir result
fi
cd result
if [ ! -d "$1" ];
then
   mkdir $1
fi
cd ..
build/Linux-x86_64/VET/prog/3dModelImgGen white.jpg $1$2.jpg $1 $2
cd test
