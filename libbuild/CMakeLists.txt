cmake_minimum_required(VERSION 3.4.1)
project(video)
set(PlayerVersion 0.2.0)
set(PixVideoCoreLibName PixVideoCore)
set(PixVideoLibName PixVideo)

set(LIBRARY_OUTPUT_PATH  ${PROJECT_BINARY_DIR}/lib)

#路径设置
set(RootPath ${CMAKE_SOURCE_DIR}/..)
set(IJKMEDIA_DIR ${RootPath}/ijkmedia)
set(IJKSDL_DIR ${IJKMEDIA_DIR}/ijksdl)
set(IJKPLAYER_DIR ${IJKMEDIA_DIR}/ijkplayer)
set(LIBYUV_DIR ${IJKMEDIA_DIR}/libyuv)
set(IJKJ4A_DIR ${IJKMEDIA_DIR}/ijkj4a)
set(SOUNDTOUCH_DIR ${IJKMEDIA_DIR}/ijksoundtouch)
set(IOS_IJKSDL_DIR ${RootPath}/ijkmedia/ijksdl/ios)
set(IOS_IJKPLAYER_DIR ${RootPath}/ijkmedia/ijkplayer/ios)
set(PXAUTH_SRC_DIR ${IJKPLAYER_DIR}/pxauth)
set(PIXVIDEO_DIR ${IJKMEDIA_DIR}/pixvideo)

set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_C_VISIBILITY_PRESET hidden)

if(ONAUTH)
    add_definitions(-DUSING_PXAUTH)
endif()

if(OHOS)
    add_definitions(-DOHOS)
endif()

#更好区分osx和ios，两者都定义了APPLE
if (${CMAKE_SYSTEM_NAME} STREQUAL Darwin)
    set(OSX 1)
    add_definitions(-DTARGET_OS_MAC)
endif ()

if(MSVC)
    add_definitions(-D_SBCS -D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_DEPRECATE)
    #warning C4244: “=”: 从“double”转换到“int64_t”，可能丢失数据
    #warning C4267: “参数”: 从“size_t”转换到“uint32_t”，可能丢失数据
    #warning C4819: 该文件包含不能在当前代码 页(936)中表示的字
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  /W3 /wd4244 /wd4267 /wd4819")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  /W3 /wd4244 /wd4267 /wd4819 /wd4090")
else ()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W -Wno-long-long -Wno-unused-parameter -Wno-sign-compare")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -W -Wno-long-long -Wno-unused-parameter -Wno-sign-compare")
endif ()

set(CMAKE_XCODE_ATTRIBUTE_ENABLE_BITCODE "NO" CACHE INTERNAL "")


