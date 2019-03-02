include_directories(${CMAKE_SOURCE_DIR}/../)
set(SRCS ../total_flow.cc
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
        ../algorithm/utilities/src/transform.cpp
        ../algorithm/utilities/src/utils.cc
        judger.cc
        smoke_judger.cc
        eye_mouth_plus.cc
        head_pose_states.cc
        config_tracker.cc
        )

if(USE_SSD)
    list(APPEND SRCS ../algorithm/object_detection/ssd/src/object_detection.cc)
else()
    list(APPEND SRCS ../algorithm/object_detection/rcnn/src/object_detection.cc)
endif()

add_library(dsm_strategy STATIC ${SRCS})

