
file(GLOB SRC "file/*.cc")
list(APPEND SOURCES ${SRC})


add_library(utils STATIC
        ${SOURCES})