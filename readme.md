# cache api url
http://localhost:9820/vcpkg-cache/


## Simple setup
Run
``` 
# 'EOL' for not change var in command
cat  >> ~/.bashrc << 'EOL'
export CUSTOM_VCPKG_MIRROR="http://localhost:9820/vcpkg-cache"
export VCPKG_BINARY_SOURCES="http,${CUSTOM_VCPKG_MIRROR}/{name}/{version}/{sha},readwrite"
export X_VCPKG_ASSET_SOURCES="x-azurl,${CUSTOM_VCPKG_MIRROR}/asset/,,readwrite"
EOL
```

## Usage
If find some error to connect to server, please check http proxy setting first.

By env

    export VCPKG_BINARY_SOURCES="http,http://localhost:9820/vcpkg-cache/{name}/{version}/{sha},readwrite"
In CLI

    --binarysource="http,http://localhost:9820/vcpkg-cache/{name}/{version}/{sha},readwrite"

## Usage for asset caching
    you can add ';x-block-origin' to prevent use origin

    

ENV

    export X_VCPKG_ASSET_SOURCES="x-azurl,http://localhost:9820/vcpkg-cache/asset/,,readwrite"

CLI

    --x-asset-sources="x-azurl,http://localhost:9820/vcpkg-cache/asset/,,readwrite"


if it still show '-- Downloading https://github.com/xxx.tar.gz -> xxx', don't worry it actually download from your mirrors.

