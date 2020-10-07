FROM ubuntu:16.04

ARG USER_ID
ARG USER_NAME
ARG GROUP_ID
ARG GROUP_NAME
ARG THREAD_COUNT=1

ARG DEBIAN_FRONTEND=noninteractive

SHELL ["/bin/bash", "-c"]

WORKDIR /root

RUN groupadd -g ${GROUP_ID} ${GROUP_NAME}
RUN useradd -ms /bin/bash -u ${USER_ID} -g ${GROUP_NAME} ${USER_NAME}

RUN apt-get update && apt-get install --no-install-recommends -y \
    wget \
    ca-certificates \
    git \
    build-essential \
    libunwind-dev \
 && rm -rf /var/lib/apt/lists/*

RUN BOOST_VERSION=1.50.0 ;\
    BOOSTDIR=boost_${BOOST_VERSION//[.]/_} ;\
    wget https://sourceforge.net/projects/boost/files/boost/${BOOST_VERSION}/${BOOSTDIR}.tar.bz2 -O ${BOOSTDIR}.tar.bz2 \
 && tar xf ${BOOSTDIR}.tar.bz2 \
 && rm ${BOOSTDIR}.tar.bz2 \
 && (cd ${BOOSTDIR} \
     && ./bootstrap.sh \
     && ./b2 \
          --prefix=/opt/boost \
          --with-filesystem \
          --with-thread \
          --with-program_options \
          -j${THREAD_COUNT} \
          install) \
 && rm -rf ${BOOSTDIR}

RUN wget https://cmake.org/files/v3.1/cmake-3.1.3-Linux-x86_64.tar.gz -O cmake-3.1.3-Linux-x86_64.tar.gz \
 && tar xf cmake-3.1.3-Linux-x86_64.tar.gz \
 && rm cmake-3.1.3-Linux-x86_64.tar.gz \
 && mv cmake-3.1.3-Linux-x86_64 /opt/cmake
