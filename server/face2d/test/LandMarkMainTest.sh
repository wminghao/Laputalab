#WARNING, to Run valgrind, Turn on flag TEST_INPUT in LandMarkMain.cpp
cd ../..
scons type=debug
cd face2d/test
rm out.list
rm /tmp/abc
mkfifo /tmp/abc
cat LandMarkMainTest.input > /tmp/abc &
valgrind --leak-check=yes ../../build/Linux-x86_64/face2d/prog/LandMarkMain < /tmp/abc
#../../build/Linux-x86_64/face2d/prog/LandMarkMain < /tmp/abc
