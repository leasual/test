option(USE_ALIGN            " " ON)
option(USE_FACEID           " " ON)
option(USE_FACE_DETECTION   " " ON)
option(USE_GAZE_TRACKING    " " OFF)
option(USE_HEAD_POSE        " " ON)
option(USE_OBJ_DETECTION    " " ON)


if(USE_ALIGN)
    include(${CMAKE_CURRENT_LIST_DIR}/align/CMakeLists.txt)
endif()

if(USE_FACEID)
    include(${CMAKE_CURRENT_LIST_DIR}/faceid/CMakeLists.txt)
endif()

if(USE_FACE_DETECTION)
    include(${CMAKE_CURRENT_LIST_DIR}/face_detection/CMakeLists.txt)
endif()

if(USE_GAZE_TRACKING)
    include(${CMAKE_CURRENT_LIST_DIR}/gaze_tracking/CMakeLists.txt)
endif()

if(USE_HEAD_POSE)
    include(${CMAKE_CURRENT_LIST_DIR}/head_pose/CMakeLists.txt)
endif()

if(USE_OBJ_DETECTION)
    include(${CMAKE_CURRENT_LIST_DIR}/object_detection/CMakeLists.txt)
endif()

include(${CMAKE_CURRENT_LIST_DIR}/utilities/CMakeLists.txt)

if(LIBRARIES)
    list(APPEND LIBRARIES ${ALGORITHM})
endif()
