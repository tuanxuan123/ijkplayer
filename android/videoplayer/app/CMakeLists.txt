cmake_minimum_required(VERSION 3.4.1)


set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/outputs/${ANDROID_ABI})

set(LibNewVideoName pixvideocore)
set(IJK_SRC_DIR ${CMAKE_SOURCE_DIR}/../../../ijkmedia)
set(IJKSDL_SRC_DIR ${CMAKE_SOURCE_DIR}/../../../ijkmedia/ijksdl)
set(IJKPLAYER_SRC_DIR ${CMAKE_SOURCE_DIR}/../../../ijkmedia/ijkplayer)
set(PXAUTH_SRC_DIR ${CMAKE_SOURCE_DIR}/../../../ijkmedia/ijkplayer/pxauth)
set(IJKJ4A_SRC_DIR ${CMAKE_SOURCE_DIR}/../../../ijkmedia/ijkj4a)
set(LIBYUV_SRC_DIR ${CMAKE_SOURCE_DIR}/../../../ijkmedia/libyuv)
set(SOUNDTOUCH_SRC_DIR ${CMAKE_SOURCE_DIR}/../../../ijkmedia/ijksoundtouch)
set(PIXVIDEO_SRC_DIR ${CMAKE_SOURCE_DIR}/../../../ijkmedia/pixvideo)

find_library(log-lib log)

