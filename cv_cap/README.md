Good Source: https://www.geeksforgeeks.org/how-to-install-opencv-in-c-on-linux/


```
sudo apt install libopencv-dev

git clone https://github.com/jbeder/yaml-cpp
cd yaml-cpp
mkdir build
cd build
cmake ..
sudo make install

cmake .
make
./DisplayImage
```

Downlaod and install yaml-cpp from github. Known issue that *sudo apt install libyaml-dev* does not come with cmake support.
