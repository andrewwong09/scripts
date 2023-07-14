# scripts
General scripts for my machines.

### Launching GST
Clone tiscamera into **$HOME/scratch/** and build.
```
git clone git@github.com:TheImagingSource/tiscamera.git
cd tiscamera
# only works on Debian based systems like Ubuntu
sudo ./scripts/dependency-manager install
mkdir build
cd build

# Without ARAVIS
cmake -DTCAM_BUILD_ARAVIS=OFF ..

make
sudo make install
```
