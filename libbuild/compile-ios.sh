cd ../ios/PixVideo_framework

FF_ARG1=$1

xcodebuild clean -project PixVideo_framework.xcodeproj -target PixVideoCore
xcodebuild clean -project PixVideo_framework.xcodeproj -target PixVideo_framework
if [ "$FF_ARG1" = "pxauth" ]; then
    xcodebuild -project PixVideo_framework.xcodeproj -target PixVideoCore GCC_PREPROCESSOR_DEFINITIONS='$GCC_PREPROCESSOR_DEFINITIONS USING_PXAUTH=1'
    xcodebuild -project PixVideo_framework.xcodeproj -target PixVideo_framework GCC_PREPROCESSOR_DEFINITIONS='$GCC_PREPROCESSOR_DEFINITIONS USING_PXAUTH=1'
else
    xcodebuild -project PixVideo_framework.xcodeproj -target PixVideoCore
    xcodebuild -project PixVideo_framework.xcodeproj -target PixVideo_framework
fi
