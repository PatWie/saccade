 # Saccade

 A **saccade** (*/səˈkɑːd/ sə-KAHD*) is a quick and simultaneous movement of both eyes in the same direction.

This project is the missing scientific HDR image viewer inspired by [shiver/idisplay](https://sourceforge.net/p/shiver/idisplay). Compare muliple images by switching between them without loosing the pixel position. Drag and zoom within one canvas and the viewer automatically synchronizes the changes across different all viewports.

The image viewer features an OpenGL-based canvas to display images with dragging and zooming. It uses the [FreeImage](freeimage.sourceforge.net) library to load images from different types.

<p align="center"> <img src="https://github.com/patwie-stuff/img/blob/master/screenshot.gif?raw=true"> </p>

## Features

- OpenGL accelerated viewer using MipMap data structure
- synchronize multiple views when dragging and zooming within one view
- drag 'nd drop for importing images
- set marker on a specific pixel
- filewatcher (image will be reloaded when it changes)
- change colors of images (interactive histogram widget)
- supported fileformats: png jpg JPG bmp ppm tif CR2

## Requirements

- Qt5.9
- CMake
- FreeImage library
- Google Log
- Google Flags

## Install from Source

Get all dependencies:

    # install dependencies
    sudo apt-get install libfreeimage3 libfreeimage-dev libgflags-dev libgoogle-glog-dev

Get Qt5:

    cd /tmp
    wget http://download.qt.io/official_releases/qt/5.9/5.9.1/qt-opensource-linux-x64-5.9.1.run
    chmod +x qt-opensource-linux-x64-5.9.1.run
    ./qt-opensource-linux-x64-5.9.1.run


and finally get this repository

    # clone this repo
    git clone https://github.com/PatWie/saccade.git
    cd saccade
    mkdir build
    cd build
    cmake ..
    make

Edit `saccade.desktop` and adjust the paths. Then 

    cp saccade.desktop $HOME/.local/share/applications/saccade.desktop

and you find the app in the Ubuntu searchbar.

## Keyboard Shortcuts

general

| Action                       | Shortcut            |
| ------                       | ------              |
| quit entire app              | Ctrl + Q            |
| distribute window geometry   | F2                  |
| automatically arange windows | F3                  |

per canvas

| Action                       | Shortcut            |
| ------                       | ------              |
| new canvas                   | Ctrl + N            |
| quit                         | Ctrl + W            |
| next image                   | key_down, key_right |
| delete image                 | Del                 |
| previous image               | key_up, key_left    |
| add new image                | Ctrl + O            |
| zoom in                      | Ctrl + plus         |
| zoom out                     | Ctrl + minus        |
| no zoom                      | Ctrl + 0            |
| fit to window                | Ctrl + 9            |
| reset histogram              | Ctrl + H            |
| center image                 | Ctrl + C            |
| fit window to image          | Ctrl + F            |
