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
    libsqlite3-dev \
    libgraphicsmagick1-dev \
    libpango1.0-dev \
    libboost-filesystem1.74-dev \
    libboost-program-options1.74-dev \
    libboost-thread1.74-dev \
    libglu1-mesa-dev \
    zlib1g-dev \
    libpng-dev \
    libssl-dev \
    qt6-base-dev \
    libsaml-dev \
    curl \
    ca-certificates \
    gnupg \
    unzip \
    autoconf \
    libtool-bin \
    libglew-dev \
    tzdata \
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

USER ${USER_ID}

RUN cd "${HOME}" && wget -qO- https://get.pnpm.io/install.sh | bash -

RUN export PNPM_HOME="${HOME}/.local/share/pnpm"; \
    export PATH="${PNPM_HOME}:${PATH}"; \
    cd "${HOME}" \
 && pnpm env use --global lts \
 && pnpm config set store-dir "${HOME}/.pnpm-store"

USER root
