# cache api url
http://localhost:9820/vcpkg-cache/

## Usage
If find some error to connect to server, please check http proxy setting first.

By env

    export VCPKG_BINARY_SOURCES="http,http://localhost:9820/vcpkg-cache/{name}/{version}/{sha},readwrite"
In CLI

    --binarysource="http,http://localhost:9820/vcpkg-cache/{name}/{version}/{sha},readwrite"