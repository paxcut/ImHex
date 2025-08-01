FROM emscripten/emsdk:4.0.8 AS build

# Used to invalidate layer cache but not mount cache
# See https://github.com/moby/moby/issues/41715#issuecomment-733976493
ARG UNIQUEKEY 1

RUN apt update
RUN apt install -y git ccache autoconf automake libtool pkg-config ninja-build

RUN <<EOF
# Install vcpkg
# Note: we are a patch on the libmagic port
set -xe

git clone https://github.com/microsoft/vcpkg /vcpkg
git -C /vcpkg pull
/vcpkg/bootstrap-vcpkg.sh
sed -i 's/vcpkg_install_make(${EXTRA_ARGS})/vcpkg_install_make(${EXTRA_ARGS} SUBPATH src)/g' /vcpkg/ports/libmagic/portfile.cmake
EOF

# Patch vcpkg build instructions to add -pthread
RUN <<EOF
set -xe

echo '
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
' >> /emsdk/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake
EOF

ENV VCPKG_DEFAULT_BINARY_CACHE=/cache/vcpkg
RUN --mount=type=cache,target=/cache <<EOF
# Install dependencies with vcpkg
set -xe

mkdir -p $VCPKG_DEFAULT_BINARY_CACHE

/vcpkg/vcpkg install --triplet=wasm32-emscripten libmagic
/vcpkg/vcpkg install --triplet=wasm32-emscripten freetype
/vcpkg/vcpkg install --triplet=wasm32-emscripten mbedtls
/vcpkg/vcpkg install --triplet=wasm32-emscripten zlib
/vcpkg/vcpkg install --triplet=wasm32-emscripten bzip2
/vcpkg/vcpkg install --triplet=wasm32-emscripten liblzma
/vcpkg/vcpkg install --triplet=wasm32-emscripten zstd
EOF

# Build ImHex
ARG JOBS=4
ENV CCACHE_DIR=/cache/ccache

RUN mkdir /build
WORKDIR /build
RUN --mount=type=cache,target=/cache \
    --mount=type=bind,source=.,target=/imhex <<EOF

set -xe
ccache -zs

/vcpkg/downloads/tools/cmake-*/cmake-*/bin/cmake /imhex                                                                                            \
    -G "Ninja"                                                                                          \
    -DIMHEX_OFFLINE_BUILD=ON                                                                            \
    -DIMHEX_STATIC_LINK_PLUGINS=ON                                                                      \
    -DIMHEX_EXCLUDE_PLUGINS="script_loader;remote"                                                      \
    -DIMHEX_COMPRESS_DEBUG_INFO=OFF                                                                     \
    -DNATIVE_CMAKE_C_COMPILER=gcc                                                                       \
    -DNATIVE_CMAKE_CXX_COMPILER=g++                                                                     \
    -DCMAKE_C_COMPILER_LAUNCHER=ccache                                                                  \
    -DCMAKE_CXX_COMPILER_LAUNCHER=ccache                                                                \
    -DCMAKE_TOOLCHAIN_FILE=/vcpkg/scripts/buildsystems/vcpkg.cmake                                      \
    -DVCPKG_CHAINLOAD_TOOLCHAIN_FILE=/emsdk/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake \
    -DLIBROMFS_COMPRESS_RESOURCES=OFF                                                                   \
    -DIMHEX_ENABLE_PLUGIN_TESTS=OFF                                                                     \
    -DCMAKE_BUILD_TYPE=Release

ninja -j $JOBS

cp /imhex/dist/web/source/* /build
ccache -s
EOF

# Create a file dedicated to store wasm size, because I know no way to get the wasm content length if the web server uses compression
# See https://stackoverflow.com/questions/41701849/cannot-modify-accept-encoding-with-fetch https://github.com/AnthumChris/fetch-progress-indicators/issues/13
RUN du -b /build/imhex.wasm | cut -f1 > imhex.wasm.size

FROM scratch AS raw
COPY --from=build [             \
    # ImHex                     \
    "/build/imhex.wasm",        \
    "/build/imhex.wasm.size",   \
    "/build/imhex.js",          \
                                \
    # Static files              \
    "/build/index.html",        \
    "/build/style.css",         \
    "/build/wasm-config.js",    \
    "/build/enable-threads.js", \
    "/build/favicon.ico",       \
    "/build/icon.svg",          \
    "/build/manifest.json",     \
    "/build/robots.txt",        \
    "/build/sitemap.xml",       \
                                \
    # Destination               \
    "./"                        \
]

FROM nginx
COPY --from=raw . /usr/share/nginx/html
