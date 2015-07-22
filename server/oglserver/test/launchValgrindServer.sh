cd ../..
scons type=debug
cd oglserver/test
sudo valgrind --leak-check=yes --tool=memcheck ~/Laputalab/server/build/Linux-x86_64/oglserver/prog/ogl_server
