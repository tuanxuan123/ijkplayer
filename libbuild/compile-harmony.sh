mkdir build
cd build
mkdir harmony
cd harmony

# 改成自己的鸿蒙sdk路径
export OHOS_NDK=/Users/xanderdeng/Library/openharmony/10
export OHOS_NATIVE_HOME=/Users/xanderdeng/Library/openharmony/10/native
export PATH=$OHOS_NATIVE_HOME/llvm/bin:$PATH



archs=("arm64-v8a" "armeabi-v7a" "x86_64")

FF_ARG1=$1
USING_AUTH="-DONAUTH=0"
if [ "$FF_ARG1" = "pxauth" ]; then
    USING_AUTH="-DONAUTH=1"
fi


for arch in ${archs[@]};
do
    mkdir ${arch}
    cd ${arch}

    cmake ../../../ -B ./    \
    -DCMAKE_TOOLCHAIN_FILE=${OHOS_NDK}/native/build/cmake/ohos.toolchain.cmake \
    -DOHOS_ARCH=${arch}    \
    ${USING_AUTH}          \
    -DCMAKE_BUILD_TYPE=RelWithDebInfo -L \
    && cmake --build . --config RelWithDebInfo -j8

    cd ..
done;