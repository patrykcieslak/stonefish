#!/bin/bash

#Install libraries
sudo cp -f build/Release/libStonefish.so /usr/local/lib
#sudo cp -f build/Debug/libStonefish_debug.so /usr/local/lib

#Copy include files
sudo cp -rf include/Stonefish /usr/local/include

#Copy shaders
sudo rm -rf /usr/local/share/Stonefish
sudo mkdir /usr/local/share/Stonefish
sudo cp -rf shaders /usr/local/share/Stonefish

#Install pkgconfig file
sudo cp -f stonefish.pc /usr/local/lib/pkgconfig
sudo ldconfig
