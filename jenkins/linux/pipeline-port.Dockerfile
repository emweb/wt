FROM ubuntu:22.04

ARG USER_ID
ARG USER_NAME
ARG GROUP_ID
ARG GROUP_NAME

ARG DEBIAN_FRONTEND=noninteractive

SHELL ["/bin/bash", "-c"]

WORKDIR /root

RUN groupadd -g ${GROUP_ID} ${GROUP_NAME}
RUN useradd -ms /bin/bash -u ${USER_ID} -g ${GROUP_NAME} ${USER_NAME}

RUN apt-get update && apt-get install --no-install-recommends -y \
    git \
    build-essential \
    cmake \
    openjdk-11-jdk \
    flex \
    libfl-dev \
    bison \
    libboost-filesystem1.74-dev \
    libboost-program-options1.74-dev \
    doxygen \
    ant \
    ant-optional \
    unzip \
 && rm -rf /var/lib/apt/lists/*