FILE(GLOB libVideoSource
    ${IJKSDL_SRC_DIR}/ijksdl_log.c
    ${IJKSDL_SRC_DIR}/ijksdl_aout.c
    ${IJKSDL_SRC_DIR}/ijksdl_audio.c
    ${IJKSDL_SRC_DIR}/ijksdl_error.c
    ${IJKSDL_SRC_DIR}/ijksdl_timer.c
    ${IJKSDL_SRC_DIR}/ijksdl_thread.c
    ${IJKSDL_SRC_DIR}/ijksdl_mutex.c
    ${IJKSDL_SRC_DIR}/ijksdl_stdinc.c
    ${IJKSDL_SRC_DIR}/ijksdl_vout.c
    ${IJKSDL_SRC_DIR}/ffmpeg/ijksdl_vout_overlay_ffmpeg.c
    ${IJKSDL_SRC_DIR}/ffmpeg/abi_all/image_convert.c
    ${IJKSDL_SRC_DIR}/android/ijksdl_android_jni.c
    ${IJKSDL_SRC_DIR}/android/android_audiotrack.c
    ${IJKSDL_SRC_DIR}/android/ijksdl_aout_android_audiotrack.c
    ${IJKSDL_SRC_DIR}/android/ijksdl_aout_android_opensles.c
    ${IJKSDL_SRC_DIR}/android/ijksdl_vout_android_null.c
    ${IJKSDL_SRC_DIR}/android/ijksdl_vout_overlay_android_mediacodec.c
    ${IJKSDL_SRC_DIR}/android/ijksdl_codec_android_mediadef.c




    ${IJKJ4A_SRC_DIR}/j4a/j4a_allclasses.c
    ${IJKJ4A_SRC_DIR}/j4a/j4a_base.c
    ${IJKJ4A_SRC_DIR}/j4a/class/android/media/AudioTrack.c
    ${IJKJ4A_SRC_DIR}/j4a/class/android/media/MediaCodec.c
    ${IJKJ4A_SRC_DIR}/j4a/class/android/media/MediaFormat.c
    ${IJKJ4A_SRC_DIR}/j4a/class/android/media/PlaybackParams.c
    ${IJKJ4A_SRC_DIR}/j4a/class/android/os/Build.c
    ${IJKJ4A_SRC_DIR}/j4a/class/android/os/Bundle.c
    ${IJKJ4A_SRC_DIR}/j4a/class/java/nio/Buffer.c
    ${IJKJ4A_SRC_DIR}/j4a/class/java/nio/ByteBuffer.c
    ${IJKJ4A_SRC_DIR}/j4a/class/java/util/ArrayList.c
    ${IJKJ4A_SRC_DIR}/j4a/class/tv/danmaku/ijk/media/player/misc/IMediaDataSource.c
    ${IJKJ4A_SRC_DIR}/j4a/class/tv/danmaku/ijk/media/player/misc/IAndroidIO.c
    ${IJKJ4A_SRC_DIR}/j4a/class/tv/danmaku/ijk/media/player/IjkMediaPlayer.c
    ${IJKJ4A_SRC_DIR}/j4au/class/android/media/AudioTrack.util.c
    ${IJKJ4A_SRC_DIR}/j4au/class/java/nio/ByteBuffer.util.c

    ${IJKPLAYER_SRC_DIR}/ff_cmdutils.c
    ${IJKPLAYER_SRC_DIR}/ff_ffplay.c
    ${IJKPLAYER_SRC_DIR}/ff_ffpipeline.c
    ${IJKPLAYER_SRC_DIR}/ff_ffpipenode.c
    ${IJKPLAYER_SRC_DIR}/ijkmeta.c
    ${IJKPLAYER_SRC_DIR}/ijkplayer.c
    ${IJKPLAYER_SRC_DIR}/ijkplayer_delegate_def.c
    ${IJKPLAYER_SRC_DIR}/pipeline/ffpipeline_ffplay.c
    ${IJKPLAYER_SRC_DIR}/pipeline/ffpipenode_ffplay_vdec.c
    ${IJKPLAYER_SRC_DIR}/pipeline/ffpipenode_ffplay_adec.c
    ${IJKPLAYER_SRC_DIR}/android/ffmpeg_api_jni.c


    ${IJKPLAYER_SRC_DIR}/ijkavutil/ijkdict.c
    ${IJKPLAYER_SRC_DIR}/ijkavutil/ijkutils.c
    ${IJKPLAYER_SRC_DIR}/ijkavutil/ijkthreadpool.c
    ${IJKPLAYER_SRC_DIR}/ijkavutil/ijktree.c
    ${IJKPLAYER_SRC_DIR}/ijkavutil/ijkfifo.c
    ${IJKPLAYER_SRC_DIR}/ijkavutil/ijkstl.c

    ${IJKPLAYER_SRC_DIR}/ijkavformat/allformats.c
    ${IJKPLAYER_SRC_DIR}/ijkavformat/ijkasync.c
    ${IJKPLAYER_SRC_DIR}/ijkavformat/ijkio.c
    ${IJKPLAYER_SRC_DIR}/ijkavformat/ijkioapplication.c
    ${IJKPLAYER_SRC_DIR}/ijkavformat/ijkiocache.c
    ${IJKPLAYER_SRC_DIR}/ijkavformat/ijkioffio.c
    ${IJKPLAYER_SRC_DIR}/ijkavformat/ijkiomanager.c
    ${IJKPLAYER_SRC_DIR}/ijkavformat/ijkioprotocol.c
    ${IJKPLAYER_SRC_DIR}/ijkavformat/ijkioandroidio.c
    ${IJKPLAYER_SRC_DIR}/ijkavformat/ijkiourlhook.c
    ${IJKPLAYER_SRC_DIR}/ijkavformat/ijkurlhook.c
    ${IJKPLAYER_SRC_DIR}/ijkavformat/ijklongurl.c
    ${IJKPLAYER_SRC_DIR}/ijkavformat/ijksegment.c
    ${IJKPLAYER_SRC_DIR}/ijkavformat/ijklivehook.c
    ${IJKPLAYER_SRC_DIR}/ijkavformat/ijkmediadatasource.c
    ${IJKPLAYER_SRC_DIR}/ijkavformat/ijkcacheutil.c
    ${LIBYUV_SRC_DIR}/source/compare.cc           
    ${LIBYUV_SRC_DIR}/source/compare_common.cc
    ${LIBYUV_SRC_DIR}/source/compare_gcc.cc
    ${LIBYUV_SRC_DIR}/source/compare_mmi.cc
    ${LIBYUV_SRC_DIR}/source/compare_msa.cc
    ${LIBYUV_SRC_DIR}/source/compare_neon.cc      
    ${LIBYUV_SRC_DIR}/source/compare_neon64.cc

    ${LIBYUV_SRC_DIR}/source/convert.cc           
    ${LIBYUV_SRC_DIR}/source/convert_argb.cc      
    ${LIBYUV_SRC_DIR}/source/convert_from.cc      
    ${LIBYUV_SRC_DIR}/source/convert_from_argb.cc 
    ${LIBYUV_SRC_DIR}/source/convert_to_argb.cc   
    ${LIBYUV_SRC_DIR}/source/convert_to_i420.cc
    ${LIBYUV_SRC_DIR}/source/cpu_id.cc            
    ${LIBYUV_SRC_DIR}/source/planar_functions.cc  
    ${LIBYUV_SRC_DIR}/source/rotate.cc
    ${LIBYUV_SRC_DIR}/source/rotate_any.cc
    ${LIBYUV_SRC_DIR}/source/rotate_argb.cc
    ${LIBYUV_SRC_DIR}/source/rotate_common.cc
    ${LIBYUV_SRC_DIR}/source/rotate_gcc.cc
    ${LIBYUV_SRC_DIR}/source/rotate_mmi.cc
    ${LIBYUV_SRC_DIR}/source/rotate_msa.cc
    ${LIBYUV_SRC_DIR}/source/rotate_neon.cc
    ${LIBYUV_SRC_DIR}/source/rotate_neon64.cc

    ${LIBYUV_SRC_DIR}/source/row_any.cc           
    ${LIBYUV_SRC_DIR}/source/row_common.cc
    ${LIBYUV_SRC_DIR}/source/row_gcc.cc
    ${LIBYUV_SRC_DIR}/source/row_mmi.cc
    ${LIBYUV_SRC_DIR}/source/row_msa.cc
    ${LIBYUV_SRC_DIR}/source/row_neon.cc          
    ${LIBYUV_SRC_DIR}/source/row_neon64.cc

    ${LIBYUV_SRC_DIR}/source/scale.cc             
    ${LIBYUV_SRC_DIR}/source/scale_any.cc         
    ${LIBYUV_SRC_DIR}/source/scale_argb.cc        
    ${LIBYUV_SRC_DIR}/source/scale_common.cc
    ${LIBYUV_SRC_DIR}/source/scale_gcc.cc
    ${LIBYUV_SRC_DIR}/source/scale_mmi.cc
    ${LIBYUV_SRC_DIR}/source/scale_msa.cc
    ${LIBYUV_SRC_DIR}/source/scale_neon.cc        
    ${LIBYUV_SRC_DIR}/source/scale_neon64.cc      
    ${LIBYUV_SRC_DIR}/source/video_common.cc

    ${SOUNDTOUCH_SRC_DIR}/source/SoundTouch/AAFilter.cpp
    ${SOUNDTOUCH_SRC_DIR}/source/SoundTouch/FIFOSampleBuffer.cpp
    ${SOUNDTOUCH_SRC_DIR}/source/SoundTouch/FIRFilter.cpp
    ${SOUNDTOUCH_SRC_DIR}/source/SoundTouch/cpu_detect_x86.cpp
    ${SOUNDTOUCH_SRC_DIR}/source/SoundTouch/sse_optimized.cpp
    ${SOUNDTOUCH_SRC_DIR}/source/SoundTouch/RateTransposer.cpp
    ${SOUNDTOUCH_SRC_DIR}/source/SoundTouch/InterpolateCubic.cpp
    ${SOUNDTOUCH_SRC_DIR}/source/SoundTouch/InterpolateLinear.cpp
    ${SOUNDTOUCH_SRC_DIR}/source/SoundTouch/InterpolateShannon.cpp
    ${SOUNDTOUCH_SRC_DIR}/source/SoundTouch/TDStretch.cpp
    ${SOUNDTOUCH_SRC_DIR}/source/SoundTouch/BPMDetect.cpp
    ${SOUNDTOUCH_SRC_DIR}/source/SoundTouch/PeakFinder.cpp
    ${SOUNDTOUCH_SRC_DIR}/source/SoundTouch/SoundTouch.cpp
    ${SOUNDTOUCH_SRC_DIR}/source/SoundTouch/mmx_optimized.cpp
    ${SOUNDTOUCH_SRC_DIR}/ijksoundtouch_wrap.cpp
)


        


