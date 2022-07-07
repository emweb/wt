FROM debian:10

ARG USER_ID
ARG USER_NAME
ARG GROUP_ID
ARG GROUP_NAME

ARG DEBIAN_FRONTEND=noninteractive

WORKDIR /root

RUN groupadd -g ${GROUP_ID} ${GROUP_NAME}
RUN useradd -ms /bin/bash -u ${USER_ID} -g ${GROUP_NAME} ${USER_NAME}

RUN apt-get update \
 && apt-get install --no-install-recommends -y \
    build-essential \
    ccache \
    cmake \
    libboost-filesystem-dev \
    libboost-program-options-dev \
    libboost-system-dev \
    libboost-thread-dev \
    ninja-build \
 && rm -rf /var/lib/apt/lists/*
