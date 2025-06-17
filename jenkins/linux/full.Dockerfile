FROM ubuntu:22.04

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
    ccache \
    libunwind-dev \
 && rm -rf /var/lib/apt/lists/*

RUN apt-get update \
 && apt-get install --no-install-recommends -y \
    libgraphicsmagick1-dev \
    libpango1.0-dev \
    libmariadb-dev \
    mariadb-client-core-10.6 \
    libboost-filesystem1.74-dev \
    libboost-program-options1.74-dev \
    libboost-thread1.74-dev \
    libglu1-mesa-dev \
    libpq-dev \
    libsqlite3-dev \
    zlib1g-dev \
    libpng-dev \
    libssl-dev \
    libfcgi-dev \
    qt6-base-dev \
    libsaml-dev \
    curl \
    ca-certificates \
    gnupg \
    unzip \
    autoconf \
    libtool-bin \
    unixodbc-dev \
    libglew-dev \
    tzdata \
 && (curl https://packages.microsoft.com/keys/microsoft.asc | apt-key add - ) \
 && (curl https://packages.microsoft.com/config/ubuntu/22.04/prod.list > /etc/apt/sources.list.d/mssql-release.list) \
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
 && rm -rf /var/lib/apt/lists/*

 RUN apt-get update \
     && apt-get install python3-pip --no-install-recommends -y \
     && python3 -m pip install trio \
     && python3 -m pip install selenium \
     && wget https://dl.google.com/linux/direct/google-chrome-stable_current_amd64.deb \
     && apt-get install ./google-chrome-stable_current_amd64.deb --no-install-recommends -y \
     && apt-get install --no-install-recommends -y \
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
 && apt-get -y remove unzip \
 && rm -rf /var/lib/apt/lists/*

