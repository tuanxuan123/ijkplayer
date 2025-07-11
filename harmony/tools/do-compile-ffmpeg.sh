# 改成自己的鸿蒙sdk路径
# export OHOS_NDK=/Users/xanderdeng/Library/openharmony/9
# export OHOS_NATIVE_HOME=/Users/xanderdeng/Library/openharmony/9/native


# 工具链
TOOLCHAIN=$OHOS_NATIVE_HOME/llvm

# 交叉编译库搜索路径
SYS_ROOT=$OHOS_NATIVE_HOME/sysroot
# 编译器
CC=$TOOLCHAIN/bin/clang
CXX=$TOOLCHAIN/bin/clang++
# 链接器，将目标文件（包括静态库和共享库）合并成一个可执行文件或共享库
LD=$TOOLCHAIN/bin/ld-lld
# 汇编器，将汇编语言代码转换为机器代码
AS=$TOOLCHAIN/bin/llvm-as
# 静态库管理工具，用于创建、修改和提取静态库中的目标文件
AR=$TOOLCHAIN/bin/llvm-ar
# 符号表工具，用于显示目标文件中的符号（函数、变量等）信息
NM=$TOOLCHAIN/bin/llvm-nm
# 静态库索引工具，用于创建和更新静态库的索引，以提高库的访问速度
RANLIB=$TOOLCHAIN/bin/llvm-ranlib
# 剥离工具，用于从可执行文件或共享库中去除调试信息，从而减小文件大小
STRIP=$TOOLCHAIN/bin/llvm-strip

#--------------------
# common defines
FF_ARCH=$1
FF_BUILD_OPT=$2
echo "FF_ARCH=$FF_ARCH"
echo "FF_BUILD_OPT=$FF_BUILD_OPT"
if [ -z "$FF_ARCH" ]; then
    echo "You must specific an architecture 'arm64, armv7a, x86_64'."
    echo ""
    exit 1
fi


FF_BUILD_ROOT=`pwd`


FF_BUILD_NAME=
FF_SOURCE=
FF_CROSS_PREFIX=
FF_DEP_OPENSSL_INC=
FF_DEP_OPENSSL_LIB=

FF_CFG_FLAGS=

FF_EXTRA_CFLAGS=
FF_EXTRA_LDFLAGS=
FF_DEP_LIBS=

FF_ARCH_R=


if [ "$FF_ARCH" = "armv7a" ]; then
    FF_BUILD_NAME=ffmpeg-armv7a
    FF_ARCH_R=armeabi-v7a
#   使用android下的ffmpeg源码
    FF_SOURCE="$FF_BUILD_ROOT/../android/contrib/ffmpeg-arm64"

    FF_CROSS_PREFIX=arm-linux-ohos

    FF_CFG_FLAGS="$FF_CFG_FLAGS --arch=armeabi-v7a --cpu=armv7a"

    FF_EXTRA_CFLAGS="$FF_EXTRA_CFLAGS -march=armv7-a --target=arm-linux-ohos"
    FF_EXTRA_LDFLAGS="$FF_EXTRA_LDFLAGS -march=armv7-a --target=arm-linux-ohos -Wl,--fix-cortex-a8"


elif [ "$FF_ARCH" = "x86_64" ]; then

    FF_BUILD_NAME=ffmpeg-x86_64
    FF_ARCH_R=x86_64

    FF_SOURCE="$FF_BUILD_ROOT/../android/contrib/ffmpeg-arm64"

    FF_CROSS_PREFIX=x86_64-linux-ohos

    FF_CFG_FLAGS="$FF_CFG_FLAGS --arch=x86_64 --enable-yasm"

    FF_EXTRA_CFLAGS="$FF_EXTRA_CFLAGS -march=x86-64 --target=x86_64-linux-ohos"
    FF_EXTRA_LDFLAGS="$FF_EXTRA_LDFLAGS -march=x86-64 --target=x86_64-linux-ohos"


elif [ "$FF_ARCH" = "arm64" ]; then

    FF_BUILD_NAME=ffmpeg-arm64
    FF_ARCH_R=arm64-v8a

    FF_SOURCE="$FF_BUILD_ROOT/../android/contrib/ffmpeg-arm64"

    FF_CROSS_PREFIX=aarch64-linux-ohos

    FF_CFG_FLAGS="$FF_CFG_FLAGS --arch=arm64 --enable-yasm"

    FF_EXTRA_CFLAGS="$FF_EXTRA_CFLAGS -march=armv8-a --target=aarch64-linux-ohos"
    FF_EXTRA_LDFLAGS="$FF_EXTRA_LDFLAGS -march=armv8-a --target=aarch64-linux-ohos"

