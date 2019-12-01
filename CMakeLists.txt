cmake_minimum_required(VERSION 3.10)
project(bstreamer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS " -O3 -g")

find_package(BZip2 REQUIRED)
find_package (Threads)

if(BZIP2_FOUND)
    message(STATUS "Using BZIP2 ${BZIP2_VERSION_STRING}")
#    add_definitions(-DHAVE_BZIP2)
endif()

#include_directories(${BZIP2_INCLUDE_DIRS})
set(CMAKE_REQUIRED_LIBRARIES BZip2::BZip2)

message("" ${BZIP2_INCLUDE_DIRS})
message("" ${BZIP2_INCLUDE_DIR})
message("" ${BZIP2_LIBRARIES})
message("" BZip2::BZip2)

set(SRCS
    RevisionStreamer.cpp
)

set(HDRS
    RevisionStreamer.h
    lib/atomicops.h
    lib/readerwriterqueue.h
    BinaryStreamer.h
)


add_executable(${PROJECT_NAME} zipMain.cpp ${SRCS} ${HDRS})
add_executable("hash${PROJECT_NAME}" streamingImpl.cpp ${SCRS} ${HDRS})

#target_link_libraries (${PROJECT_NAME} PRIVATE BZip2::BZip2)
#target_include_directories(bstreamer PRIVATE ${BZIP2_INCLUDE_DIR})
#include_directories(${BZIP2_INCLUDE_DIR})
#target_link_libraries(${PROJECT_NAME} ${BZIP2_LIBRARIES})
#target_link_libraries(${PROJECT_NAME} ${BZIP2_LIBRARIES})

target_link_libraries(${PROJECT_NAME} PRIVATE BZip2::BZip2 ${CMAKE_THREAD_LIBS_INIT})