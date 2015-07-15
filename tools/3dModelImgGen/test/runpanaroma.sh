#!/bin/bash
pwd=`pwd`
modeldir='/root/3dmodels'
echo $modeldir
cd $modeldir/result
rm $1.tar.gz 
rm -rf $1
cd $pwd
./runsingle.sh $1 +0 $pwd $modeldir
./runsingle.sh $1 +10 $pwd $modeldir
./runsingle.sh $1 +20 $pwd $modeldir
./runsingle.sh $1 +30 $pwd $modeldir
./runsingle.sh $1 +40 $pwd $modeldir
./runsingle.sh $1 +50 $pwd $modeldir
./runsingle.sh $1 +60 $pwd $modeldir
./runsingle.sh $1 -10 $pwd $modeldir
./runsingle.sh $1 -20 $pwd $modeldir
./runsingle.sh $1 -30 $pwd $modeldir
./runsingle.sh $1 -40 $pwd $modeldir
./runsingle.sh $1 -50 $pwd $modeldir
./runsingle.sh $1 -60 $pwd $modeldir
cd $modeldir/result
tar -zcvf $1.tar.gz $1
cd $pwd
