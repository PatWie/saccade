cmake_minimum_required(VERSION 2.8.11)
project(Saccade)

set(SACCADE_VERSION "0.1beta2")

option(OPENMP_ENABLED "Whether to enable OpenMP" ON)
option(CUDA_ENABLED "Whether to enable CUDA, if available" ON)

set (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/Modules)
message(STATUS ${CMAKE_MODULE_PATH})

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON) 
set(CMAKE_AUTORCC ON) 

find_package(Qt5Widgets)
find_package(OpenGL REQUIRED)
find_package(FreeImage REQUIRED)
find_package(Qt5OpenGL REQUIRED)

find_package(GFlags)
find_package(Glog)
if(OPENMP_ENABLED)
    find_package(OpenMP)
    if (OPENMP_FOUND)
        set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
        set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    else()
        message("NO OMP")
    endif()
endif()

set(CUDA_MIN_VERSION "7.0")
if(CUDA_ENABLED)
    find_package(CUDA ${CUDA_MIN_VERSION} QUIET)
endif()

set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -std=c++11")
if (CMAKE_COMPILER_IS_GNUCC)
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall -Wignored-qualifiers -Wunused-parameter")
endif(CMAKE_COMPILER_IS_GNUCC)

if(CUDA_FOUND)
    if(CUDA_ENABLED)
        add_definitions("-DCUDA_ENABLED")

        include(${CMAKE_SOURCE_DIR}/cmake/Modules/SelectCudaComputeArch.cmake)

        CUDA_SELECT_NVCC_ARCH_FLAGS(CUDA_ARCH_FLAGS Auto)
        list(APPEND CUDA_NVCC_FLAGS "-Wno-deprecated-gpu-targets")
        list(APPEND CUDA_NVCC_FLAGS ${CUDA_ARCH_FLAGS})

        message(STATUS "Enabling CUDA support (version: ${CUDA_VERSION_STRING},"
                       " archs: ${CUDA_ARCH_FLAGS_readable})")
    else()
        set(CUDA_FOUND FALSE)
        message(STATUS "Disabling CUDA support")
    endif()
else()
    set(CUDA_ENABLED FALSE)
    if(CUDA_VERSION_STRING)
        message(STATUS "Disabling CUDA support (found version "
                "${CUDA_VERSION_STRING} but >= ${CUDA_MIN_VERSION} required)")
    else()
        message(STATUS "Disabling CUDA support")
    endif()
endif()

set(SACCADE_SOURCES
    main.cpp
    GUI/marker.cpp
    GUI/layer.cpp
    GUI/slides.cpp
    GUI/canvas.cpp
    GUI/clickable_label.cpp
    GUI/histogram.cpp
    GUI/about_window.cpp
    GUI/image_window.cpp
    GUI/window.cpp
    Utils/ascii_loader.cpp
    Utils/selection.cpp
    Utils/gl_manager.cpp
    Utils/mipmap_tile.cpp
    Utils/mipmap_level.cpp
    Utils/mipmap.cpp
    Utils/image_data.cpp
    Utils/histogram_data.cpp
    Utils/version.cpp
)

set(SACCADE_LIBRARIES
    Qt5::Widgets 
    gflags 
    glog
    ${GLOG_LIBRARIES}
    ${FREEIMAGE_LIBRARIES}
    ${OPENGL_gl_LIBRARY}
    ${OPENGL_glu_LIBRARY}
  )

if(CUDA_ENABLED)
    LIST(APPEND SACCADE_LIBRARIES cuda_op_histogram)
endif()


include(GenerateVersionDefinitions)
add_subdirectory(GUI)
if(CUDA_ENABLED)
    cuda_add_library(cuda_op_histogram Utils/Ops/histogram_op.cu Utils/Ops/gamma_op.cu)
else()
    LIST(APPEND SACCADE_SOURCES Utils/Ops/histogram_op.cpp)
    LIST(APPEND SACCADE_SOURCES Utils/Ops/gamma_op.cpp)
endif()
add_executable(saccade main.cpp ${SACCADE_SOURCES} )
target_link_libraries(saccade ${SACCADE_LIBRARIES} ) 
