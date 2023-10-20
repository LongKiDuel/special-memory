# cache api url
http://localhost:9820/vcpkg-cache/

## Usage
By env

    export VCPKG_BINARY_SOURCES="http,http://localhost:9820/vcpkg-cache/{name}/{version}/{sha},readwrite"
In CLI

    --binarysource="http,http://localhost:9820/vcpkg-cache/{name}/{version}/{sha},readwrite"