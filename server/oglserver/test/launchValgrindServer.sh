cd ../..
scons type=debug
cd oglserver/test
#sudo cp ~/Laputalab/server/build/Linux-x86_64/face2d/prog/LandMarkMain /usr/bin/
#sudo chown root:root /usr/bin/LandMarkMain
#sudo chown -R root:root /usr/bin/Model
sudo valgrind --leak-check=yes --tool=memcheck ~/Laputalab/server/build/Linux-x86_64/oglserver/prog/ogl_server
