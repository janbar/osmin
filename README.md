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

git, cmake, clang/clang++ (**>=10.0**), Qt5Core (**=5.12**), Qt5Gui, Qt5Qml, Qt5Quick, Qt5QuickControls2, Qt5Widgets, Qt5Xml, Qt5Svg, Qt5Network, Qt5Sensors, Qt5Multimedia.

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
