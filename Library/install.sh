#!/bin/bash
sudo cp -f build/Release/libStonefish.so /usr/local/lib
sudo cp -f build/Debug/libStonefish_debug.so /usr/local/lib
sudo cp -rf include/Stonefish /usr/local/include
sudo cp -f stonefish.pc /usr/local/lib/pkgconfig
sudo ldconfig
