cmake_minimum_required(VERSION 3.4.1)

# build native_app_glue as a static lib
set(APP_GLUE_DIR ${ANDROID_NDK}/sources/android/native_app_glue)
include_directories(${APP_GLUE_DIR})
add_library( app-glue STATIC ${APP_GLUE_DIR}/android_native_app_glue.c)


# build vulkan app


set(SRC_DIR src/main/jni)
set(WRAPPER_DIR src/main/jni/vulkan_wrapper)

add_library(vktuts SHARED
            ${SRC_DIR}/VulkanMain.cpp
            ${SRC_DIR}/AndroidMain.cpp
            ${WRAPPER_DIR}/vulkan_wrapper.cpp)

target_include_directories(vktuts PRIVATE
                            ${SRC_DIR})




add_library(ijkffmpeg SHARED IMPORTED)
add_library(ijksdl SHARED IMPORTED)
add_library(videoplayer SHARED IMPORTED)



set_target_properties(ijkffmpeg PROPERTIES IMPORTED_LOCATION
                        ${CMAKE_SOURCE_DIR}/libs/armeabi-v7a/libijkffmpeg.so)

set_target_properties(ijksdl PROPERTIES IMPORTED_LOCATION
                        ${CMAKE_SOURCE_DIR}/libs/${ANDROID_ABI}/libijksdl.so)

set_target_properties(videoplayer PROPERTIES IMPORTED_LOCATION
                        ${CMAKE_SOURCE_DIR}/libs/${ANDROID_ABI}/libvideoplayer.so)

include_directories(${WRAPPER_DIR})



set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Werror \
                     -DVK_USE_PLATFORM_ANDROID_KHR")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")
target_link_libraries(vktuts app-glue log android ijkffmpeg ijksdl videoplayer )
