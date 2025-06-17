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
    wget \
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
 && mkdir -p /opt/tinymce/3 /opt/tinymce/4 /opt/tinymce/6 \
 && wget https://download.tiny.cloud/tinymce/community/tinymce_6.8.4.zip -O /opt/tinymce/6/tinymce.zip \
 && unzip /opt/tinymce/6/tinymce.zip -d /opt/tinymce/6 \
 && rm /opt/tinymce/6/tinymce.zip \
 && wget http://download.tiny.cloud/tinymce/community/tinymce_4.9.11.zip -O /opt/tinymce/4/tinymce.zip \
 && unzip /opt/tinymce/4/tinymce.zip -d /opt/tinymce/4 \
 && rm /opt/tinymce/4/tinymce.zip \
 && wget http://download.tiny.cloud/tinymce/community/tinymce_3.5.12.zip -O /opt/tinymce/3/tinymce.zip \
 && unzip /opt/tinymce/3/tinymce.zip -d /opt/tinymce/3 \
 && rm /opt/tinymce/3/tinymce.zip \
 && rm -rf /var/lib/apt/lists/*

