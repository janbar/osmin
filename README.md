# osmin
A GPS navigator On-Road/Off-Road for Android device.

Strongly inspired by osmscout by Karry, it uses a fork of libosmscout as backend. It includes features such as generic compass, tracker, GPX reader/writer, road router, POI database.

<p align="center">
  <img src="https://github.com/janbar/osmin/raw/master/screenshots/tracking.png"/>
  <img src="https://github.com/janbar/osmin/raw/master/screenshots/informations.png"/>
  <img src="https://github.com/janbar/osmin/raw/master/screenshots/routing.png"/>
<p>

## Routing with your Android device
Supported Android OS is Nougat (Android 7.0) or newer, LineageOS 14.1 or newer.

## Develop/debug osmin
You can build and test osmin on Unix (Linux, BSD, MacOS 10.14) supported by Qt 5.12. Before build from source you have to install the following dependencies.

git, cmake, clang/clang++ (**>=10.0**), Qt5Core (**>=5.12**), Qt5Gui, Qt5Qml, Qt5Quick, Qt5QuickControls2, Qt5Widgets, Qt5Xml, Qt5Svg, Qt5Network, Qt5Sensors, Qt5Multimedia, Qt5RemoteObjects, Qt5::Positioning.

## Build on Unix from source

Clone the sources
```
git clone https://github.com/janbar/osmin.git
```
Move to the sources path and configure the build
```
cd osmin
git submodule init
git submodule update
mkdir build
cmake -B build -DCMAKE_C_COMPILER=/usr/bin/clang -DCMAKE_CXX_COMPILER=/usr/bin/clang++ . 
```
Finally build it to make the target binary `osmin`
```
cmake --build build/ -j8
```

## About osmin on Android

Installation of the APK can be achieved without issue. From release 1.7.0, **the storage for maps is no longer supported in the external SD card** of the device, due to security limitations with Android 10 and up. Therefore all data are saved in the internal storage of the device, i.e `Android/data/io.github.janbar.osmin/files/`. This folder is accessible when the device is plugged to an USB port, or directly using the android file manager. So you could manually manage the content yourself. New maps could be manually uploaded, or removed to make free space.
Downloading big map could failed in background because android stop the activity as soon as possible when the app isn't on front or the device change to sleeping state. If you are unable to keep the device active, you should prefer to manually upload the map using the USB port.

## About osmin on Unix desktop

On startup, the application creates storage folders in the user's home directory: `osmin` and `Maps`. The first contains user data, configuration files and resources. The last contains downloaded maps.

## Configure hillshade tile server

To enable the hill shading feature, you need to configure the file `hillshade-tile-server.json` from the folder *resources*, i.e. *~/osmin/resources* or *Android/data/io.github.janbar.osmin/files/resources*. Previously the service was provided by wmflabs.org. Unfortunately it is no longer available today. So you can use your own server or any other providing the tile service. An example file looks like the following.
```
{
  "id": "wmflabs",
  "name": "wmflabs",
  "servers": [
    "http://tiles.wmflabs.org/hillshading/%1/%2/%3.png"
  ],
  "maximumZoomLevel": 18, 
  "copyright": "© wmflabs Hillshading"
}
```
The arguments 1-3 are respectively the zoom (Z), and coordinates (X , Y).

