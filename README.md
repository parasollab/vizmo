# Vizmo

Visualization software for PPL problems and results.

## Prerequisites

```bash
apt-get install libva-dev libvdpau-dev libfontenc-dev, libxaw7-dev, libxkbfile-dev, libxmu-dev, libxmuu-dev, libxpm-dev, libxres-dev, libxss-dev, libxtst-dev, libxv-dev, libxvmc-dev, libxxf86vm-dev, libxcb-dri3-dev
```

## Install using VCPKG
```bash
cmake -G Ninja -DCMAKE_TOOLCHAIN_FILE=/opt/vcpkg/scripts/buildsystems/vcpkg.cmake -B cmake-build-debug
```

```bash
cmake --build cmake-build-debug
```

## Install using Conan 2
### Install Conan Packages
```bash
conan install . --install-folder cmake-build-release --build=missing -e CONAN_CMAKE_GENERATOR=Ninja -c tools.system.package_manager:mode=install -c tools.system.package_manager:sudo=true
```
### Run
```bash
cmake . -DCMAKE_TOOLCHAIN_FILE=cmake-build-release/conan_toolchain.cmake
cmake --build .
```

