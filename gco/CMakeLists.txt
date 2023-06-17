cmake_minimum_required(VERSION 3.13)

project(GCO)

# set(CMAKE_CXX_STANDARD 11)
set(INC ${PROJECT_SOURCE_DIR}/include)
file(GLOB SOURCES "src/*.cpp")
# set(SOURCES ${PROJECT_SOURCE_DIR}/src)
include_directories(${INC})
# include_directories(${SOURCES})
# add_executable(${PROJECT_NAME} ${SOURCES})
add_library(${PROJECT_NAME} SHARED ${SOURCES})
