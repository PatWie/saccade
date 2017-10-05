 # Saccade

A **saccade** (*/səˈkɑːd/ sə-KAHD*) is a quick and simultaneous movement of both eyes in the same direction.

When working on computer vision problems a good image viewer can ease the process of inspecting images. The *saccade* project aims at providing an efficient image viewer with some sophisticated functions for comparing images. When dragging and zooming an image in one canvas, all other canvases synchronously also zoom and shift, so you get the pixel position in all images. Further, a marker can be set precisely on a specific image and all other views share the marker at exact the same position.

The image viewer features an OpenGL-based canvas to display images with dragging and zooming. It uses the [FreeImage](freeimage.sourceforge.net) library to load images from different types.

<p align="center"> <img src="https://github.com/patwie-stuff/img/blob/master/screenshot.gif?raw=true"> </p>

This project is the missing scientific HDR image viewer inspired by [shiver/idisplay](https://sourceforge.net/p/shiver/idisplay). 

## Features

- OpenGL accelerated viewer using MipMap data structure
- synchronize multiple views when dragging and zooming within one view
- drag'n drop for open images
- set marker on a specific pixel
- filewatcher (images will be reloaded when they change)
- interactive histogram widget which effects the image
- supported fileformats: png jpg JPG bmp ppm tif CR2
- helpful commands to arange multiple windows

## Requirements

I use it on Ubuntu 16.04 with Qt5.9. The requirements are currently:

- Qt5.9 (should work with version >= 5.4)
- CMake
- FreeImage library
- Google Log
- Google Flags

I plan to add CUDA for speed up some functions.

## Install from Source

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
    cmake ..
    make

To add the saccade application to the uniy-launcher, edit `saccade.desktop` and adjust the paths. Then run

    cp saccade.desktop $HOME/.local/share/applications/saccade.desktop

and you find the app in the Ubuntu searchbar. When debugging the application it might be helpful to start it with the flag `--logtostderr 1`.

## Keyboard Shortcuts

General shortcuts

| Action                       | Shortcut             |
| ------                       | ------               |
| quit entire app              | Ctrl + Q             |
| about and version info       | F1                   |
| distribute window geometry   | F2                   |
| automatically arange windows | F3                   |

shortcuts for a single canvas

| Action                       | Shortcut             |
| ------                       | ------               |
| new canvas                   | Ctrl + N             |
| quit                         | Ctrl + W             |
| next image                   | key_down, key_right  |
| delete image                 | Del                  |
| previous image               | key_up, key_left     |
| add new image                | Ctrl + O             |
| zoom in                      | Ctrl + plus          |
| zoom out                     | Ctrl + minus         |
| no zoom                      | Ctrl + 0             |
| fit to window                | Ctrl + 9             |
| reset histogram              | Ctrl + H             |
| center image                 | Ctrl + C             |
| fit window to image          | Ctrl + F             |
| toggle marker                | Ctrl + M             |
| selection tool               | Shift + "left click" |
