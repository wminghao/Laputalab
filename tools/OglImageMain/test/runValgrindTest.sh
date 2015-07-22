cd ..
scons type=debug
cd test
rm /tmp/abc
mkfifo /tmp/abc
cat OglImageMainTest.input2 > /tmp/abc &
valgrind --leak-check=yes --tool=memcheck /Laputalab/tools/OglImageMain/build/Linux-x86_64/prog/OglImageMain < /tmp/abc

