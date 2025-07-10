
mkdir build
cd build
mkdir harmony
cd harmony
set OHOS_NDK=C:\Users\pandora-pc\AppData\Local\OpenHarmony\Sdk\12pixvideo
set CMAKE_HOME=C:\Users\pandora-pc\AppData\Local\OpenHarmony\Sdk\12pixvideo\native\build-tools\cmake
set PATH=%CMAKE_HOME%\bin;%PATH%

set USING_AUTH=-DONAUTH=0
if "%1%" == "pxauth" set USING_AUTH=-DONAUTH=1
set archs="arm64-v8a" "armeabi-v7a" "x86_64"
for %%i in (%archs%) do (
    mkdir %%i
    cd %%i
    del .\lib\libPixVideo.so
    del .\lib\libPixVideo.so.symbol
    del .\lib\libPixVideoCore.so
    del .\lib\libPixVideoCore.so.symbol
    %CMAKE_HOME%\bin\cmake.exe ../../../ -G "Ninja" ^
    -DCMAKE_TOOLCHAIN_FILE=%OHOS_NDK%\native\build\cmake\ohos.toolchain.cmake ^
    -DOHOS_ARCH=%%i ^
    %USING_AUTH% ^
    -DCMAKE_BUILD_TYPE=RelWithDebInfo ^
    && %CMAKE_HOME%\bin\cmake.exe --build . --config RelWithDebInfo
    copy .\lib\libPixVideo.so .\lib\libPixVideo.so.symbol
    %OHOS_NDK%\native\llvm\bin\llvm-strip.exe -g -S -d --strip-debug .\lib\libPixVideo.so
    copy .\lib\libPixVideoCore.so .\lib\libPixVideoCore.so.symbol
    %OHOS_NDK%\native\llvm\bin\llvm-strip.exe -g -S -d --strip-debug .\lib\libPixVideoCore.so
    cd ..
)

