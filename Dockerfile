FROM ubuntu

ENV TZ=Europe/Berlin
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

# update system
ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get upgrade -y

# install deps
RUN apt-get install -y build-essential cmake libopenvdb-dev

# copy code
WORKDIR /workspace
COPY CMakeLists.txt ./
COPY src/ src/
COPY tools/ tools/
COPY submodules/ submodules/

# build
RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -Wno-dev && cmake --build build --parallel
