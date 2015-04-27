cd ../..
scons type=debug
cd face2d/test
rm /tmp/abc
mkfifo /tmp/abc
exec 3</tmp/abc
sudo valgrind --leak-check=yes ../../build/Linux-x86_64/face2d/prog/LandMarkMain --input-fd=3
#../../build/Linux-x86_64/face2d/prog/LandMarkMain --input-fd=3

#Warning.
#make sure you have enough memory.
#wait for a long time before it finishes. very time consuming
#in a different console,
#cat LandMarkMainTest.input > /tmp/abc
