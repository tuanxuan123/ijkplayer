cmake_minimum_required(VERSION 3.4.1)


# build native_app_glue as a static lib
set(APP_GLUE_DIR ${ANDROID_NDK}/sources/android/native_app_glue)
include_directories(${APP_GLUE_DIR})
add_library( app-glue STATIC ${APP_GLUE_DIR}/android_native_app_glue.c)


# build vulkan app

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/outputs/${ANDROID_ABI})

set(SRC_DIR src/main/jni)
set(WRAPPER_DIR src/main/jni/vulkan_wrapper)

add_library(videodemo SHARED
            ${SRC_DIR}/VulkanMain.cpp
            ${SRC_DIR}/AndroidMain.cpp
            ${WRAPPER_DIR}/vulkan_wrapper.cpp)

target_include_directories(videodemo PRIVATE
                            ${SRC_DIR})

set(IJK_SRC_DIR ${CMAKE_SOURCE_DIR}/../../../ijkmedia)
set(IJKSDL_SRC_DIR ${CMAKE_SOURCE_DIR}/../../../ijkmedia/ijksdl)
set(IJKPLAYER_SRC_DIR ${CMAKE_SOURCE_DIR}/../../../ijkmedia/ijkplayer)
set(IJKJ4A_SRC_DIR ${CMAKE_SOURCE_DIR}/../../../ijkmedia/ijkj4a)

find_library(log-lib log)


add_library(videoplayer SHARED
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
        ${IJKPLAYER_SRC_DIR}/h5_player.c
        ${IJKPLAYER_SRC_DIR}/pipeline/ffpipeline_ffplay.c
        ${IJKPLAYER_SRC_DIR}/pipeline/ffpipenode_ffplay_vdec.c
        ${IJKPLAYER_SRC_DIR}/android/ffmpeg_api_jni.c
        ${IJKPLAYER_SRC_DIR}/android/ijkplayer_android.c
        ${IJKPLAYER_SRC_DIR}/android/ijkplayer_jni.c
        ${IJKPLAYER_SRC_DIR}/android/pipeline/ffpipeline_android.c
        #${IJKPLAYER_SRC_DIR}/android/pipeline/ffpipenode_android_mediacodec_vdec.c


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
        )



target_include_directories(videoplayer PRIVATE
        ${IJK_SRC_DIR}/ijkj4a)

target_include_directories(videoplayer PRIVATE
        ${IJK_SRC_DIR}/)
target_include_directories(videoplayer PRIVATE
        libs/include)

target_include_directories(videoplayer PRIVATE
        ${IJKPLAYER_SRC_DIR}/)


include_directories(${WRAPPER_DIR})


add_library(ijkffmpeg SHARED IMPORTED)

set_target_properties(ijkffmpeg PROPERTIES IMPORTED_LOCATION
                        ${CMAKE_SOURCE_DIR}/libs/${ANDROID_ABI}/libijkffmpeg.so)





set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Werror \
                     -DVK_USE_PLATFORM_ANDROID_KHR")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")

#减小relaese so体积
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -s")

target_link_libraries(videoplayer ${log-lib} ijkffmpeg OpenSLES)

target_link_libraries(videodemo app-glue ${log-lib} android ijkffmpeg videoplayer)
