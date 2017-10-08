cmake_minimum_required(VERSION 2.8.11)
project(Saccade)

set(SACCADE_VERSION "0.1beta1")

SET (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/Modules)
message(STATUS ${CMAKE_MODULE_PATH})

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)
SET(CMAKE_AUTOUIC ON) 
SET(CMAKE_AUTORCC ON) 

find_package(Qt5Widgets)
find_package(OpenGL REQUIRED)
find_package(FreeImage REQUIRED)
find_package(Qt5OpenGL REQUIRED)

find_package(GFlags)
find_package(Glog)

find_package(OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
else()
    message("NO OMP")
endif()


set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -std=c++11")

set(SACCADE_SOURCES
    main.cpp
    GUI/marker.cpp
    GUI/layer.cpp
    GUI/slides.cpp
    GUI/canvas.cpp
    GUI/histogram.cpp
    GUI/about_window.cpp
    GUI/image_window.cpp
    GUI/window.cpp
    Utils/Ops/gamma_op.cpp
    Utils/Ops/histogram_op.cpp
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

include(GenerateVersionDefinitions)

add_subdirectory(GUI)

add_executable(saccade main.cpp ${SACCADE_SOURCES})
target_link_libraries(saccade ${SACCADE_LIBRARIES} ) 
