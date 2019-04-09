if(USE_DLIB)
    set(dlib_DIR $ENV{UNTOUCH_ROOT}/externals/dlib-19.16/${ARCH}/lib/cmake/dlib)
    find_package(dlib)
    include_directories(${dlib_INCLUDE_DIRS})
    list(APPEND ALGORITHM dlib::dlib)
endif()

include_directories(${CMAKE_CURRENT_LIST_DIR}/include)
