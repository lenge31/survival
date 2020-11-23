PROJECT(client)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET(CMAKE_BUILD_TYPE "Debug")  
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb")  
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")  

INCLUDE_DIRECTORIES(. protocol policy)

ADD_SUBDIRECTORY(protocol)
ADD_SUBDIRECTORY(policy)

AUX_SOURCE_DIRECTORY(. DIR_CLIENT_SRCS)

ADD_EXECUTABLE(client ${DIR_CLIENT_SRCS})

TARGET_LINK_LIBRARIES(client protocol policy)

# For GoogleTest ------
option(AUTOTEST, "Whether enable autotest" OFF)
if (AUTOTEST)
ADD_EXECUTABLE(out.test autotest/test.cpp client.cpp)
TARGET_LINK_LIBRARIES(out.test protocol policy
		${CMAKE_CURRENT_SOURCE_DIR}/autotest/gtest/libgtest.a pthread)
INCLUDE_DIRECTORIES(out.test protocol policy autotest/gtest)
# SET_TARGET_PROPERTIES(out.test PROPERTIES COMPILE_FLAGS "-D_GLIBCXX_USE_CXX11_ABI=1")
endif()
