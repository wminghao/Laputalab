cd ../..
scons
cd oglserver
if [ ! -d "/usr/share/oglserver/" ]; then
    sudo mkdir /usr/share/oglserver
fi
sudo cp prog/OglServerNanny.py /usr/share/oglserver/
sudo cp -r prog/pythonlib /usr/share/oglserver/
cd scripts
sudo cp oglserver.sh /etc/init.d/
sudo cp oglservernanny.sh /etc/init.d/
sudo /etc/init.d/oglservernanny.sh stop
# TODO change from LandMarkMain to OGLImageMain
sudo cp /Laputalab/server/build/Linux-x86_64/face2d/prog/LandMarkMain /usr/bin/
sudo chown root:root /usr/bin/LandMarkMain
sudo chown -R root:root /usr/bin/Model
sudo cp /Laputalab/server/build/Linux-x86_64/oglserver/prog/ogl_server /usr/share/oglserver/
sudo /etc/init.d/oglservernanny.sh start
