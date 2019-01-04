include_directories(${CMAKE_CURRENT_LIST_DIR}/file)
if(ANDROID)
    include_directories(/home/chris/Downloads/OpenCV-android-sdk/sdk/native/jni/include/opencv)
    include_directories(/home/chris/Downloads/OpenCV-android-sdk/sdk/native/jni/include/opencv2)
    add_library( lib_opencv2 SHARED IMPORTED )
#    set_target_properties(lib_opencv2 PROPERTIES IMPORTED_LOCATION /home/chris/nfs_sw/UNTOUCH_ROOT/external/opencv-3.4.3/aarch64-linux-android-4.9/sdk/native/libs/armeabi-v7a/libopencv_java3.so)

    set_target_properties(lib_opencv2 PROPERTIES IMPORTED_LOCATION /home/chris/nfs_sw/UNTOUCH_ROOT/external/opencv-3.4.3/aarch64-linux-android-4.9/sdk/native/libs/arm64-v8a/libopencv_java3.so)
else()
    include_directories(/home/chris/Downloads/OpenCV-android-sdk/sdk/native/jni/include)
    add_library( lib_opencv2 SHARED IMPORTED )
    set_target_properties(lib_opencv2 PROPERTIES IMPORTED_LOCATION /home/chris/nfs_sw/UNTOUCH_ROOT/external/opencv-3.4.3/x86_64-linux-gnu/lib/libopencv.so)

endif()
file(GLOB SRC "file/*.cc")
list(APPEND SOURCES ${SRC})


add_library(utils SHARED
        ${SOURCES})

target_link_libraries(
        utils
        lib_opencv2)
