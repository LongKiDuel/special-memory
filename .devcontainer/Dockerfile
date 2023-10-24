FROM debian:latest

ARG REINSTALL_CMAKE_VERSION_FROM_SOURCE="none"

RUN apt-get update && apt-get install -y g++ cmake gcc make git ninja-build pkg-config

COPY ./reinstall-cmake.sh /tmp/

# RUN if [ "${REINSTALL_CMAKE_VERSION_FROM_SOURCE}" != "none" ]; then \
#         chmod +x /tmp/reinstall-cmake.sh && /tmp/reinstall-cmake.sh ${REINSTALL_CMAKE_VERSION_FROM_SOURCE}; \
#     fi \
#     && rm -f /tmp/reinstall-cmake.sh

RUN git clone --depth 1 https://github.com/microsoft/vcpkg.git ~/app/vcpkg

RUN apt-get install -y curl zip unzip tar \
    && cd ~/app/vcpkg && ./bootstrap-vcpkg.sh 

ENV VCPKG_ROOT=~/app/vcpkg
ENV CMAKE_TOOLCHAIN_FILE="${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
ENV CMAKE_GENERATOR=Ninja