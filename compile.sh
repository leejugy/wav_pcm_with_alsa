source /opt/fsl-frdm-multimedia/environment-setup-armv8a-poky-linux

export TARGET="main"
export CFLAGS=""
export CXXFLAGS=""
export CPPFLAGS=""
export LDFLAGS="-lasound"
make -j$(cat /proc/cpuinfo  | grep -c processor)

cp $TARGET ~/share