FROM ros:humble

RUN apt-get update && apt-get upgrade -y

RUN apt-get install -y git

# Cloning Vortex fork of Stonefish

RUN git clone https://github.com/vortexntnu/stonefish /opt/stonefish

# Installing Stonefish dependencies and MCAP for rosbags
RUN apt-get install -y \
    libsdl2-dev \
    libglm-dev \
    libfreetype6-dev \
    ros-humble-pcl-conversions \
    ros-humble-image-transport \
    ros-humble-rosbag2-storage-mcap

# Python dependencies needed for specific packages in vortex-auv

RUN apt-get install -y python3-pip python3-setuptools python3-wheel && \
    pip3 install --upgrade pip && \
    pip3 install --upgrade 'numpy<1.25' 'scipy<1.12'

# Update Compiler version

RUN apt-get update -qq && \
    apt-get install -y --no-install-recommends software-properties-common && \
    add-apt-repository ppa:ubuntu-toolchain-r/test -y && \
    apt-get update -qq && apt-get install -y --no-install-recommends gcc-13 g++-13 && \
    update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-13 100 && \
    update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-13 100

# Build and install the Stonefish library

RUN mkdir -p /opt/stonefish/build && \
    cd /opt/stonefish/build && \
    cmake .. && \
    make -j$(nproc) && \
    make install

# Make sure the library file can be found

ENV LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH