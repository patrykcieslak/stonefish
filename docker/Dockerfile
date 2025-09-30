FROM ros:humble

RUN apt-get update && apt-get upgrade -y

RUN apt-get install -y git

RUN git clone https://github.com/vortexntnu/stonefish /opt/stonefish

RUN apt-get install -y \
    libsdl2-dev \
    libglm-dev \
    libfreetype6-dev \
    ros-humble-pcl-conversions \
    ros-humble-image-transport \
    ros-humble-rosbag2-storage-mcap

RUN apt-get install -y python3-pip python3-setuptools python3-wheel
RUN pip3 install --upgrade pip
RUN pip3 install --upgrade 'numpy<1.25' 'scipy<1.12'

RUN apt-get update -qq && \
    apt-get install -y --no-install-recommends software-properties-common && \
    add-apt-repository ppa:ubuntu-toolchain-r/test -y
RUN apt-get update -qq && apt-get install -y --no-install-recommends gcc-13 g++-13
RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-13 100
RUN update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-13 100

RUN mkdir -p /opt/stonefish/build
RUN cd /opt/stonefish/build && \
    cmake .. && \
    make -j1 && \
    make install