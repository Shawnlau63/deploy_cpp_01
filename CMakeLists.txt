cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project(deploy_cpp_01)

#set(CUDA_TOOLKIT_ROOT_DIR "C:\\Program Files\\NVIDIA GPU Computing Toolkit\\CUDA\\v10.1")
set(CMAKE_PREFIX_PATH C:/libtorch)
#set(CMAKE_PREFIX_PATH C:\\libtorch\\share\\cmake\\Torch)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS}")
#set(OpenCV_DIR C:\\opencv\\build\\x64\\vc15\\lib)
set(OpenCV_DIR C:\\opencv\\build)

find_package(Torch REQUIRED)
find_package(OpenCV REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS}")


add_executable(deploy_cpp_01 main_b1.cpp utils.hpp detector.hpp service.hpp)
target_link_libraries(deploy_cpp_01 "${TORCH_LIBRARIES}")
target_link_libraries(deploy_cpp_01 ${OpenCV_LIBS})
set_property(TARGET deploy_cpp_01 PROPERTY CXX_STANDARD 11)

if (MSVC)
    file(GLOB TORCH_DLLS "${TORCH_INSTALL_PREFIX}/lib/*.dll")
    add_custom_command(TARGET deploy_cpp_01
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${TORCH_DLLS}
            $<TARGET_FILE_DIR:deploy_cpp_01>)
endif (MSVC)