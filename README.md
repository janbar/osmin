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

git, cmake, clang/clang++ (**>=12.0**), OpenMP, Qt5Core (**>=5.12**), Qt5Gui, Qt5Qml, Qt5Quick, Qt5QuickControls2, Qt5Widgets, Qt5Xml, Qt5Svg, Qt5Network, Qt5Sensors, Qt5Multimedia, Qt5RemoteObjects, Qt5Positioning.

As example type the following on Ubuntu (>=20.04) to install all requirements.
```
sudo apt install git cmake clang curl wget libomp-dev liblzma-dev libqt5remoteobjects5-bin libqt5quickwidgets5 libqt5quickcontrols2-5 libqt5qmlmodels5 libqt5qml5 libqt5positioning5 libqt5remoteobjects5-dev libqt5svg5-dev libqt5sensors5-dev qtquickcontrols2-5-dev qtmultimedia5-dev qtpositioning5-dev qml-module-qtgraphicaleffects qml-module-qtquick2 qml-module-qtquick-layouts qml-module-qtquick-controls2 qml-module-qt-labs-settings
```

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
```
To build for desktop, use the following command.
```
cmake -B build -DCMAKE_C_COMPILER=/usr/bin/clang -DCMAKE_CXX_COMPILER=/usr/bin/clang++ .
```
To build for mobile devices, use the following command to enable behaviors for mobile.
```
cmake -B build -DBUILD_DEVICE_MOBILE=ON -DCMAKE_C_COMPILER=/usr/bin/clang -DCMAKE_CXX_COMPILER=/usr/bin/clang++ .
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