#源文件设置
    set(COMMON_SRC 
    # yuv
    ${LIBYUV_DIR}/source/compare.cc           
    ${LIBYUV_DIR}/source/compare_common.cc
    ${LIBYUV_DIR}/source/compare_gcc.cc
    ${LIBYUV_DIR}/source/compare_mmi.cc
    ${LIBYUV_DIR}/source/compare_msa.cc
    ${LIBYUV_DIR}/source/compare_neon.cc      
    ${LIBYUV_DIR}/source/compare_neon64.cc
    ${LIBYUV_DIR}/source/compare_win.cc

    ${LIBYUV_DIR}/source/convert.cc           
    ${LIBYUV_DIR}/source/convert_argb.cc      
    ${LIBYUV_DIR}/source/convert_from.cc      
    ${LIBYUV_DIR}/source/convert_from_argb.cc 
    ${LIBYUV_DIR}/source/convert_jpeg.cc
    ${LIBYUV_DIR}/source/convert_to_argb.cc   
    ${LIBYUV_DIR}/source/convert_to_i420.cc

    ${LIBYUV_DIR}/source/cpu_id.cc   
    ${LIBYUV_DIR}/source/mjpeg_decoder.cc  
    ${LIBYUV_DIR}/source/mjpeg_validate.cc         
    ${LIBYUV_DIR}/source/planar_functions.cc  

    ${LIBYUV_DIR}/source/rotate.cc
    ${LIBYUV_DIR}/source/rotate_any.cc
    ${LIBYUV_DIR}/source/rotate_argb.cc
    ${LIBYUV_DIR}/source/rotate_common.cc
    ${LIBYUV_DIR}/source/rotate_gcc.cc
    ${LIBYUV_DIR}/source/rotate_mmi.cc
    ${LIBYUV_DIR}/source/rotate_msa.cc
    ${LIBYUV_DIR}/source/rotate_neon.cc
    ${LIBYUV_DIR}/source/rotate_neon64.cc
    ${LIBYUV_DIR}/source/rotate_win.cc

    ${LIBYUV_DIR}/source/row_any.cc           
    ${LIBYUV_DIR}/source/row_common.cc
    ${LIBYUV_DIR}/source/row_gcc.cc
    ${LIBYUV_DIR}/source/row_mmi.cc
    ${LIBYUV_DIR}/source/row_msa.cc
    ${LIBYUV_DIR}/source/row_neon.cc          
    ${LIBYUV_DIR}/source/row_neon64.cc
    ${LIBYUV_DIR}/source/row_win.cc

    ${LIBYUV_DIR}/source/scale.cc             
    ${LIBYUV_DIR}/source/scale_any.cc         
    ${LIBYUV_DIR}/source/scale_argb.cc        
    ${LIBYUV_DIR}/source/scale_common.cc
    ${LIBYUV_DIR}/source/scale_gcc.cc
    ${LIBYUV_DIR}/source/scale_mmi.cc
    ${LIBYUV_DIR}/source/scale_msa.cc
    ${LIBYUV_DIR}/source/scale_neon.cc        
    ${LIBYUV_DIR}/source/scale_neon64.cc     
    ${LIBYUV_DIR}/source/scale_win.cc   
    ${LIBYUV_DIR}/source/video_common.cc

    #sdl
    ${IJKSDL_DIR}/ijksdl_log.c 
    ${IJKSDL_DIR}/ijksdl_error.c 
    ${IJKSDL_DIR}/ijksdl_timer.c 
    ${IJKSDL_DIR}/ijksdl_thread.c 
    ${IJKSDL_DIR}/ijksdl_mutex.c 
    ${IJKSDL_DIR}/ijksdl_stdinc.c 
    ${IJKSDL_DIR}/ijksdl_vout.c 
    ${IJKSDL_DIR}/ijksdl_aout.c 
    ${IJKSDL_DIR}/ijksdl_audio.c 

    ${IJKSDL_DIR}/ffmpeg/ijksdl_vout_overlay_ffmpeg.c 
    ${IJKSDL_DIR}/ffmpeg/abi_all/image_convert.c 

    #ijkplayer
    ${IJKPLAYER_DIR}/pipeline/ffpipenode_ffplay_vdec.c
    ${IJKPLAYER_DIR}/pipeline/ffpipenode_ffplay_adec.c   
    ${IJKPLAYER_DIR}/ff_ffplay.c 
    ${IJKPLAYER_DIR}/ff_cmdutils.c 
    ${IJKPLAYER_DIR}/ff_ffpipeline.c 
    ${IJKPLAYER_DIR}/ff_ffpipenode.c 
    ${IJKPLAYER_DIR}/ijkmeta.c 
    ${IJKPLAYER_DIR}/ijkplayer.c
    ${IJKPLAYER_DIR}/ijkplayer_delegate_def.c

    ${IJKPLAYER_DIR}/ijkavutil/ijkstl.c 
    ${IJKPLAYER_DIR}/ijkavutil/ijkdict.c 
    ${IJKPLAYER_DIR}/ijkavutil/ijkfifo.c 
    ${IJKPLAYER_DIR}/ijkavutil/ijkthreadpool.c 
    ${IJKPLAYER_DIR}/ijkavutil/ijktree.c 
    ${IJKPLAYER_DIR}/ijkavutil/ijkutils.c 

    ${IJKPLAYER_DIR}/ijkavformat/allformats.c 
    ${IJKPLAYER_DIR}/ijkavformat/ijkasync.c 
    ${IJKPLAYER_DIR}/ijkavformat/ijkio.c  
    ${IJKPLAYER_DIR}/ijkavformat/ijkioapplication.c 
    ${IJKPLAYER_DIR}/ijkavformat/ijkiocache.c 
    ${IJKPLAYER_DIR}/ijkavformat/ijkioffio.c 
    ${IJKPLAYER_DIR}/ijkavformat/ijkiomanager.c 
    ${IJKPLAYER_DIR}/ijkavformat/ijkioprotocol.c 
    ${IJKPLAYER_DIR}/ijkavformat/ijkiourlhook.c 
    ${IJKPLAYER_DIR}/ijkavformat/ijklivehook.c 
    ${IJKPLAYER_DIR}/ijkavformat/ijklongurl.c 
    ${IJKPLAYER_DIR}/ijkavformat/ijkmediadatasource.c 
    ${IJKPLAYER_DIR}/ijkavformat/ijksegment.c 
    ${IJKPLAYER_DIR}/ijkavformat/ijkurlhook.c
    ${IJKPLAYER_DIR}/ijkavformat/ijkcacheutil.c
)

