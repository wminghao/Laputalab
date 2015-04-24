cd ../..
scons type=debug
cd face2d/test
sudo cp ~/Laputalab/server/build/Linux-x86_64/face2d/prog/LandMarkMain /usr/bin/
sudo chown root:root /usr/bin/LandMarkMain
sudo chown -R root:root /usr/bin/Model
sudo ~/Laputalab/server/build/Linux-x86_64/face2d/prog/face2d_server

