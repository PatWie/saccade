 # Saccade
[![Build Status](http://ci.patwie.com/api/badges/PatWie/saccade/status.svg)](http://ci.patwie.com/PatWie/saccade)
[![GitHub release](https://img.shields.io/badge/release-get_latest-blue.svg)](https://files.patwie.com/builds/saccade/)

A **saccade** (*/səˈkɑːd/ sə-KAHD*) is a quick and simultaneous movement of both eyes in the same direction.

This project is the missing scientific HDR image viewer inspired by [shiver/idisplay](https://sourceforge.net/p/shiver/idisplay). 

When working on computer vision problems a good image viewer can ease the process of inspecting images. The *saccade* project aims at providing an efficient image viewer with some sophisticated functions for comparing images. When dragging and zooming an image in one canvas, all other canvases synchronously also zoom and shift, so you get the pixel position in all images. Further, a marker can be set precisely on a specific image and all other views share the marker at exact the same position.

The image viewer features an OpenGL-based canvas to display images with dragging and zooming. It uses the [FreeImage](freeimage.sourceforge.net) library to load images from different types.

<p align="center"> <img src="https://github.com/patwie-stuff/img/blob/master/screenshot.gif?raw=true"> </p>

The concept of Saccade is that are multiple canvases (each window so speaking) consists of different layers (all images per canvas). Each canvase has an active layer (the one containing the image you see).

Some action might affect only the *active* layer accross *all* canvases (global effects), e.g., zooming and dragging.

When manipulating the histogram, the changes can be applied to only the current *active* layer or *all* layers in the *current* canvas (holding the `SHIFT` key).

## Features

- OpenGL accelerated viewer using MipMap data structure
- synchronize multiple views when dragging and zooming within one view
- drag'n drop for open images
- set marker on a specific pixel
- filewatcher (images will be reloaded when they change)
- interactive histogram widget which effects the image
- supported fileformats: png jpg JPG bmp ppm tif CR2
- helpful commands to arange multiple windows

## Install from binary

The related binaries for the master branch are generated automatically and static linked to Qt5.9 to reduce dependencies and filesize. The [pre-compile binaries](https://files.patwie.com/builds/saccade/) are available with *all dependencies* for Ubuntu 16.04.

## Install from Source

### Requirements

I use it on Ubuntu 16.04 with Qt5.9. The requirements are currently:

- Qt5.9 (should work with version >= 5.4)
- CMake
- FreeImage library
- Google Log
- Google Flags

I plan to add CUDA for speed up some functions.

### Get dependencies and compile

See the [dockerfile](https://github.com/PatWie/saccade/blob/docker/.docker/dynamic/Dockerfile) for all installing all dependencies.

Assuming you are on Ubuntu 16.04 getting all dependencies is done by

    # install dependencies
    sudo apt-get install libfreeimage3 libfreeimage-dev libgflags-dev libgoogle-glog-dev

The Ubuntu-packages contain old versions of Qt5. To get a recent Qt5 version use the following commands (you might need to register for Qt).

    cd /tmp
    wget http://download.qt.io/official_releases/qt/5.9/5.9.1/qt-opensource-linux-x64-5.9.1.run
    chmod +x qt-opensource-linux-x64-5.9.1.run
    ./qt-opensource-linux-x64-5.9.1.run

And finally just get this repository by

    # clone this repo
    git clone https://github.com/PatWie/saccade.git
    cd saccade
    mkdir build
    cd build
    cmake -DCMAKE_BUILD_TYPE=Release ..
    make

A copy and paste version to automate the GUI installer is described in the [Dockerfile](https://github.com/PatWie/saccade/blob/master/.deploy/images/dynamic/Dockerfile).

To add the saccade application to the uniy-launcher, edit `saccade.desktop` and adjust the paths. Then run

    cp saccade.desktop $HOME/.local/share/applications/saccade.desktop

and you find the app icon in the Ubuntu searchbar. When debugging the application it might be helpful to start it with the flag `--logtostderr 1`.

## Keyboard Shortcuts


General shortcuts

| Action                        | Shortcut                  |
| ------                        | ------                    |
| quit entire app               | Ctrl + Q                  |
| about and version info        | F1                        |
| distribute window geometry    | F2                        |
| automatically arrange windows | F3                        |
| new canvas                    | Ctrl + N                  |

shortcuts for a single canvas (visible layer)

| Action                        | Shortcut                  |
| ------                        | ------                    |
| quit canvas                   | Ctrl + W                  |
| add new image                 | Ctrl + O                  |
| next image                    | ⇩, ⇨                      |
| delete single image           | Del                       |
| previous image                | ⇧, ⇦                      |
| fit window to image           | Ctrl + F                  |
| reset histogram               | Ctrl + H                  |

shortcuts for local effects (all layers in single canvas)

| Action                        | Shortcut                  |
| ------                        | ------                    |
| reset histogram (all layers)  | Ctrl + Shift + H          |
| delete all images             | Ctrl + Del                |
| change hist. for all layers   | Shift + mouse (histogram) |

shortcuts for global effects (synchronized across all canvases)

| Action                        | Shortcut                  |
| ------                        | ------                    |
| zoom in                       | Ctrl + plus               |
| zoom out                      | Ctrl + minus              |
| no zoom                       | Ctrl + 0                  |
| fit to window                 | Ctrl + 9                  |
| center image                  | Ctrl + C                  |
| toggle marker                 | Ctrl + M                  |
| shift image 10% right         | Shift + ⇨                 |
| shift image 10% left          | Shift + ⇦                 |
| shift image 10% down          | Shift + ⇩                 |
| shift image 10% up            | Shift + ⇧                 |
| next image in all canvases    | Ctrl + ⇩, Ctrl + ⇨        |
| prev. image in all canvases   | Ctrl + ⇧, Ctrl + ⇦        |

