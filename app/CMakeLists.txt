
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



include_directories(src/main/cpp/include)
include_directories(src/main/cpp/srcs)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds it for you.
# Gradle automatically packages shared libraries with your APK.

add_library( # Sets the name of the library.
             native-lib
             # Sets the library as a shared library.
             SHARED
             # Provides a relative path to your source file(s).
             # Associated headers in the same location as their source
             # file are automatically included.
            src/main/cpp/jni_part.cpp
        src/main/cpp/srcs/object_detection.cc
        src/main/cpp/srcs/align_method.cpp
        src/main/cpp/srcs/faceAlign.cpp
        src/main/cpp/srcs/head_pose_estimator.cc
        src/main/cpp/srcs/shape_predict_untouch.cc
        src/main/cpp/srcs/transform.cpp
        src/main/cpp/srcs/utils.cc
        src/main/cpp/faceid.cc
        src/main/cpp/PictureProcessor.cpp
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

#                       libdlib
                       lib_ncnn
                       #libutils
                       lib_opencv
#                       libmsg
                        ${log-lib}
                       ${android-lib}
                       )
