cd ../..
scons
cd face2d
if [ ! -d "/usr/share/face2d/" ]; then
    sudo mkdir /usr/share/face2d
fi
sudo cp prog/Face2dServerNanny.py /usr/share/face2d/
sudo cp -r prog/pythonlib /usr/share/face2d/
cd scripts
sudo cp face2d.sh /etc/init.d/
sudo cp face2dnanny.sh /etc/init.d/
sudo /etc/init.d/face2dnanny.sh stop
sudo cp /home/ubuntu/Laputalab/server/build/Linux-x86_64/face2d/prog/LandMarkMain /usr/bin/
sudo chown root:root /usr/bin/LandMarkMain
sudo chown -R root:root /usr/bin/Model
sudo cp /home/ubuntu/Laputalab/server/build/Linux-x86_64/face2d/prog/face2d_server /usr/share/face2d/
sudo /etc/init.d/face2dnanny.sh start
