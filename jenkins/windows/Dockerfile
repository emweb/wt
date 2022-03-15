# escape=`

# Copyright (C) Emweb bv.
# Based on the native-desktop sample: https://github.com/Microsoft/vs-Dockerfiles
# Original license:
# Copyright (C) Microsoft Corporation. All rights reserved.
# Licensed under the MIT license. See LICENSE.txt in the project root for license information.

FROM mcr.microsoft.com/dotnet/framework/sdk:4.8-windowsservercore-ltsc2019 AS msvc_toolset

# Reset the shell.
SHELL ["cmd", "/S", "/C"]

# Set up environment to collect install errors.
COPY Install.cmd C:\TEMP\
RUN powershell -NoProfile -ExecutionPolicy Bypass -Command "(New-Object System.Net.WebClient).DownloadFile('https://aka.ms/vscollect.exe', 'C:\TEMP\collect.exe')"

# Download channel for fixed install.
ARG CHANNEL_URL=https://aka.ms/vs/17/release/channel
ADD ${CHANNEL_URL} C:\TEMP\VisualStudio.chman

# Download and install Build Tools for Visual Studio 2017 for native desktop workload.
RUN powershell -NoProfile -ExecutionPolicy Bypass -Command "(New-Object System.Net.WebClient).DownloadFile('https://aka.ms/vs/17/release/vs_buildtools.exe', 'C:\TEMP\vs_buildtools.exe')"
RUN C:\TEMP\Install.cmd C:\TEMP\vs_buildtools.exe --quiet --wait --norestart --nocache `
    --channelUri C:\TEMP\VisualStudio.chman `
    --installChannelUri C:\TEMP\VisualStudio.chman `
    --add Microsoft.VisualStudio.Workload.VCTools --includeRecommended`
    --installPath C:\BuildTools

FROM msvc_toolset AS minimal

RUN powershell -NoProfile -ExecutionPolicy Bypass -Command "iex ((New-Object System.Net.WebClient).DownloadString('https://chocolatey.org/install.ps1'))"

RUN choco install `
      7zip.install `
      -y

RUN powershell -NoProfile -ExecutionPolicy Bypass -Command "(New-Object System.Net.WebClient).DownloadFile('https://boostorg.jfrog.io/artifactory/main/release/1.78.0/source/boost_1_78_0.7z', 'C:\TEMP\boost_1_78_0.7z')"

RUN cd %UserProfile% && 7z x C:\TEMP\boost_1_78_0.7z

RUN C:\BuildTools\Common7\Tools\VsDevCmd.bat -arch=amd64 -host_arch=amd64 -vcvars_ver=14.3 && `
    cd %UserProfile%\boost_1_78_0 && `
    .\bootstrap.bat && `
    .\b2 `
      variant=debug,release `
      link=shared,static `
      runtime-link=shared `
      address-model=64 `
      threading=multi `
      toolset=msvc-14.3 `
      --build-type=minimal `
      --with-program_options `
      --with-thread `
      --with-filesystem `
      --with-test `
      --prefix=C:\boost `
      install