# sound touch 先只加到android构建流程
set(ANDROID_ONLY_SRC
    ${IJKSDL_DIR}/android/ijksdl_android_jni.c
    ${IJKSDL_DIR}/android/android_audiotrack.c
    ${IJKSDL_DIR}/android/ijksdl_aout_android_audiotrack.c
    ${IJKSDL_DIR}/android/ijksdl_aout_android_opensles.c
    ${IJKSDL_DIR}/android/ijksdl_vout_android_null.c
    ${IJKSDL_DIR}/android/ijksdl_codec_android_mediadef.c
    ${IJKSDL_DIR}/android/ijksdl_vout_overlay_android_mediacodec.c
    ${IJKJ4A_DIR}/j4a/j4a_allclasses.c
    ${IJKJ4A_DIR}/j4a/j4a_base.c
    ${IJKJ4A_DIR}/j4a/class/android/media/AudioTrack.c
    ${IJKJ4A_DIR}/j4a/class/android/media/MediaCodec.c
    ${IJKJ4A_DIR}/j4a/class/android/media/MediaFormat.c
    ${IJKJ4A_DIR}/j4a/class/android/media/PlaybackParams.c
    ${IJKJ4A_DIR}/j4a/class/android/os/Build.c
    ${IJKJ4A_DIR}/j4a/class/android/os/Bundle.c
    ${IJKJ4A_DIR}/j4a/class/java/nio/Buffer.c
    ${IJKJ4A_DIR}/j4a/class/java/nio/ByteBuffer.c
    ${IJKJ4A_DIR}/j4a/class/java/util/ArrayList.c
    ${IJKJ4A_DIR}/j4a/class/tv/danmaku/ijk/media/player/misc/IMediaDataSource.c
    ${IJKJ4A_DIR}/j4a/class/tv/danmaku/ijk/media/player/misc/IAndroidIO.c
    ${IJKJ4A_DIR}/j4a/class/tv/danmaku/ijk/media/player/IjkMediaPlayer.c
    ${IJKJ4A_DIR}/j4au/class/android/media/AudioTrack.util.c
    ${IJKJ4A_DIR}/j4au/class/java/nio/ByteBuffer.util.c    
    ${IJKPLAYER_DIR}/pipeline/ffpipeline_ffplay.c
    ${IJKPLAYER_DIR}/android/ffmpeg_api_jni.c
    ${IJKPLAYER_DIR}/ijkavformat/ijkioandroidio.c

    ${SOUNDTOUCH_DIR}/source/SoundTouch/AAFilter.cpp
    ${SOUNDTOUCH_DIR}/source/SoundTouch/FIFOSampleBuffer.cpp
    ${SOUNDTOUCH_DIR}/source/SoundTouch/FIRFilter.cpp
    ${SOUNDTOUCH_DIR}/source/SoundTouch/cpu_detect_x86.cpp
    ${SOUNDTOUCH_DIR}/source/SoundTouch/sse_optimized.cpp
    ${SOUNDTOUCH_DIR}/source/SoundTouch/RateTransposer.cpp
    ${SOUNDTOUCH_DIR}/source/SoundTouch/InterpolateCubic.cpp
    ${SOUNDTOUCH_DIR}/source/SoundTouch/InterpolateLinear.cpp
    ${SOUNDTOUCH_DIR}/source/SoundTouch/InterpolateShannon.cpp
    ${SOUNDTOUCH_DIR}/source/SoundTouch/TDStretch.cpp
    ${SOUNDTOUCH_DIR}/source/SoundTouch/BPMDetect.cpp
    ${SOUNDTOUCH_DIR}/source/SoundTouch/PeakFinder.cpp
    ${SOUNDTOUCH_DIR}/source/SoundTouch/SoundTouch.cpp
    ${SOUNDTOUCH_DIR}/source/SoundTouch/mmx_optimized.cpp
    ${SOUNDTOUCH_DIR}/ijksoundtouch_wrap.cpp
)

set(HARMONY_ONLY_SRC
    ${IJKSDL_DIR}/android/ijksdl_vout_overlay_android_mediacodec.c

    ${SOUNDTOUCH_DIR}/source/SoundTouch/AAFilter.cpp
    ${SOUNDTOUCH_DIR}/source/SoundTouch/FIFOSampleBuffer.cpp
    ${SOUNDTOUCH_DIR}/source/SoundTouch/FIRFilter.cpp
    ${SOUNDTOUCH_DIR}/source/SoundTouch/cpu_detect_x86.cpp
    ${SOUNDTOUCH_DIR}/source/SoundTouch/sse_optimized.cpp
    ${SOUNDTOUCH_DIR}/source/SoundTouch/RateTransposer.cpp
    ${SOUNDTOUCH_DIR}/source/SoundTouch/InterpolateCubic.cpp
    ${SOUNDTOUCH_DIR}/source/SoundTouch/InterpolateLinear.cpp
    ${SOUNDTOUCH_DIR}/source/SoundTouch/InterpolateShannon.cpp
    ${SOUNDTOUCH_DIR}/source/SoundTouch/TDStretch.cpp
    ${SOUNDTOUCH_DIR}/source/SoundTouch/BPMDetect.cpp
    ${SOUNDTOUCH_DIR}/source/SoundTouch/PeakFinder.cpp
    ${SOUNDTOUCH_DIR}/source/SoundTouch/SoundTouch.cpp
    ${SOUNDTOUCH_DIR}/source/SoundTouch/mmx_optimized.cpp
    ${SOUNDTOUCH_DIR}/ijksoundtouch_wrap.cpp
)

set(IPHONE_ONLY_SRC
    ${IJKSDL_DIR}/ios/ijksdl_vout_ios_null.c 
    ${IOS_IJKSDL_DIR}/ijksdl_aout_ios_audiounit.m 
    ${IOS_IJKPLAYER_DIR}/ijkplayer_ios.m
    ${IOS_IJKPLAYER_DIR}/px_lib_info.m   
    ${IOS_IJKPLAYER_DIR}/pipeline/IJKVideoToolBoxAsync.m  
    ${IOS_IJKPLAYER_DIR}/pipeline/IJKVideoToolBoxSync.m  
    ${IOS_IJKPLAYER_DIR}/pipeline/IJKVideoToolBox.m  
    ${IOS_IJKPLAYER_DIR}/pipeline/ffpipenode_ios_videotoolbox_vdec.m
    ${IOS_IJKPLAYER_DIR}/pipeline/ffpipeline_ios.c 
)

