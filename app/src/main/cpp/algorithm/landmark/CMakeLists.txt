if(USE_DLIB)
    include_directories(${CMAKE_CURRENT_LIST_DIR}/dlib/include)
endif()

if(USE_NCNN)
    include_directories(${CMAKE_CURRENT_LIST_DIR}/fast-rcnn/include)
endif()

if(USE_TVM)
    include_directories(${CMAKE_CURRENT_LIST_DIR}/tvm/include)
endif()
