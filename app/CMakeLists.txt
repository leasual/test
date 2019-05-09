
# Sets the minimum version of CMake required to build the native
# library. You should either keep the default value or only pass a
# value of 3.4.0 or lower.

cmake_minimum_required(VERSION 3.4.1)

set(CMAKE_VERBOSE_MAKEFILE on)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


#include_directories(/media/chris/软件/OpenCV-android-sdk/sdk/native/jni/include)
include_directories(D:/OpenCV-android-sdk/sdk/native/jni/include)
add_library( lib_opencv SHARED IMPORTED )
set_target_properties(lib_opencv PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libopencv_java3.so)

include_directories(src/main/cpp/include/ncnn)

add_library( lib_ncnn STATIC IMPORTED )
set_target_properties(lib_ncnn PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libncnn.a)

include_directories(src/main/cpp/algorithm/align/include)
include_directories(src/main/cpp/algorithm/face_detection/include)
include_directories(src/main/cpp/algorithm/faceid/include)
include_directories(src/main/cpp/algorithm/gaze_tracking/include)
include_directories(src/main/cpp/algorithm/head_pose/include)
include_directories(src/main/cpp/algorithm/landmark/fast-rcnn/include)
include_directories(src/main/cpp/algorithm/object_detection/rcnn/include)
include_directories(src/main/cpp/algorithm/utilities/include)
include_directories(src/main/cpp/dsm_strategy)
include_directories(src/main/cpp/dsm_strategy/utils/file)
include_directories(src/main/cpp/include)
include_directories(src/main/cpp/message/include)
include_directories(src/main/cpp/message/api)
include_directories(src/main/cpp/message/base)
include_directories(src/main/cpp/message)


#add_library( libutils STATIC IMPORTED )
#set_target_properties(libutils PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libutils.a)
add_library( libdlib STATIC IMPORTED )
set_target_properties(libdlib PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libdlib.a)
#add_library( libdsm_strategy STATIC IMPORTED )
#set_target_properties(libdsm_strategy PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libdsm_strategy.a)
#add_library( lib_dsm STATIC IMPORTED )
#set_target_properties(lib_dsm PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libdsm.a)
add_library( libmsg STATIC IMPORTED )
set_target_properties(libmsg PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libhpsocket.so)



# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds it for you.
# Gradle automatically packages shared libraries with your APK.

file(GLOB native_srcs "src/main/cpp/*.cpp" "src/main/cpp/*.cc")
file(GLOB native_srcs2 "src/main/cpp/algorithm/align/src/*.cpp")
file(GLOB native_srcs3 "src/main/cpp/algorithm/face_detection/src/*.cc")
file(GLOB native_srcs4 "src/main/cpp/algorithm/faceid/src/*.cpp")
#file(GLOB native_srcs5 "src/main/cpp/algorithm/gaze_tracking/src/*.cpp")
file(GLOB native_srcs6 "src/main/cpp/algorithm/head_pose/src/*.cc")
file(GLOB native_srcs7 "src/main/cpp/algorithm/landmark/fast-rcnn/src/*.cc")
file(GLOB native_srcs8 "src/main/cpp/algorithm/object_detection/rcnn/src/*.cc")
file(GLOB native_srcs9 "src/main/cpp/algorithm/utilities/src/*.cc" "src/main/cpp/algorithm/utilities/src/*.cpp")

file(GLOB native_srcs10 "src/main/cpp/dsm_strategy/*.cpp" "src/main/cpp/dsm_strategy/*.cc")
file(GLOB native_srcs11 "src/main/cpp/dsm_strategy/utils/file/*.cpp" "src/main/cpp/dsm_strategy/utils/file/*.cc")
#file(GLOB native_srcs12 "src/main/cpp/dsm_strategy/utils/time/*.cpp" "src/main/cpp/dsm_strategy/utils/time/*.cc")
file(GLOB native_srcs13 "src/main/cpp/message/*.cpp")
file(GLOB native_srcs14 "src/main/cpp/message/api/*.cpp")
file(GLOB native_srcs15 "src/main/cpp/message/base/*.cpp")


add_library( # Sets the name of the library.
        native-lib
        # Sets the library as a shared library.
        SHARED
        ${native_srcs}
        ${native_srcs2}
        ${native_srcs3}
        ${native_srcs4}
#        ${native_srcs5}
        ${native_srcs6}
        ${native_srcs7}
        ${native_srcs8}
        ${native_srcs9}
        ${native_srcs10}
        ${native_srcs11}
#        ${native_srcs12}
        ${native_srcs13}
        ${native_srcs14}
        ${native_srcs15}
        )



# Searches for a specified prebuilt library and stores the path as a
# variable. Because system libraries are included in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
              log-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log )
#find_library( camera-lib camera2ndk )
#find_library( media-lib mediandk )
find_library( android-lib android )

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in the
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
                       native-lib

                       #lib_dsm
                       #libdsm_strategy

                       libdlib
                       lib_ncnn
                       #libutils
                       lib_opencv
                       libmsg
                        ${log-lib}
                       ${android-lib}
                       )
