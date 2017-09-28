 # EagleEye

A scientific image viewer inspired by [shiver/idisplay](https://sourceforge.net/p/shiver/idisplay). It features an OpenGL-based canvas to display images with dragging and zooming. These actions are further synchronized across different windows.

This is currently under development and lacks some of the original features. It uses FreeImage to load the images.

<p align="center"> <img src="./screenshot.jpg"> </p>

## Features

- drop placement of images
- OpenGL accelerated viewer
- synchronize views when moving and zooming
- mipmap based datastructure
- set marker on pixels
- filewatcher

## Requirements

- Qt5.9
- CMake
- FreeImage library

## Install from Source

To get Qt5

    cd /tmp
    wget http://download.qt.io/official_releases/qt/5.9/5.9.1/qt-opensource-linux-x64-5.9.1.run
    chmod +x qt-opensource-linux-x64-5.9.1.run
    ./qt-opensource-linux-x64-5.9.1.run

Then get FreeImage

    # install dependencies
    sudo apt-get install libfreeimage3 libfreeimage-dev libgflags-dev libgoogle-glog-dev

and finally this repository

    # clone this repo
    git clone https://github.com/PatWie/eagle_eye.git
    cd eagle_eye
    mkdir build
    cd build
    cmake ..
    make

edit `eagle_eye.desktop` and adjust the paths. Then 

    cp eagle_eye.desktop $HOME/.local/share/applications/eagle_eye.desktop

and you find the app in the Ubuntu searchbar.

## Keyboard Shortcuts


| Action         | Shortcut            |
| ------         | ------              |
| next image     | key_down, key_right |
| delete image   | Del                 |
| previous image | key_up, key_left    |
| open image     | Ctrl+O              |
| new canvas     | Ctrl+N              |
| quit canvas    | Ctrl+W              |
| zoom in        | Ctrl+plus           |
| no zoom        | Ctrl+0              |
| fit to window  | Ctrl+9              |
| zoom out       | Ctrl+minus          |
