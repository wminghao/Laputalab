cd ../..
scons
cd face2d/install
sudo cp face2d.sh /etc/init.d/
sudo /etc/init.d/face2d.sh stop
sudo cp /home/ubuntu/Laputalab/server/build/Linux-x86_64/face2d/prog/LandMarkMain /usr/bin/
sudo chown root:root /usr/bin/LandMarkMain
sudo chown -R root:root /usr/bin/Model
if [ ! -d "/usr/share/face2d/" ]; then
    sudo mkdir /usr/share/face2d
fi
sudo cp /home/ubuntu/Laputalab/server/build/Linux-x86_64/face2d/prog/face2d_server /usr/share/face2d/
sudo /etc/init.d/face2d.sh start
