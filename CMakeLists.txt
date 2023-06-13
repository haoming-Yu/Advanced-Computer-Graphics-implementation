cmake_minimum_required(VERSION 3.13)
project(HQRC)
set(GCO_DIR ${PROJECT_SOURCE_DIR}/gco)
set(QR_DIR ${PROJECT_SOURCE_DIR}/qrcode)
add_compile_options(-g )
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
file(GLOB_RECURSE CXX_SOURCES "src/*.cpp" "src/*/*.cpp" "src/*/*/*.cpp")
<<<<<<< HEAD
set(SOURCES ${CXX_SOURCES} ${GCO_DIR} ${QR_DIR})
=======
# 排除 src/venv 目录中的文件
foreach(source_file IN LISTS CXX_SOURCES)
    if(source_file MATCHES "/venv/")
        list(REMOVE_ITEM CXX_SOURCES ${source_file})
    endif()
endforeach()
set(SOURCES ${CXX_SOURCES} ${GCO_DIR})
>>>>>>> d818a9c28cbcdbf21f2595ddd997c2a1a37cabda
include_directories(${GCO_DIR}/include)
include_directories(${QR_DIR})
include_directories(src/include)
add_subdirectory(${GCO_DIR})
add_subdirectory(${QR_DIR})

add_executable(HQRC ${SOURCES})
link_directories(${OpenCV_LIBRARY_DIRS})
# message(STATUS "path = ${OpenCV_LIBS}")
# target_link_directories(HQRC PUBLIC ${OpenCV_LIBS})
target_link_libraries(HQRC ${OpenCV_LIBS})
target_link_libraries(HQRC GCO)
target_link_libraries(HQRC qrencode)

# link_directories(${OpenCV_LIBRARY_DIRS})