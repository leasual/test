include_directories(${CMAKE_SOURCE_DIR}/../)
include_directories(${CMAKE_CURRENT_LIST_DIR})
#include_directories(/home/chris/Downloads/OpenCV-android-sdk/sdk/native/jni/include/opencv)
#include_directories(/home/chris/Downloads/OpenCV-android-sdk/sdk/native/jni/include/opencv2)

#add_library( lib_opencv3 SHARED IMPORTED )
#set_target_properties(lib_opencv3 PROPERTIES IMPORTED_LOCATION /home/chris/nfs_sw/UNTOUCH_ROOT/external/opencv-3.4.3/aarch64-linux-android-4.9/sdk/native/libs/arm64-v8a/libopencv_java3.so)
if(ANDROID)
#    include_directories(/home/chris/nfs_sw/UNTOUCH_ROOT/external/opencv-3.4.3/aarch64-linux-android-4.9/sdk/native/jni/include)

#    include_directories($ENV{UNTOUCH_ROOT_LOCAL}/external/dlib-19.16/${ARCH}/include)
    include_directories(/home/chris/Downloads/dlib-19.16/include)
    add_library(dlib2 STATIC IMPORTED )
    set_target_properties(dlib2 PROPERTIES IMPORTED_LOCATION /home/chris/nfs_sw/UNTOUCH_ROOT/external/dlib-19.16/aarch64-linux-android-4.9/lib/libdlib.a)
#    set_target_properties(dlib2 PROPERTIES IMPORTED_LOCATION /home/chris/Downloads/dlib-19.16/libdlib.so)

else()
    include_directories(/home/chris/nfs_sw/UNTOUCH_ROOT/external/opencv-3.4.3/x86_64-linux-gnu/include)

    include_directories(/home/chris/nfs_sw/UNTOUCH_ROOT/external/dlib-19.16/x86_64-linux-gnu/include)
    add_library(dlib2 STATIC IMPORTED )
    set_target_properties(dlib2 PROPERTIES IMPORTED_LOCATION /home/chris/nfs_sw/UNTOUCH_ROOT/externals/dlib-19.16/x86_64-linux-gnu/lib/libdlib.a)
endif()

add_library(dsm_strategy SHARED
        judgement_strategy/alignment_detection.cc
        judgement_strategy/call_judger.cc
        judgement_strategy/distraction_judger.cc
        judgement_strategy/yawn_judger.cc
        judgement_strategy/fatigue_judger.cc
        judgement_strategy/real_time_detect.cc
        judgement_strategy/smoke_judger.cc
        eyemouthstatus.cc
        config_tracker.cc
        ../total_flow.cc
        ../algorithm/align/src/align_method.cpp
        ../algorithm/align/src/faceAlign.cpp
        ../algorithm/align/src/shape_predict_untouch.cc
        ../algorithm/face_detection/src/detector.cc
        ../algorithm/face_detection/src/onet.cc
        ../algorithm/face_detection/src/pnet.cc
        ../algorithm/face_detection/src/rnet.cc
        ../algorithm/faceid/src/faceAttribute.cpp
        ../algorithm/faceid/src/faceID.cpp
        ../algorithm/head_pose/src/head_pose_estimator.cc
        ../algorithm/object_detection/src/objDetection.cpp
        ../algorithm/utilities/src/transform.cpp
        ../algorithm/utilities/src/utils.cc
        )

target_link_libraries(
        dsm_strategy
        utils
        dlib2
#        lib_opencv3
)