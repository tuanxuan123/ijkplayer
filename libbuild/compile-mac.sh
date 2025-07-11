mkdir build
cd build
mkdir mac
cd mac

FF_ARG1=$1
FF_ARG2=$2

USING_AUTH="-DONAUTH=0"
if [ "$FF_ARG2" = "pxauth" ]; then
    USING_AUTH="-DONAUTH=1"
fi

case "$FF_ARG1" in
    all)
    cmake ../../ -B ./ "-DCMAKE_OSX_ARCHITECTURES=x86_64;arm64" ${USING_AUTH} -G "Xcode" && cmake --build . --config RelWithDebInfo --target PixVideoCore -j8 && cmake --build . --config RelWithDebInfo --target PixVideo -j8
    ;;
    x86_64)
    cmake ../../ -B ./ "-DCMAKE_OSX_ARCHITECTURES=x86_64" ${USING_AUTH} -G "Xcode" && cmake --build . --config RelWithDebInfo --target PixVideoCore -j8 && cmake --build . --config RelWithDebInfo --target PixVideo -j8
    ;;
    arm64)
    cmake ../../ -B ./ "-DCMAKE_OSX_ARCHITECTURES=arm64" ${USING_AUTH} -G "Xcode" && cmake --build . --config RelWithDebInfo --target PixVideoCore -j8 && cmake --build . --config RelWithDebInfo --target PixVideo -j8
    ;;
esac