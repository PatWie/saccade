 # Saccade
[![Build Status](http://ci.patwie.com/api/badges/PatWie/saccade/status.svg)](http://ci.patwie.com/PatWie/saccade)
[![GitHub release](https://img.shields.io/badge/release-get_latest-blue.svg)](https://files.patwie.com/builds/saccade/)

A **saccade** (*/səˈkɑːd/ sə-KAHD*) is a quick and simultaneous movement of both eyes in the same direction.

<p align="center"> <img src="https://github.com/patwie-stuff/img/blob/master/saccade_intro.gif?raw=true"> </p>

This project is the missing scientific HDR image viewer for Linux inspired by [shiver/idisplay](https://sourceforge.net/p/shiver/idisplay). It is OpenGL-based and aims at providing an efficient image viewer with some sophisticated functions for comparing images. When dragging and zooming an image in one viewport, all other viewports synchronously zoom and shift in the same way, so you see exactly the same patch in all images.

Supports the following file formats:
    - image: *.png *.jpg *.jpeg *.bmp *.ppm *.tif *.CR2 *.JPG *.JPEG, *.JPE
    - optical-flow: *.flo

Hereby, each viewport consists of different image-layers. Some action might affect only the *active* layer across *all* viewports (global effects), e.g., zooming and dragging, e.g. setting the crop-region:

<p align="center"> <img src="https://github.com/patwie-stuff/img/blob/master/saccade_crop.gif?raw=true"> </p>

When manipulating the histogram, the changes can be applied to only the current *active* layer or *all* layers in the *current* viewport (holding the `SHIFT` key). Further, a marker can be set precisely on a specific pixel, and all other views share the marker at exacly the same location. It uses the [FreeImage](freeimage.sourceforge.net) library to load images from different types:

<p align="center"> <img src="https://github.com/patwie-stuff/img/blob/master/saccade_histogram.gif?raw=true"> </p>

## Features

- OpenGL accelerated viewer using mip-mapping data structure
- synchronize multiple viewports when dragging and zooming within one viewport
- drag'n drop for open images
- crop regions from image
- keyboard short-cuts for all actions
- set marker on a specific pixel
- filewatcher (images will be reloaded when they change)
- interactive histogram widget which effects the image
- supported file formats: png jpg bmp ppm tif CR2 and many more
- helpful commands to arrange multiple windows
- multi-threaded loading and writing

## Install from binary

The related binaries for the master branch are generated automatically and statically linked to Qt5.9.2 to reduce dependencies and filesize. The [pre-compile binaries](https://files.patwie.com/builds/saccade/) are available with *all dependencies* and are tested in a VM with plain Ubuntu 16.04 .

## Install from Source

### Requirements

The requirements (Ubuntu 16.04) are currently:

- Qt5.9
- CMake
- FreeImage library
- Google Log
- Google Flags

I plan to add CUDA for speeding up some functions.

### Get dependencies and compile

The [dockerfile](https://github.com/PatWie/saccade/blob/master/Dockerfile) lists all dependencies for building this project. Or follow the following guide:

Assuming you are on Ubuntu 16.04 getting all dependencies is done by

    # install dependencies
    sudo apt-get install libfreeimage3 libfreeimage-dev libgflags-dev libgoogle-glog-dev

The Ubuntu-packages contain an old version of Qt5. To get a recent Qt5 version (>=5.9) use the following commands (you might need to register for Qt).

    cd /tmp
    wget http://download.qt.io/official_releases/qt/5.9/5.9.1/qt-opensource-linux-x64-5.9.1.run
    chmod +x qt-opensource-linux-x64-5.9.1.run
    ./qt-opensource-linux-x64-5.9.1.run

And finally just get and build this repository by

    # clone this repo
    git clone https://github.com/PatWie/saccade.git
    cd saccade
    mkdir build
    cd build
    cmake -DCMAKE_BUILD_TYPE=Release ..
    make -j4

A copy and paste version to automate the GUI installer is described in the [Dockerfile](https://github.com/PatWie/saccade/blob/master/.deploy/images/dynamic/Dockerfile).

To add the saccade application to the unity-launcher, edit the `saccade.desktop` file by adjust the paths:

    edit saccade.desktop
    cp saccade.desktop $HOME/.local/share/applications/saccade.desktop

and you find the app icon in the Ubuntu search bar. When debugging the application, it might be helpful to start it with the flag `--logtostderr 1` and build it with `DCMAKE_BUILD_TYPE=Debug`.

## Keyboard Shortcuts

These are very likely to changed in the next versions.

**General shortcuts**

| Action                        | Shortcut                  |
| ------                        | ------                    |
| quit entire the app           | Ctrl + Q                  |
| version info                  | F1                        |
| set all viewports to same size| F2                        |
| automatically arrange viewports | F3                        |
| new viewport                  | Ctrl + N                  |

**shortcuts for a single viewport (visible layer)**

| Action                        | Shortcut                  |
| ------                        | ------                    |
| quit viewport                 | Ctrl + W                  |
| add new image                 | Ctrl + O                  |
| save current image            | Ctrl + S                  |
| crop rectangle                | Crlt + left mouse         |
| toggle crop                   | Ctrl + right click        |
| save current crop             | Ctrl + X                  |
| next image                    | ⇩, ⇨                      |
| delete single image           | Del                       |
| previous image                | ⇧, ⇦                      |
| fit window to image           | Ctrl + F                  |
| reset histogram               | Ctrl + H                  |

**shortcuts for local effects (all layers in single viewport)**

| Action                        | Shortcut                  |
| ------                        | ------                    |
| reset histogram (all layers)  | Ctrl + Shift + H          |
| delete all images             | Ctrl + Del                |
| change hist. for all layers   | Shift + mouse (histogram) |

**shortcuts for global effects (synchronized across all viewports)**

| Action                        | Shortcut                  |
| ------                        | ------                    |
| zoom rectangle                | Shift + left mouse        |
| zoom in                       | Ctrl + plus               |
| zoom out                      | Ctrl + minus              |
| no zoom                       | Ctrl + 0                  |
| fit to window                 | Ctrl + 9                  |
| center image                  | Ctrl + C                  |
| toggle marker                 | Ctrl + M                  |
| toggle marker                 | right click               |
| shift image 10% right         | Shift + ⇨                 |
| shift image 10% left          | Shift + ⇦                 |
| shift image 10% down          | Shift + ⇩                 |
| shift image 10% up            | Shift + ⇧                 |
| next image in all viewports   | Ctrl + ⇩, Ctrl + ⇨        |
| prev. image in all viewports  | Ctrl + ⇧, Ctrl + ⇦        |

