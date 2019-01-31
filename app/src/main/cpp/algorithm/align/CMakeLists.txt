set(dlib_DIR $ENV{UNTOUCH_ROOT_LOCAL}/externals/dlib-19.16/${ARCH}/lib/cmake/dlib)
find_package(dlib)
include_directories(${dlib_INCLUDE_DIRS})

include_directories(${CMAKE_CURRENT_LIST_DIR}/include)
#link_directories(${CMAKE_CURRENT_LIST_DIR}/lib)
list(APPEND ALGORITHM dlib::dlib #[[align]])
