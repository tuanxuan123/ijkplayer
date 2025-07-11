
@echo off
set count=0
for %%x in (%*) do set /a count+=1
echo The number of parameters is: %count%
if %count% LSS 3 (
    echo Error: At least three parameters are required.
    echo arg1 vcvars64.bat path  eg. "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
    echo arg2 msys2_shell.cmd path  eg. "C:\msys64\msys2_shell.cmd"
    echo arg3 project pixui_ijkplayer/windows path  eg. "/e/video/pixui_ijkplayer/windows"
    echo eg. .\build-ffmpeg.bat "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" "C:\msys64\msys2_shell.cmd" "/e/video/pixui_ijkplayer/windows"
    exit 0
)

echo The parameter you passed is: %1 %2 %3
call %1
%2 -mingw64 -use-full-path -c ^
"cd %3/FFmpeg-ijkplayer;pwd; ^
 export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:$PWD/../libvorbis:$PWD/../libogg:$PWD/../openssl; ^
 pkg-config --list-all; make clean; make uninstall;^
 ./configure --enable-shared --disable-static --disable-x86asm --disable-gpl --enable-nonfree --enable-runtime-cpudetect --disable-gray --disable-swscale-alpha ^
 --disable-programs --enable-ffmpeg --enable-ffplay --disable-ffprobe --disable-doc --disable-htmlpages --disable-manpages --disable-podpages --disable-txtpages ^
 --enable-avdevice --enable-avcodec --enable-avformat --enable-avutil --enable-swresample --enable-swscale --disable-postproc --enable-avfilter --disable-avresample ^
 --enable-network --disable-small --disable-d3d11va --disable-dxva2 --disable-vaapi --disable-vdpau --disable-videotoolbox --disable-encoders --disable-decoders ^
 --enable-decoder=aac --enable-decoder=flv --enable-decoder=h264 --enable-decoder=mp3* --enable-decoder=mpeg4 --enable-decoder=hevc --disable-hwaccels ^
 --disable-muxers --enable-muxer=mp4 --disable-demuxers --enable-demuxer=mov --enable-demuxer=aac --enable-demuxer=concat --enable-demuxer=data --enable-demuxer=flv ^
 --enable-demuxer=hls --enable-demuxer=mpegts --enable-demuxer=mpegtsraw --enable-demuxer=mp3 --enable-demuxer=mpegvideo --disable-parsers --enable-parser=aac ^
 --enable-parser=aac_latm --enable-parser=h264 --enable-parser=flac --enable-parser=hevc --enable-bsfs --disable-bsf=chomp --disable-bsf=dca_core ^
 --disable-bsf=dump_extradata --disable-bsf=hevc_mp4toannexb --disable-bsf=imx_dump_header --disable-bsf=mjpeg2jpeg --disable-bsf=mjpega_dump_header ^
 --disable-bsf=mp3_header_decompress --disable-protocols --enable-protocol=async --enable-protocol=file --enable-protocol=http --enable-protocol=ffrtmphttp ^
 --enable-protocol=tcp --enable-protocol=https --enable-protocol=crypto --enable-protocol=hls --enable-protocol=httpproxy --enable-protocol=rtmp --disable-devices ^
 --disable-filters --disable-iconv --disable-audiotoolbox --disable-linux-perf --disable-zlib --arch=x86_64 --enable-demuxer=ogg --enable-demuxer=wav --enable-decoder=pcm* ^
 --toolchain=msvc --enable-protocol=ijksegment --enable-protocol=ijkio --disable-protocol=tls_schannel --enable-protocol=ijkhttphook ^
 --enable-decoder=vorbis --enable-decoder=ass --enable-demuxer=ass --enable-decoder=srt --enable-demuxer=srt --enable-decoder=movtext --enable-demuxer=movtext --enable-libvorbis --enable-openssl --enable-protocol=tls_openssl --prefix=build/x64 ^
 --extra-ldflags=\"-LIBPATH:%3/vorbis/x64\" ^
 --extra-ldflags=\"-LIBPATH:%3/ogg/x64\" ^
 --extra-cflags=\"-I%3/openssl\"; ^
 make -j8; make install"
 exit 0