#! /usr/bin/env bash
#
# Copyright (C) 2013-2014 Bilibili
# Copyright (C) 2013-2014 Zhang Rui <bbcallen@gmail.com>
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

# This script is based on projects below
# https://github.com/kolyvan/kxmovie
# https://github.com/yixia/FFmpeg-Android
# http://git.videolan.org/?p=vlc-ports/android.git;a=summary
# https://github.com/kewlbear/FFmpeg-iOS-build-script/

\
echo "===================="
echo "[*] check host"
echo "===================="
set -e



FF_ARCH=$1
FF_BUILD_OPT=$2
echo "FF_ARCH=$FF_ARCH"
echo "FF_BUILD_OPT=$FF_BUILD_OPT"
if [ -z "$FF_ARCH" ]; then
    echo "You must specific an architecture 'arm64, x86_64, ...'.\n"
    exit 1
fi


FF_BUILD_ROOT=`pwd`
FF_TAGET_OS="darwin"

FF_XCRUN_CC="xcrun -sdk macosx clang"
# ffmpeg build params
export COMMON_FF_CFG_FLAGS=
source $FF_BUILD_ROOT/../config/module.sh

FFMPEG_CFG_FLAGS=
FFMPEG_CFG_FLAGS="$FFMPEG_CFG_FLAGS $COMMON_FF_CFG_FLAGS"


FFMPEG_CFG_FLAGS="$FFMPEG_CFG_FLAGS --disable-stripping"


FFMPEG_CFG_FLAGS="$FFMPEG_CFG_FLAGS --arch=$FF_ARCH"

FFMPEG_CFG_FLAGS="$FFMPEG_CFG_FLAGS --enable-static"
FFMPEG_CFG_FLAGS="$FFMPEG_CFG_FLAGS --disable-shared"
FFMPEG_CFG_FLAGS="$FFMPEG_CFG_FLAGS --disable-openssl"
FFMPEG_CFG_FLAGS="$FFMPEG_CFG_FLAGS --enable-securetransport"
FFMPEG_CFG_FLAGS="$FFMPEG_CFG_FLAGS --enable-protocol=tls_securetransport"

case "$FF_BUILD_OPT" in
    debug)
        FFMPEG_CFG_FLAGS="$FFMPEG_CFG_FLAGS --disable-optimizations"
        FFMPEG_CFG_FLAGS="$FFMPEG_CFG_FLAGS --enable-debug"
        FFMPEG_CFG_FLAGS="$FFMPEG_CFG_FLAGS --disable-small"
    ;;
    *)
        FFMPEG_CFG_FLAGS="$FFMPEG_CFG_FLAGS --enable-optimizations"
        FFMPEG_CFG_FLAGS="$FFMPEG_CFG_FLAGS --enable-debug"
        FFMPEG_CFG_FLAGS="$FFMPEG_CFG_FLAGS --enable-small"
    ;;
esac

echo "build_root: $FF_BUILD_ROOT"


echo "===================="
echo "[*] config arch $FF_ARCH"
echo "===================="

FF_BUILD_NAME="unknown"
FF_XCRUN_PLATFORM="MacOS arm64"
FF_XCRUN_OSVERSION=
FF_GASPP_EXPORT=
FF_DEP_OPENSSL_INC=
FF_DEP_OPENSSL_LIB=
FF_XCODE_BITCODE=
FFMPEG_DEP_LIBS=


if [ "$FF_ARCH" = "x86_64" ]; then
    FF_BUILD_NAME="ffmpeg-x86_64"
    FF_BUILD_NAME_OPENSSL=openssl-x86_64
    FF_XCRUN_PLATFORM="MacOS x86_64"
    FFMPEG_CFG_FLAGS="$FFMPEG_CFG_FLAGS --disable-x86asm --enable-cross-compile --target-os=$FF_TAGET_OS --arch=x86_64 --disable-asm"
    FF_XCRUN_OSVERSION="-mmacosx-version-min=10.10"
elif [ "$FF_ARCH" = "arm64" ]; then
    FF_BUILD_NAME="ffmpeg-arm64"
    FF_BUILD_NAME_OPENSSL=openssl-arm64
    FFMPEG_CFG_FLAGS="$FFMPEG_CFG_FLAGS --enable-cross-compile --enable-neon --target-os=$FF_TAGET_OS"
    FF_XCRUN_OSVERSION="-mmacosx-version-min=10.10"
    FF_XCRUN_CC="xcrun -sdk macosx clang -destination platform=macOS,arch=arm64"
else
    echo "unknown architecture $FF_ARCH";
    exit 1
fi

echo "build_name: $FF_BUILD_NAME"
echo "platform:   $FF_XCRUN_PLATFORM"
echo "osversion:  $FF_XCRUN_OSVERSION"

#--------------------
echo "===================="
echo "[*] make ios toolchain $FF_BUILD_NAME"
echo "===================="

FF_BUILD_SOURCE="$FF_BUILD_ROOT/ffmpeg-x86_64"
FF_BUILD_PREFIX="$FF_BUILD_ROOT/build/$FF_BUILD_NAME/output"

FFMPEG_CFG_FLAGS="$FFMPEG_CFG_FLAGS --prefix=$FF_BUILD_PREFIX"

mkdir -p $FF_BUILD_PREFIX

echo "build_source: $FF_BUILD_SOURCE"
echo "build_prefix: $FF_BUILD_PREFIX"

#--------------------
echo "\n--------------------"
echo "[*] configurate ffmpeg"
echo "--------------------"

FFMPEG_CFLAGS=
FFMPEG_CFLAGS="$FFMPEG_CFLAGS -arch $FF_ARCH"
FFMPEG_CFLAGS="$FFMPEG_CFLAGS"
FFMPEG_LDFLAGS="$FFMPEG_CFLAGS"



#--------------------
echo "\n--------------------"
echo "[*] configure"
echo "----------------------"


cd $FF_BUILD_SOURCE

echo "config: $FFMPEG_CFG_FLAGS $FF_XCRUN_CC"
./configure \
    $FFMPEG_CFG_FLAGS  \
    --cc="$FF_XCRUN_CC" \
    --extra-cflags="$FFMPEG_CFLAGS" \
    --extra-cxxflags="$FFMPEG_CFLAGS" \
     --extra-ldflags="$FFMPEG_LDFLAGS"


make clean

echo "\n--------------------"
echo "[*] compile ffmpeg"
echo "--------------------"
cp config.* $FF_BUILD_PREFIX
make -j3 $FF_GASPP_EXPORT
make install
mkdir -p $FF_BUILD_PREFIX/include/libffmpeg
cp -f config.h $FF_BUILD_PREFIX/include/libffmpeg/config.h
