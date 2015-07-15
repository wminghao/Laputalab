cd ../result
rm $1.tar.gz 
rm -rf $1
cd ../test
./runsingle.sh $1 +0
./runsingle.sh $1 +10
./runsingle.sh $1 +20
./runsingle.sh $1 +30
./runsingle.sh $1 +40
./runsingle.sh $1 +50
./runsingle.sh $1 +60
./runsingle.sh $1 -10
./runsingle.sh $1 -20
./runsingle.sh $1 -30
./runsingle.sh $1 -40
./runsingle.sh $1 -50
./runsingle.sh $1 -60
cd ../result
tar -zcvf $1.tar.gz $1
cd ../test
