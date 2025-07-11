#把此脚本放到ogg和vorbis源码根目录下执行

export OHOS_NDK=/Users/xanderdeng/Library/openharmony/9

mkdir build
cd build

archs=("arm64-v8a" "armeabi-v7a" "x86_64")

for arch in ${archs[@]};
do
    mkdir ${arch}
    cd ${arch}
    cmake ../.. -DCMAKE_TOOLCHAIN_FILE=${OHOS_NDK}/native/build/cmake/ohos.toolchain.cmake -DOHOS_ARCH=${arch}  \
    -DCMAKE_BUILD_TYPE=Release && cmake --build . --config Release -j8
    
    cd ..
done;

   

