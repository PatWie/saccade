import glob

headers = glob.glob('Utils/*.h') + glob.glob('GUI/*.h') + glob.glob('Utils/Ops/*.h')

print "HEADERS = ", headers[0], "\\"
for h in range(1, len(headers) - 1):
    print "          ", headers[h], "\\"
print "          ", headers[len(headers) - 1]


sources = glob.glob('Utils/*.cpp') + glob.glob('GUI/*.cpp') + glob.glob('Utils/Ops/*.cpp') + ["main.cpp"]

print "SOURCES = ", sources[0], "\\"
for h in range(1, len(sources) - 1):
    print "          ", sources[h], "\\"
print "          ", sources[len(sources) - 1]

print """RESOURCES     = textures.qrc

QT           += widgets gui opengl core
CONFIG += c++11
CONFIG += static

LIBS += -lgflags -lglog -lglog -lfreeimage -lpthread -fopenmp

FORMS += \
    GUI/about_window.ui
"""