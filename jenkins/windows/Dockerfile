# escape=`

# Copyright (C) Emweb bv.
# Based on the native-desktop sample: https://github.com/Microsoft/vs-Dockerfiles
# Original license:
# Copyright (C) Microsoft Corporation. All rights reserved.
# Licensed under the MIT license. See LICENSE.txt in the project root for license information.

FROM mcr.microsoft.com/windows/servercore:ltsc2022 AS msvc_toolset

# Reset the shell.
SHELL ["cmd", "/S", "/C"]

# Set up environment to collect install errors.
COPY Install.cmd C:\TEMP\
RUN powershell `
      -NoProfile `
      -ExecutionPolicy Bypass `
      -Command "(New-Object System.Net.WebClient).DownloadFile('https://aka.ms/vscollect.exe', 'C:\TEMP\collect.exe')"

# Download channel for fixed install.
# This targets a specific version, since the global current channel version is updated regularly.
# Which can lead to an incompatibility issue in the builds. Now we target an LTSC channel for stability.
ARG CHANNEL_URL=https://aka.ms/vs/17/release.ltsc.17.6/channel
RUN powershell `
      -NoProfile `
      -ExecutionPolicy Bypass `
      -Command "(New-Object System.Net.WebClient).DownloadFile(\"$env:CHANNEL_URL\", 'C:\TEMP\VisualStudio.chman')"

# Download and install Build Tools for Visual Studio 2022 for native desktop workload,
# including 14.1 (VS 2017) and 14.2 (VS 2019) components.
# This targets a specific version, since the global current channel version is updated regularly.
# Which can lead to an incompatibility issue in the builds. Now we target an LTSC channel for stability.
ARG BUILDTOOLS_URL=https://aka.ms/vs/17/release.ltsc.17.6/vs_buildtools.exe
RUN powershell `
    -NoProfile `
    -ExecutionPolicy Bypass `
    -Command "(New-Object System.Net.WebClient).DownloadFile(\"$env:BUILDTOOLS_URL\", 'C:\TEMP\vs_buildtools.exe')"

RUN C:\TEMP\Install.cmd C:\TEMP\vs_buildtools.exe --quiet --wait --norestart --nocache `
    --channelUri C:\TEMP\VisualStudio.chman `
    --installChannelUri C:\TEMP\VisualStudio.chman `
    --add Microsoft.VisualStudio.Workload.VCTools --includeRecommended `
    --add Microsoft.VisualStudio.Component.VC.v141.x86.x64 `
    --add Microsoft.VisualStudio.ComponentGroup.VC.Tools.142.x86.x64 `
    --installPath C:\BuildTools

FROM msvc_toolset AS minimal

ARG CHOCO_URL=https://chocolatey.org/install.ps1
RUN powershell `
      -NoProfile `
      -ExecutionPolicy Bypass `
      -Command "Invoke-Expression ((New-Object System.Net.WebClient).DownloadString(\"$env:CHOCO_URL\"))"

RUN choco install `
      7zip.install `
      -y

ARG BOOST_URL=https://boostorg.jfrog.io/artifactory/main/release/1.81.0/source/boost_1_81_0.7z
RUN powershell `
      -NoProfile `
      -ExecutionPolicy Bypass `
      -Command "(New-Object System.Net.WebClient).DownloadFile(\"$env:BOOST_URL\", 'C:\TEMP\boost_1_81_0.7z')"

RUN cd %UserProfile% && 7z x C:\TEMP\boost_1_81_0.7z

ARG VCVARS_VER=14.3

RUN C:\BuildTools\Common7\Tools\VsDevCmd.bat -arch=amd64 -host_arch=amd64 -vcvars_ver=%VCVARS_VER% && `
    cd %UserProfile%\boost_1_81_0 && `
    .\bootstrap.bat && `
    .\b2 `
      variant=debug,release `
      link=shared,static `
      runtime-link=shared `
      address-model=64 `
      threading=multi `
      toolset=msvc-%VCVARS_VER% `
      --build-type=minimal `
      --with-program_options `
      --with-thread `
      --with-filesystem `
      --with-test `
      --prefix=C:\boost `
      install
