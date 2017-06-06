g++ -Wall -c OglImageAPI.cpp -I `pkg-config --libs opencv` -I ../../LaputaApp/thirdpartylib/glm/
ar rvs libOglImageAPI.a OglImageAPI.o
ranlib libOglImageAPI.a
#g++ -Wall OglImageAPIMain.cpp libOglImageAPI.a -I `pkg-config --libs opencv` -I ../../LaputaApp/thirdpartylib/glm/ -o OglImageAPIMain
g++ OglImageAPIMain.cpp libOglImageAPI.a -I `pkg-config --libs opencv` -I ../../LaputaApp/thirdpartylib/glm/ -o OglImageAPIMain
