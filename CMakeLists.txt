cmake_minimum_required(VERSION 3.4...3.18)
project(test VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_OSX_ARCHITECTURES "arm64")

# add_subdirectory(MNN)
# include_directories(MNN/include)
# link_directories(build/MNN)
# find_package(MNN REQUIRED)
find_package(Boost 1.84.0 REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

add_subdirectory(protobuf)

add_library(proto proto/KoalaAI_G2C.pb.h proto/KoalaAI_G2C.pb.cc)



add_executable(main main.cpp)


target_link_libraries(main proto  ${Boost_LIBRARIES} libprotobuf)
# target_link_libraries(env_cpp PRIVATE ${Boost_LIBRARIES})
# target_link_libraries(env_cpp PRIVATE "C:/Program Files/boost/stage/lib")