else
    echo "unknown architecture $FF_ARCH";
    exit 1
fi

if [ ! -d $FF_SOURCE ]; then
    echo ""
    echo "!! ERROR"
    echo "!! Can not find FFmpeg directory for $FF_BUILD_NAME"
    echo "!! Run 'sh init-android.sh' first"
    echo ""
    exit 1
fi


FF_PREFIX=$FF_BUILD_ROOT/build/$FF_BUILD_NAME/output

FF_DEP_VORBIS_INC=$FF_BUILD_ROOT/libvorbis/include
FF_DEP_VORBIS_LIB=$FF_BUILD_ROOT/libvorbis/lib/$FF_ARCH_R
FF_DEP_OPENSSL_INC=$FF_BUILD_ROOT/openssl/include
FF_DEP_OPENSSL_LIB=$FF_BUILD_ROOT/openssl/lib/$FF_ARCH_R


mkdir -p $FF_PREFIX


#--------------------
echo ""
echo "--------------------"
echo "[*] check ffmpeg env"
echo "--------------------"
export PATH=$OHOS_NATIVE_HOME/llvm/bin:$PATH


export AR="$AR"
export LD="$LD"
export AS="$AS"
export NM="$NM"
export RANLIB="$RANLIB"
export STRIP="$STRIP"
#export CFLAGS="$CFLAGS -fPIC"
#export CXXFLAGS="$CXXFLAGS -fPIC"


#FF_CFLAGS="-O3 -Wall -pipe \
#    -std=c99 \
#    -ffast-math \
#    -fstrict-aliasing -Werror=strict-aliasing \
#    -Wno-psabi -Wa,--noexecstack \
#    -DNDEBUG"


case "$FF_BUILD_OPT" in
    mini)
    export COMMON_FF_CFG_FLAGS=
    . $FF_BUILD_ROOT/../config/module-mini.sh

    ;;
    *)
        
    export COMMON_FF_CFG_FLAGS=
    . $FF_BUILD_ROOT/../config/module.sh


    #--------------------
    # with openssl
    if [ -f "${FF_DEP_OPENSSL_LIB}/libssl.a" ]; then
        echo "OpenSSL detected"

        FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-openssl"

        FF_CFLAGS="$FF_CFLAGS -I${FF_DEP_OPENSSL_INC}"
        FF_DEP_LIBS="$FF_DEP_LIBS -L${FF_DEP_OPENSSL_LIB} -lssl -lcrypto"
    fi


    if [ -f "${FF_DEP_VORBIS_LIB}/libvorbis.a" ]; then
        echo "Vorbis detected"
        FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-libvorbis"

        FF_CFLAGS="$FF_CFLAGS -I${FF_DEP_VORBIS_INC}"
        FF_DEP_LIBS="$FF_DEP_LIBS -L${FF_DEP_VORBIS_LIB} -lvorbis -logg -lm"
    fi

    ;;
esac

FF_CFG_FLAGS="$FF_CFG_FLAGS $COMMON_FF_CFG_FLAGS"

#--------------------
# Standard options:
FF_CFG_FLAGS="$FF_CFG_FLAGS --prefix=$FF_PREFIX"

FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-cross-compile"
FF_CFG_FLAGS="$FF_CFG_FLAGS --target-os=linux"
FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-pic"
FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-x86asm"
FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-shared --enable-static"


case "$FF_BUILD_OPT" in
    debug)
        FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-optimizations"
        FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-debug"
        FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-small"
    ;;
    *)
        #优化ffmpeg体积
        FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-optimizations"
        FF_CFG_FLAGS="$FF_CFG_FLAGS --disable-debug"
        FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-small"
        FF_CFG_FLAGS="$FF_CFG_FLAGS --enable-stripping"
        FF_EXTRA_CFLAGS="$FF_EXTRA_CFLAGS -s"
    ;;
esac

#--------------------
echo ""
echo "--------------------"
echo "[*] configurate ffmpeg"
echo "--------------------"
cd $FF_SOURCE

which $CC

./configure $FF_CFG_FLAGS \
    --sysroot=${SYS_ROOT} --cc=${CC} --cxx=${CXX}  --ranlib=${RANLIB}    \
    --extra-cflags="$FF_CFLAGS $FF_EXTRA_CFLAGS -s"    \
    --extra-ldflags="$FF_DEP_LIBS $FF_EXTRA_LDFLAGS"

make clean




#--------------------
echo ""
echo "--------------------"
echo "[*] compile ffmpeg"
echo "--------------------"
cp config.* $FF_PREFIX
make -j8
make install
mkdir -p $FF_PREFIX/include/libffmpeg
cp -f config.h $FF_PREFIX/include/libffmpeg/config.h