if(ANDROID_ABI STREQUAL armeabi-v7a)
    set_source_files_properties(${LIBYUV_SRC_DIR}/source/compare_neon.cc PROPERTIES COMPILE_FLAGS -mfpu=neon)
    set_source_files_properties(${LIBYUV_SRC_DIR}/source/rotate_neon.cc PROPERTIES COMPILE_FLAGS -mfpu=neon)
    set_source_files_properties(${LIBYUV_SRC_DIR}/source/row_neon.cc PROPERTIES COMPILE_FLAGS -mfpu=neon)
    set_source_files_properties(${LIBYUV_SRC_DIR}/source/scale_neon.cc PROPERTIES COMPILE_FLAGS -mfpu=neon)
    add_definitions(-D__ARM_NEON__=1)
endif()

include_directories(${IJK_SRC_DIR}/ijkj4a)
        

include_directories(${IJK_SRC_DIR}/)
        
include_directories(libs/include)
        

include_directories(${IJKPLAYER_SRC_DIR}/)
include_directories(${PXAUTH_SRC_DIR}/)
        
include_directories(${IJKSDL_SRC_DIR}/)

include_directories(${IJKSDL_SRC_DIR}/ffmpeg)

include_directories(${IJKSDL_SRC_DIR}/android)

include_directories(${LIBYUV_SRC_DIR}/include)
        
