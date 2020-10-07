FROM ubuntu:20.04

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
    cmake \
    libunwind-dev \
 && rm -rf /var/lib/apt/lists/*

RUN BOOST_VERSION=1.71.0 ;\
    BOOSTDIR=boost_${BOOST_VERSION//[.]/_} ;\
    wget https://dl.bintray.com/boostorg/release/${BOOST_VERSION}/source/${BOOSTDIR}.tar.bz2 -O ${BOOSTDIR}.tar.bz2 \
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

RUN apt-get update \
 && apt-get install --no-install-recommends -y \
    libgraphicsmagick1-dev \
    libpango1.0-dev \
    libmariadbclient-dev \
    mariadb-client-core-10.3 \
    libpq-dev \
    libsqlite3-dev \
    zlib1g-dev \
    libpng-dev \
    libssl-dev \
    libfcgi-dev \
    qtbase5-dev \
    curl \
    ca-certificates \
    gnupg \
    unzip \
    autoconf \
    libtool-bin \
 && (curl https://packages.microsoft.com/keys/microsoft.asc | apt-key add - ) \
 && (curl https://packages.microsoft.com/config/ubuntu/20.04/prod.list > /etc/apt/sources.list.d/mssql-release.list) \
 && apt-get update \
 && ACCEPT_EULA=Y apt-get install --no-install-recommends -y msodbcsql17 \
 && wget https://github.com/libharu/libharu/archive/RELEASE_2_3_0.zip -O haru.zip \
 && unzip haru.zip \
 && (cd libharu-RELEASE_2_3_0 \
     && ./buildconf.sh --force \
     && ./configure --prefix=/opt/haru \
     && make -j${THREAD_COUNT} \
     && make install) \
 && rm -rf libharu-RELEASE_2_3_0 \
 && rm haru.zip \
 && apt-get -y remove unzip autoconf libtool-bin curl gnupg \
 && apt-get -y autoremove \
 && rm -rf /var/lib/apt/lists/*

RUN apt-get update \
 && apt-get install --no-install-recommends -y \
    unixodbc-dev \
 && rm -rf /var/lib/apt/lists/*