set(MAC_ONLY_SRC
    ${RootPath}/mac/PixVideo/libavutil/application.c  
    ${IJKSDL_DIR}/mac/ijksdl_aout_mac_audiounit.m 
    ${IJKSDL_DIR}/mac/ijksdl_vout_mac_null.c 
    ${IJKPLAYER_DIR}/mac/ijkplayer_mac.c  
    ${IJKPLAYER_DIR}/mac/pipeline/ffpipeline_mac.c  
)

set(IPHONE_MAC_COMMON_SRC
    ${IOS_IJKSDL_DIR}/ijksdl_vout_overlay_videotoolbox.m
    ${IOS_IJKSDL_DIR}/IJKSDLAudioKit.m
    ${IOS_IJKSDL_DIR}/IJKSDLAudioQueueController.m
    ${IOS_IJKSDL_DIR}/ijksdl_thread_ios.m
)

set(WINDOWS_ONLY_SRC 
    ${IJKSDL_DIR}/ijksdl_windows_interface.c
    ${IJKSDL_DIR}/windows/ijksdl_aout_windows_directsound.c
    ${IJKSDL_DIR}/windows/ijksdl_aout_windows_xaudio2.c
    ${IJKSDL_DIR}/windows/ijksdl_vout_windows_null.c

    ${IJKPLAYER_DIR}/windows/pipeline/ffpipeline_windows.c
    ${IJKPLAYER_DIR}/windows/ijkplayer_windows.c
    
)

#头文件设置
set(COMMON_HEADER 
    libs/include
    ${IJKMEDIA_DIR}
    ${IJKSDL_DIR}
    ${IJKSDL_DIR}/ffmpeg
    ${IJKPLAYER_DIR}
    ${IJKPLAYER_DIR}/pipeline
    ${LIBYUV_DIR}/include
    ${PXAUTH_SRC_DIR}
    ${IJKMEDIA_DIR}/pixvideo
)

set(ANDROID_ONLY_HEADER 
    ${RootPath}/android/videoplayer/app/libs/include
    ${IJKMEDIA_DIR}/ijkj4a
    ${SOUNDTOUCH_DIR}
    ${SOUNDTOUCH_DIR}/include
    ${IJKPLAYER_DIR}/android/pipeline
    ${IJKPLAYER_DIR}/android
    ${IJKSDL_DIR}/android
    ${PIXVIDEO_DIR}/android/sdl
)

set(HARMONY_ONLY_HEADER
    ${SOUNDTOUCH_DIR}
    ${SOUNDTOUCH_DIR}/include
    ${IJKPLAYER_DIR}/harmony
    ${IJKPLAYER_DIR}/harmony/pipeline
    ${RootPath}/harmony/libffmpeg/include
)

set(IPHONE_ONLY_HEADER 
    ${RootPath}/ios/libffmpeg-all/include
    ${IJKSDL_DIR}/ios
)

set(MAC_ONLY_HEADER 
    ${RootPath}/mac/libffmpeg/include
    ${IJKPLAYER_DIR}/mac
    ${IJKPLAYER_DIR}/mac/pipeline
    ${IJKSDL_DIR}/mac
    ${PIXVIDEO_DIR}/ios/pipeline
)

set(IPHONE_MAC_COMMON_HEADER
    ${RootPath}/ios/IJKMediaPlayer/IJKMediaPlayer/ijkmedia
    ${IOS_IJKSDL_DIR}
    ${IOS_IJKPLAYER_DIR}
)

set(WINDOWS_ONLY_HEADER 
    ${RootPath}/windows/libffmpeg/include
    ${RootPath}/windows/pthread/include
    ${IJKPLAYER_DIR}/windows
    ${IJKPLAYER_DIR}/windows/pipeline
    ${IJKSDL_DIR}/windows
)

#外部链接库设置
if(ANDROID)
    MESSAGE(STATUS "Build Android Video .......")
    find_library(log-lib log)

    add_library(PixFFmpeg SHARED IMPORTED)
    set_target_properties(PixFFmpeg PROPERTIES IMPORTED_LOCATION ${RootPath}/android/videoplayer/app/libs/${ANDROID_ABI}/libPixFFmpeg.so)
    
    add_library(pxauth SHARED IMPORTED)
    set_target_properties(pxauth PROPERTIES IMPORTED_LOCATION ${RootPath}/android/contrib/pxauth/lib/${ANDROID_ABI}/libpxauth.so)

    add_library(pxavalidator STATIC IMPORTED)
    set_target_properties(pxavalidator PROPERTIES IMPORTED_LOCATION ${RootPath}/android/contrib/pxauth/lib/${ANDROID_ABI}/libpxavalidator.a)

    set(EXTRA_LIB ${log-lib} PixFFmpeg OpenSLES mediandk android pxauth pxavalidator)
