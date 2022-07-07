FROM rockylinux:8

ARG USER_ID
ARG USER_NAME
ARG GROUP_ID
ARG GROUP_NAME

WORKDIR /root

# || true to ignore error for when group or user id already exists
RUN groupadd -g ${GROUP_ID} ${GROUP_NAME} || true
RUN useradd -ms /bin/bash -u ${USER_ID} -g ${GROUP_NAME} ${USER_NAME} || true

RUN dnf -y group install "Development Tools" \
    # for dnf config-manager
 && dnf -y install dnf-plugins-core \
    # for ninja-build
 && dnf config-manager --set-enabled powertools \
    # for ccache
 && dnf -y install epel-release \
 && dnf -y install \
    ccache \
    cmake \
    boost-devel \
    ninja-build
