cd ..
scons
cd test
rm /tmp/abc
mkfifo /tmp/abc
cat OglImageMainTest.input > /tmp/abc &
time /Laputalab/tools/OglImageMain/build/Linux-x86_64/prog/OglImageMain < /tmp/abc

