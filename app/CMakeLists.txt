
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

#include_directories(src/main/cpp/include/dsm)
#include_directories(src/main/cpp/include/dsm/algorithm/align/include)
#include_directories(src/main/cpp/include/dsm/algorithm/face_detection/include)
#include_directories(src/main/cpp/include/dsm/algorithm/faceid/include)
#include_directories(src/main/cpp/include/dsm/algorithm/gaze_tracking/include)
#include_directories(src/main/cpp/include/dsm/algorithm/head_pose/include)
#include_directories(src/main/cpp/include/dsm/algorithm/object_detection/include)
#include_directories(src/main/cpp/include/dsm/algorithm/utilities/include)
#include_directories(src/main/cpp/include/dsm/dsm_strategy)
#include_directories(src/main/cpp/include/dsm/dsm_strategy/judgement_strategy)
#include_directories(src/main/cpp/include/dsm/dsm_strategy/utils/file)
#
include_directories(src/main/cpp)

#add_library( libutils STATIC IMPORTED )
##set_target_properties(libutils PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libutils.a)
#add_library( libdlib STATIC IMPORTED )
#set_target_properties(libdlib PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libdlib.a)
##add_library( libdsm_strategy STATIC IMPORTED )
##set_target_properties(libdsm_strategy PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libdsm_strategy.a)
##add_library( lib_dsm STATIC IMPORTED )
##set_target_properties(lib_dsm PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libdsm.a)
#add_library( libmsg STATIC IMPORTED )
#set_target_properties(libmsg PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libhpsocket.so)

#include_directories(src/main/cpp/message/include)
#include_directories(src/main/cpp/aes/include)


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
#        src/main/cpp/message/src/dsm_transmit_jtt.cpp
#        src/main/cpp/message/src/dsmapp_sms_custom_jtt.cpp
#        src/main/cpp/message/src/dsmapp_srv_jtt.cpp
#        src/main/cpp/message/src/dsmapp_srvinteraction_jtt.cpp
#        src/main/cpp/message/src/others.cpp
#        src/main/cpp/message/src/main.cpp
#        src/main/cpp/message/src/DSM_JTT_API.cpp

            src/main/cpp/native-lib.cpp
            src/main/cpp/jni_part.cpp


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
                       #libutils
                       lib_opencv
#                       libmsg
#                        libmsg
                        ${log-lib}
                       ${android-lib}
                       )
