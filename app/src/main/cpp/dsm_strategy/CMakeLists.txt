include_directories(${CMAKE_SOURCE_DIR}/../)

set(SRCS ../total_flow.cc
        ../algorithm/align/src/align_method.cpp
        ../algorithm/align/src/faceAlign.cpp
        ../algorithm/faceid/src/faceAttribute.cpp
        ../algorithm/faceid/src/faceID.cpp
        ../algorithm/head_pose/src/head_pose_estimator.cc
        ../algorithm/utilities/src/transform.cpp
        ../algorithm/utilities/src/utils.cc
        judger.cc
        distract_judger.cc
        eye_mouth_plus.cc
        head_pose_states.cc
        config_tracker.cc
        )

if(USE_DLIB)
    list(APPEND SRCS ../algorithm/landmark/dlib/src/shape_predict.cc)
    list(APPEND SRCS ../algorithm/object_detection/rcnn/src/classify.cc)
    list(APPEND SRCS ../algorithm/object_detection/rcnn/src/object_detection.cc)
endif()

if(USE_NCNN)
        list(APPEND SRCS ../algorithm/landmark/fast-rcnn/src/shape_predict.cc)
        list(APPEND SRCS ../algorithm/object_detection/rcnn/src/classify.cc)
        list(APPEND SRCS ../algorithm/object_detection/rcnn/src/object_detection.cc)
endif()

if(USE_TVM)
    list(APPEND SRCS ../algorithm/landmark/tvm/src/shape_predict.cc)
    list(APPEND SRCS ../algorithm/object_detection/tvm/src/object_detection.cc)
    list(APPEND SRCS ../algorithm/object_detection/tvm/src/classify.cc)

    list(APPEND SRCS ../algorithm/tvm_runner/tvm_data.cc)
    list(APPEND SRCS ../algorithm/tvm_runner/tvm_runner.cc)
endif()

add_library(dsm_strategy STATIC ${SRCS})

