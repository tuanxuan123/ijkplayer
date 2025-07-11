# 改成自己的鸿蒙sdk路径
export OHOS_NDK=/Users/xanderdeng/Library/openharmony/9
export OHOS_NATIVE_HOME=/Users/xanderdeng/Library/openharmony/9/native
export PATH=$OHOS_NATIVE_HOME/llvm/bin:$PATH

ohos_archs=("arm64" "armeabi-v7a" "x86_64")
ohos_targets=("aarch64-linux-ohos" "arm-linux-ohos" "x86_64-linux-ohos")
openssl_archs=("linux-aarch64" "linux-armv4" "linux-x86_64")
BASE_FLAGS="--sysroot=$OHOS_NATIVE_HOME/sysroot -fdata-sections -ffunction-sections -funwind-tables -fstack-protector-strong -no-canonical-prefixes -fno-addrsig -Wa,--noexecstack -fPIC"

#OHOS_ARCH="arm64"
#OHOS_TARGET="aarch64-linux-ohos"
#OPENSSL_ARCH="linux-aarch64"
#FF_EXTRA_CFLAGS="--target=$OHOS_TARGET $BASE_FLAGS"
#FF_CFLAGS="--target=$OHOS_TARGET $BASE_FLAGS"

OHOS_ARCH="armeabi-v7a"
OHOS_TARGET="arm-linux-ohos"
OPENSSL_ARCH="linux-armv4"
FF_EXTRA_CFLAGS="--target=$OHOS_TARGET $BASE_FLAGS -march=armv7a"
FF_CFLAGS="--target=$OHOS_TARGET $BASE_FLAGS -march=armv7a"


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


LIBS_DIR=$(pwd)/libs
PREFIX=$LIBS_DIR/$OHOS_ARCH

echo "PREFIX"=$PREFIX

export CC="$CC --target=$OHOS_TARGET"
export CXX="$CXX --target=$OHOS_TARGET"
export CXXFLAGS=$FF_EXTRA_CFLAGS
export CFLAGS=$FF_CFLAGS
export AR="$AR"
export LD="$LD"
export AS="$AS"
export NM="$NM"
export RANLIB="$RANLIB"
export STRIP="$STRIP"
export LDFLAGS="--rtlib=compiler-rt -fuse-ld=lld"

./Configure $OPENSSL_ARCH zlib-dynamic enable-md2 enable-rc5 experimental-jpake experimental-store \
--prefix=$PREFIX \
no-asm \
no-threads

make clean

#make -j8会报错，需要对opensslconf.h修改报错误的换行
#make -j8
#make install
