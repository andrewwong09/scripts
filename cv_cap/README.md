Good Source: https://www.geeksforgeeks.org/how-to-install-opencv-in-c-on-linux/


```
sudo apt-get update && sudo apt-get install build-essential
sudo apt install libopencv-dev

# Install yaml-cpp
git clone https://github.com/jbeder/yaml-cpp
cd yaml-cpp
mkdir build
cd build
cmake ..
sudo make install

# Make program
cmake .
make
./DisplayImage
```

Downlaod and install yaml-cpp from github. Known issue that *sudo apt install libyaml-dev* does not come with cmake support.