endif(ANDROID)


if(OHOS)
    MESSAGE(STATUS "Build Harmony Video .......")

    add_library(libavcodec STATIC IMPORTED)
    add_library(libavformat STATIC IMPORTED)
    add_library(libavutil STATIC IMPORTED)
    add_library(libswresample STATIC IMPORTED)

    set_target_properties(libavcodec PROPERTIES IMPORTED_LOCATION ${RootPath}/harmony/libffmpeg/lib/${OHOS_ARCH}/libavcodec.a)
    set_target_properties(libavformat PROPERTIES IMPORTED_LOCATION ${RootPath}/harmony/libffmpeg/lib/${OHOS_ARCH}/libavformat.a)
    set_target_properties(libavutil PROPERTIES IMPORTED_LOCATION ${RootPath}/harmony/libffmpeg/lib/${OHOS_ARCH}/libavutil.a)
    set_target_properties(libswresample PROPERTIES IMPORTED_LOCATION ${RootPath}/harmony/libffmpeg/lib/${OHOS_ARCH}/libswresample.a)

    add_library(libogg STATIC IMPORTED)
    add_library(libvorbis STATIC IMPORTED)
    set_target_properties(libogg PROPERTIES IMPORTED_LOCATION ${RootPath}/harmony/libvorbis/lib/${OHOS_ARCH}/libogg.a)
    set_target_properties(libvorbis PROPERTIES IMPORTED_LOCATION ${RootPath}/harmony/libvorbis/lib/${OHOS_ARCH}/libvorbis.a)

    add_library(libcrypto STATIC IMPORTED)
    add_library(libssl STATIC IMPORTED)
    set_target_properties(libcrypto PROPERTIES IMPORTED_LOCATION ${RootPath}/harmony/openssl/lib/${OHOS_ARCH}/libcrypto.a)
    set_target_properties(libssl PROPERTIES IMPORTED_LOCATION ${RootPath}/harmony/openssl/lib/${OHOS_ARCH}/libssl.a)

    add_library(pxauth SHARED IMPORTED)
    set_target_properties(pxauth PROPERTIES IMPORTED_LOCATION ${RootPath}/harmony/pxauth/lib/${OHOS_ARCH}/libpxauth.so)

    add_library(pxavalidator STATIC IMPORTED)
    set_target_properties(pxavalidator PROPERTIES IMPORTED_LOCATION ${RootPath}/harmony/pxauth/lib/${OHOS_ARCH}/libpxavalidator.a)

    set(EXTRA_LIB libogg libvorbis libcrypto libssl libavcodec libavformat libavutil libswresample pxauth pxavalidator libohaudio.so libnative_media_vdec.so OpenSLES libnative_media_codecbase.so libnative_media_core.so libnative_media_acodec.so libhilog_ndk.z.so libnative_image.so)

endif(OHOS)

if(IOS)
    MESSAGE(STATUS "Build IOS Video .......") 

    add_library(libPixFFmpeg STATIC IMPORTED)
    set_target_properties(libPixFFmpeg PROPERTIES IMPORTED_LOCATION ${RootPath}/ios/libffmpeg-all/lib/libPixFFmpeg.a)

    set(SYSTEM_FRAMEWORK "-framework AudioToolBox" "-framework CoreVideo" "-framework VideoToolBox" "-framework CoreMedia" "-framework UIKit")    

    set(EXTRA_LIB ${SYSTEM_FRAMEWORK} libavcodec libavformat libavutil libswresample libvorbis libcrypto libssl)

endif(IOS)

if(OSX)
    MESSAGE(STATUS "Build Mac Video .......")   
    
    add_library(PixFFmpeg STATIC IMPORTED)
    set_target_properties(PixFFmpeg PROPERTIES IMPORTED_LOCATION ${RootPath}/mac/libffmpeg/lib/libPixFFmpeg.a)

    add_library(pxauth SHARED IMPORTED)
    set_target_properties(pxauth PROPERTIES IMPORTED_LOCATION ${RootPath}/mac/pxauth/lib/libpxauth.dylib)

    add_library(pxavalidator STATIC IMPORTED)
    set_target_properties(pxavalidator PROPERTIES IMPORTED_LOCATION ${RootPath}/mac/pxauth/lib/libpxavalidator.a)

    set(EXTRA_LIB PixFFmpeg pxauth pxavalidator "-framework AudioToolbox" "-framework Security" "-framework CoreMedia" "-framework Foundation" "-framework CoreVideo")
endif(OSX)

