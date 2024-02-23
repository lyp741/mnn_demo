cmake_minimum_required(VERSION 3.4...3.18)
project(test VERSION 0.1.0 LANGUAGES C CXX)


set(CMAKE_CXX_STANDARD 14)
# set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_OSX_ARCHITECTURES "arm64")
add_subdirectory(MNN)
include_directories(MNN/include)
# link_directories(build/MNN)
# find_package(MNN REQUIRED)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

include(CTest)
enable_testing()

add_executable(main main.cpp)


target_link_libraries(main MNN)
# target_link_libraries(env_cpp PRIVATE ${Boost_LIBRARIES})
# target_link_libraries(env_cpp PRIVATE "C:/Program Files/boost/stage/lib")