include_directories(${SOUNDTOUCH_SRC_DIR})
        
include_directories(${SOUNDTOUCH_SRC_DIR}/include)

include_directories(${IJKPLAYER_SRC_DIR}/android/pipeline)

include_directories(${IJKPLAYER_SRC_DIR}/android)

include_directories(${PIXVIDEO_SRC_DIR})

include_directories(${PIXVIDEO_SRC_DIR}/android/sdl)

include_directories(${IJKPLAYER_SRC_DIR}/pipeline)

set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_C_VISIBILITY_PRESET hidden)

#减小release so体积
#set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -s")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s")
        

# ===================  Pix Video Core ==================
add_library(${LibNewVideoName} SHARED ${libVideoSource})

add_library(PixFFmpeg SHARED IMPORTED)
set_target_properties(PixFFmpeg PROPERTIES IMPORTED_LOCATION
                        ${CMAKE_SOURCE_DIR}/libs/${ANDROID_ABI}/libPixFFmpeg.so)

target_link_libraries(${LibNewVideoName} ${log-lib} PixFFmpeg OpenSLES mediandk android "-Wl,--exclude-libs,ALL")
target_link_options(${LibNewVideoName} PRIVATE "-Wl,-z,max-page-size=16384")
target_link_options(${LibNewVideoName} PRIVATE "-Wl,-z,common-page-size=16384")

# ===================  Pix Video ==================
add_library(pixvideo SHARED
        ${PIXVIDEO_SRC_DIR}/h5_player.c
        ${PIXVIDEO_SRC_DIR}/pandora_player.c
        ${PIXVIDEO_SRC_DIR}/player_utils.c
        ${PIXVIDEO_SRC_DIR}/cJSON.c
        ${PIXVIDEO_SRC_DIR}/authorization.c
        ${PIXVIDEO_SRC_DIR}/android/pipeline/ffpipeline_native_android.c
        ${PIXVIDEO_SRC_DIR}/android/pipeline/ffpipenode_native_mediacodec_vdec.c
        ${PIXVIDEO_SRC_DIR}/android/pipeline/ffpipenode_native_mediacodec_adec.c
        ${PIXVIDEO_SRC_DIR}/android/ijkplayer_android.c
        ${PIXVIDEO_SRC_DIR}/android/sdl/ijksdl_aout_android_aaudio.c
)

add_library(pxauth SHARED IMPORTED)
set_target_properties(pxauth PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/../../contrib/pxauth/lib/${ANDROID_ABI}/libpxauth.so)

add_library(pxavalidator STATIC IMPORTED)
set_target_properties(pxavalidator PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/../../contrib/pxauth/lib/${ANDROID_ABI}/libpxavalidator.a)

target_link_options(pixvideo PRIVATE "-Wl,-z,max-page-size=16384")
target_link_options(pixvideo PRIVATE "-Wl,-z,common-page-size=16384")
target_link_libraries(pixvideo ${log-lib} pixvideocore OpenSLES mediandk android pxauth pxavalidator)