if(WIN32)
    MESSAGE(STATUS "Build Windows Video .......")
    add_library(avcodec SHARED IMPORTED)
    set_target_properties(avcodec PROPERTIES IMPORTED_IMPLIB ${RootPath}/windows/libffmpeg/x64/avcodec.lib)
    
    add_library(avdevice SHARED IMPORTED)
    set_target_properties(avdevice PROPERTIES IMPORTED_IMPLIB ${RootPath}/windows/libffmpeg/x64/avdevice.lib)
    
    add_library(avfilter SHARED IMPORTED)
    set_target_properties(avfilter PROPERTIES IMPORTED_IMPLIB ${RootPath}/windows/libffmpeg/x64/avfilter.lib)
    
    add_library(avformat SHARED IMPORTED)
    set_target_properties(avformat PROPERTIES IMPORTED_IMPLIB ${RootPath}/windows/libffmpeg/x64/avformat.lib)
    
    add_library(avutil SHARED IMPORTED)
    set_target_properties(avutil PROPERTIES IMPORTED_IMPLIB ${RootPath}/windows/libffmpeg/x64/avutil.lib)
    
    add_library(swresample SHARED IMPORTED)
    set_target_properties(swresample PROPERTIES IMPORTED_IMPLIB ${RootPath}/windows/libffmpeg/x64/swresample.lib)
    
    add_library(swscale SHARED IMPORTED)
    set_target_properties(swscale PROPERTIES IMPORTED_IMPLIB ${RootPath}/windows/libffmpeg/x64/swscale.lib)
    
    add_library(pthreadVC2 SHARED IMPORTED)
    set_target_properties(pthreadVC2 PROPERTIES IMPORTED_IMPLIB ${RootPath}/windows/libffmpeg/x64/pthreadVC2.lib)
    
    add_library(ogg SHARED IMPORTED)
    set_target_properties(ogg PROPERTIES IMPORTED_IMPLIB ${RootPath}/windows/libogg/x64/ogg.lib)
    
    add_library(libvorbis SHARED IMPORTED)
    set_target_properties(libvorbis PROPERTIES IMPORTED_IMPLIB ${RootPath}/windows/libvorbis/x64/vorbis.lib)
    
    add_library(dsound SHARED IMPORTED)
    set_target_properties(dsound PROPERTIES IMPORTED_IMPLIB ${RootPath}/windows/demo/directx/Lib/x64/dsound.lib)

    set(EXTRA_LIB avcodec avdevice avfilter avformat avutil swresample swscale pthreadVC2 ogg libvorbis dsound)
endif(WIN32)

if(ANDROID)
    if(ANDROID_ABI STREQUAL armeabi-v7a)
        set_source_files_properties(${LIBYUV_DIR}/source/compare_neon.cc PROPERTIES COMPILE_FLAGS -mfpu=neon)
        set_source_files_properties(${LIBYUV_DIR}/source/rotate_neon.cc PROPERTIES COMPILE_FLAGS -mfpu=neon)
        set_source_files_properties(${LIBYUV_DIR}/source/row_neon.cc PROPERTIES COMPILE_FLAGS -mfpu=neon)
        set_source_files_properties(${LIBYUV_DIR}/source/scale_neon.cc PROPERTIES COMPILE_FLAGS -mfpu=neon)
        add_definitions(-D__ARM_NEON__=1)
    endif()
   
    add_library(${PixVideoCoreLibName} SHARED ${COMMON_SRC} ${ANDROID_ONLY_SRC})

    target_include_directories(${PixVideoCoreLibName} PRIVATE ${COMMON_HEADER} ${ANDROID_ONLY_HEADER})

    # 隐藏静态库的符号，优先链接本so中的符号
    target_link_options(${PixVideoCoreLibName} PUBLIC "-Wl,--exclude-libs,ALL,-Bsymbolic")
    target_link_options(${PixVideoCoreLibName} PRIVATE "-Wl,-z,max-page-size=16384")
    target_link_options(${PixVideoCoreLibName} PRIVATE "-Wl,-z,common-page-size=16384")

    #减小release so体积
    If (NOT CMAKE_BUILD_TYPE STREQUAL Debug)
        add_custom_command(TARGET ${PixVideoCoreLibName} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PixVideoCoreLibName}> $<TARGET_FILE:${PixVideoCoreLibName}>.symbol
            COMMENT "copy so for keep symbol.")
        add_custom_command(TARGET ${PixVideoCoreLibName} POST_BUILD
                COMMAND ${ANDROID_TOOLCHAIN_PREFIX}strip -s -x $<TARGET_FILE:${PixVideoCoreLibName}>
                COMMENT "Strip debug symbols done on final binary.")
    endif ()
elseif(OHOS)
    set(CMAKE_SHARED_LINKER_FLAGS "-Wl,-Bsymbolic")
    add_compile_options(-fPIC)
    add_library(${PixVideoCoreLibName} SHARED ${COMMON_SRC} ${HARMONY_ONLY_SRC})
    target_include_directories(${PixVideoCoreLibName} PRIVATE ${COMMON_HEADER} ${HARMONY_ONLY_HEADER})

    # 隐藏静态库的符号，优先链接本so中的符号
    #target_link_options(${PixVideoCoreLibName} PUBLIC "-Wl,--exclude-libs,ALL,-Bsymbolic")

