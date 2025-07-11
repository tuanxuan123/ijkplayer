mkdir build
cd build
mkdir android
cd android

archs=("arm64-v8a" "armeabi-v7a" "x86_64" "x86")

FF_ARG1=$1
USING_AUTH="-DONAUTH=0"
if [ "$FF_ARG1" = "pxauth" ]; then
    USING_AUTH="-DONAUTH=1"
fi

toolchain="${NDK_ROOT}/build/cmake/android.toolchain.cmake"

for arch in ${archs[@]};
do
    mkdir ${arch}
    cd ${arch}

    cmake ../../../ -B ./    \
    -DCMAKE_VERBOSE_MAKEFILE=1 \
    -DCMAKE_TOOLCHAIN_FILE=${toolchain} \
    -DCMAKE_ANDROID_ARCH=${arch} \
    -DANDROID_ABI=${arch} \
    -DCMAKE_ANDROID_NDK_TOOLCHAIN_VERSION=clang \
    -DCMAKE_CXXFLAGS=-std=c++14 \
    -DCMAKE_CXX14_EXTENSION_COMPILE_OPTION=-std=c++14 \
    -DCMAKE_ANDROID_STL_TYPE=c++_static \
    -DANDROID_STL=c++_static \
    -DANDROID_PLATFORM=android-21 \
    -DANDROID_NATIVE_API_LEVEL=21 \
    -DANDROID_TOOLCHAIN=clang \
    ${USING_AUTH}  \
    -DCMAKE_BUILD_TYPE=RelWithDebInfo \
    && cmake --build . --config RelWithDebInfo -j8

    cd ..
done;