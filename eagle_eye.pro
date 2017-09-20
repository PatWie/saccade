HEADERS       = GUI/window.h\
                GUI/marker.h\
                GUI/layer.h\
                GUI/slides.h\
                GUI/canvas.h\
                GUI/about_window.h\
                GUI/image_window.h\
                Utils/gl_manager.h\
                Utils/mipmap_tile.h\
                Utils/mipmap_level.h\
                Utils/mipmap.h\
                Utils/image_data.h
SOURCES       = main.cpp \
                GUI/marker.cpp\
                GUI/layer.cpp\
                GUI/slides.cpp\
                GUI/canvas.cpp\
                GUI/about_window.cpp\
                GUI/image_window.cpp\
                GUI/window.cpp\
                Utils/gl_manager.cpp\
                Utils/mipmap_tile.cpp\
                Utils/mipmap_level.cpp\
                Utils/mipmap.cpp\
                Utils/image_data.cpp

RESOURCES     = textures.qrc

QT           += widgets gui opengl core
CONFIG += c++11

LIBS += -lfreeimage

# install
# target.path = $$[QT_INSTALL_EXAMPLES]/opengl/textures
# INSTALLS += target

FORMS += \
    GUI/about_window.ui