elseif(IOS)
   
    MESSAGE("build ios shared lib")
    add_definitions(-DTARGET_OS_IPHONE)
    set(LINK_FLAGS    "-sectcreate __LLVM __bundle ${PROJECT_SOURCE_DIR}/llvm_flag.txt")
    set(CMAKE_SHARED_LINKER_FLAGS "${LINK_FLAGS}")


    add_library(${PixVideoCoreLibName} SHARED 
        ${COMMON_SRC} ${IPHONE_MAC_COMMON_SRC} ${IPHONE_ONLY_SRC}
    )


    set_target_properties(${PixVideoCoreLibName} PROPERTIES
        FRAMEWORK TRUE
        FRAMEWORK_VERSION C
        MACOSX_FRAMEWORK_IDENTIFIER com.vasd.pandora.videoplayer
        MACOSX_FRAMEWORK_SHORT_VERSION_STRING ${PlayerVersion}
        MACOSX_FRAMEWORK_BUNDLE_VERSION ${PlayerVersion}
    )

    target_include_directories(${PixVideoCoreLibName} PRIVATE ${COMMON_HEADER} ${IPHONE_MAC_COMMON_HEADER} ${IPHONE_ONLY_HEADER} )
    

elseif(OSX)
    add_library(${PixVideoCoreLibName} SHARED ${COMMON_SRC} ${IPHONE_MAC_COMMON_SRC} ${MAC_ONLY_SRC})
    target_include_directories(${PixVideoCoreLibName} PRIVATE ${COMMON_HEADER} ${IPHONE_MAC_COMMON_HEADER} ${MAC_ONLY_HEADER} )

elseif(WIN32)
    add_library(${PixVideoCoreLibName} SHARED ${COMMON_SRC} ${WINDOWS_ONLY_SRC})
    target_include_directories(${PixVideoCoreLibName} PRIVATE ${COMMON_HEADER} ${WINDOWS_ONLY_HEADER} )
endif()

target_link_libraries(${PixVideoCoreLibName} ${EXTRA_LIB})


if (APPLE)
    MESSAGE(STATUS "APPLE Set extra property")
    set_property(TARGET ${PixVideoCoreLibName} APPEND_STRING PROPERTY COMPILE_FLAGS " -fno-objc-arc")
    target_compile_options(${PixVideoCoreLibName} PUBLIC $<$<COMPILE_LANGUAGE:CXX>:-x objective-c++>)
    target_compile_options(${PixVideoCoreLibName} PUBLIC $<$<COMPILE_LANGUAGE:C>:-x objective-c>)
endif ()

if(NOT ${CMAKE_HOST_SYSTEM} MATCHES Windows)
    add_custom_command(TARGET ${PixVideoCoreLibName} POST_BUILD COMMAND ls -l $<TARGET_FILE:${PixVideoCoreLibName}>)
endif ()




############################## PixVideoLibName #############################
set(COMMON_PIXVIDEO_SRC 
    ${PIXVIDEO_DIR}/h5_player.c 
    ${PIXVIDEO_DIR}/pandora_player.c
    ${PIXVIDEO_DIR}/authorization.c
    ${PIXVIDEO_DIR}/player_utils.c
    ${PIXVIDEO_DIR}/cJSON.c  
)

set(ANDROID_ONLY_PIXVIDEO_SRC
    ${PIXVIDEO_DIR}/android/pipeline/ffpipeline_native_android.c
    ${PIXVIDEO_DIR}/android/pipeline/ffpipenode_native_mediacodec_vdec.c
    ${PIXVIDEO_DIR}/android/pipeline/ffpipenode_native_mediacodec_adec.c
    ${PIXVIDEO_DIR}/android/ijkplayer_android.c
    ${PIXVIDEO_DIR}/android/sdl/ijksdl_aout_android_aaudio.c
)

set(IOS_ONLY_PIXVIDEO_SRC
    ${PIXVIDEO_DIR}/ios/pipeline/ffpipenode_ios_audiotoolbox_adec.m
)

set(MAC_ONLY_PIXVIDEO_SRC
    ${PIXVIDEO_DIR}/ios/pipeline/ffpipenode_ios_audiotoolbox_adec.m
)

set(HARMONY_ONLY_PIXVIDEO_SRC
    ${PIXVIDEO_DIR}/harmony/sdl/ijksdl_aout_harmony_audio.c
    ${PIXVIDEO_DIR}/harmony/sdl/ijksdl_vout_harmony.c
    ${PIXVIDEO_DIR}/harmony/ijkplayer_harmony.c
    ${PIXVIDEO_DIR}/harmony/pipeline/ffpipeline_harmony.c
    ${PIXVIDEO_DIR}/harmony/pipeline/ffpipenode_harmony_vdec.c
)

