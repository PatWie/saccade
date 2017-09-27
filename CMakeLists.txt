cmake_minimum_required(VERSION 2.8.11)
project(EagleEye)

set(EAGLEEYE_VERSION "0.1alpha1")

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
find_package(Glew REQUIRED)
find_package(FreeImage REQUIRED)
find_package(Qt5OpenGL REQUIRED)

find_package(GFlags)
find_package(Glog)


set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCES
    main.cpp
    GUI/marker.cpp
    GUI/layer.cpp
    GUI/slides.cpp
    GUI/canvas.cpp
    GUI/about_window.cpp
    GUI/image_window.cpp
    GUI/window.cpp
    Utils/Ops/gamma_op.cpp
    Utils/gl_manager.cpp
    Utils/mipmap_tile.cpp
    Utils/mipmap_level.cpp
    Utils/mipmap.cpp
    Utils/image_data.cpp
    Utils/version.cpp
)

set(EAGLEEYE_LIBRARIES
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

add_executable(eagle_eye main.cpp ${SOURCES})
target_link_libraries(eagle_eye ${EAGLEEYE_LIBRARIES} ) 