if(ANDROID)
    if(ANDROID_ABI STREQUAL armeabi-v7a)
        set_source_files_properties(${LIBYUV_DIR}/source/compare_neon.cc PROPERTIES COMPILE_FLAGS -mfpu=neon)
        set_source_files_properties(${LIBYUV_DIR}/source/rotate_neon.cc PROPERTIES COMPILE_FLAGS -mfpu=neon)
        set_source_files_properties(${LIBYUV_DIR}/source/row_neon.cc PROPERTIES COMPILE_FLAGS -mfpu=neon)
        set_source_files_properties(${LIBYUV_DIR}/source/scale_neon.cc PROPERTIES COMPILE_FLAGS -mfpu=neon)
        add_definitions(-D__ARM_NEON__=1)
    endif()
   
    add_library(${PixVideoLibName} SHARED ${COMMON_PIXVIDEO_SRC} ${ANDROID_ONLY_PIXVIDEO_SRC})

    target_include_directories(${PixVideoLibName} PRIVATE ${COMMON_HEADER} ${ANDROID_ONLY_HEADER})

    # 隐藏静态库的符号，优先链接本so中的符号
    target_link_options(${PixVideoLibName} PUBLIC "-Wl,--exclude-libs,ALL,-Bsymbolic")
    target_link_options(${PixVideoLibName} PRIVATE "-Wl,-z,max-page-size=16384")
    target_link_options(${PixVideoLibName} PRIVATE "-Wl,-z,common-page-size=16384")

    #减小release so体积
    If (NOT CMAKE_BUILD_TYPE STREQUAL Debug)
        add_custom_command(TARGET ${PixVideoLibName} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PixVideoLibName}> $<TARGET_FILE:${PixVideoLibName}>.symbol
            COMMENT "copy so for keep symbol.")
        add_custom_command(TARGET ${PixVideoLibName} POST_BUILD
                COMMAND ${ANDROID_TOOLCHAIN_PREFIX}strip -s -x $<TARGET_FILE:${PixVideoLibName}>
                COMMENT "Strip debug symbols done on final binary.")
    endif ()
elseif(OHOS)
    set(CMAKE_SHARED_LINKER_FLAGS "-Wl,-Bsymbolic")
    add_compile_options(-fPIC)
    add_library(${PixVideoLibName} SHARED ${COMMON_PIXVIDEO_SRC} ${HARMONY_ONLY_PIXVIDEO_SRC})
    target_include_directories(${PixVideoLibName} PRIVATE ${COMMON_HEADER} ${HARMONY_ONLY_HEADER})

    # 隐藏静态库的符号，优先链接本so中的符号
    #target_link_options(${PixVideoLibName} PUBLIC "-Wl,--exclude-libs,ALL,-Bsymbolic")

elseif(IOS)
   
    MESSAGE("build ios shared lib")
    add_definitions(-DTARGET_OS_IPHONE)
    set(LINK_FLAGS    "-sectcreate __LLVM __bundle ${PROJECT_SOURCE_DIR}/llvm_flag.txt")
    set(CMAKE_SHARED_LINKER_FLAGS "${LINK_FLAGS}")


    add_library(${PixVideoLibName} SHARED 
        ${COMMON_PIXVIDEO_SRC}
        ${IOS_ONLY_PIXVIDEO_SRC}
    )


    set_target_properties(${PixVideoLibName} PROPERTIES
        FRAMEWORK TRUE
        FRAMEWORK_VERSION C
        MACOSX_FRAMEWORK_IDENTIFIER com.vasd.pandora.videoplayer
        MACOSX_FRAMEWORK_SHORT_VERSION_STRING ${PlayerVersion}
        MACOSX_FRAMEWORK_BUNDLE_VERSION ${PlayerVersion}
    )

    target_include_directories(${PixVideoLibName} PRIVATE ${COMMON_HEADER} ${IPHONE_MAC_COMMON_HEADER} ${IPHONE_ONLY_HEADER} )
    

elseif(OSX)
    add_library(${PixVideoLibName} SHARED ${COMMON_PIXVIDEO_SRC} ${MAC_ONLY_PIXVIDEO_SRC})
    target_include_directories(${PixVideoLibName} PRIVATE ${COMMON_HEADER} ${IPHONE_MAC_COMMON_HEADER} ${MAC_ONLY_HEADER} )
    add_custom_command(TARGET ${PixVideoLibName} POST_BUILD
        COMMAND ${CMAKE_INSTALL_NAME_TOOL} -add_rpath "@loader_path" $<TARGET_FILE:${PixVideoLibName}>
        COMMENT "add @loader_path into rpath for ${PixVideoLibName}"
    )
elseif(WIN32)
    add_library(${PixVideoLibName} SHARED ${COMMON_PIXVIDEO_SRC})
    target_include_directories(${PixVideoLibName} PRIVATE ${COMMON_HEADER} ${WINDOWS_ONLY_HEADER} )
endif()

target_link_libraries(${PixVideoLibName} ${PixVideoCoreLibName})


if (APPLE)
    MESSAGE(STATUS "APPLE Set extra property")
    set_property(TARGET ${PixVideoLibName} APPEND_STRING PROPERTY COMPILE_FLAGS " -fno-objc-arc")
    target_compile_options(${PixVideoLibName} PUBLIC $<$<COMPILE_LANGUAGE:CXX>:-x objective-c++>)
    target_compile_options(${PixVideoLibName} PUBLIC $<$<COMPILE_LANGUAGE:C>:-x objective-c>)
endif ()

if(NOT ${CMAKE_HOST_SYSTEM} MATCHES Windows)
    add_custom_command(TARGET ${PixVideoLibName} POST_BUILD COMMAND ls -l $<TARGET_FILE:${PixVideoLibName}>)
endif ()