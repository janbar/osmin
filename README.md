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

**It is strongly recommended to disable "Battery optimization" for the Osmin app**. Otherwise, the app may be stopped by the system based on optimization rules.

## Install osmin for Android
The APKs are available on the Release page, for armv7(32bits) and arm64(64bits) devices.
Alternatively you can install osmin through the [IzzyOnDroid](https://apt.izzysoft.de/fdroid/index/apk/io.github.janbar.osmin) F-Droid repo, which should be available by enabling IzzyOnDroid from Settings > Repositories, otherwise, please refer to instructions at [IzzyOnDroid](https://apt.izzysoft.de/fdroid/) main page.

## Develop/debug osmin
You can build and test osmin on Unix (Linux, BSD) supported by Qt 6.9. Before build from source you have to install the following dependencies.

git, cmake (**>=3.20), clang/clang++ (**>=18.0** standard C++20), OpenMP, Qt6Core (**>=6.9.0**), Qt6Gui, Qt6Qml, Qt6Quick, Qt6QuickControls2, Qt6Widgets, Qt6Xml, Qt6Svg, Qt6Network, Qt6Sensors, Qt6Multimedia, Qt6RemoteObjects, Qt6Positioning, Qt6DBus, libdbus-1, zlib1g, libxml2, liblzma, OpenSSL


As example type the following on Ubuntu (>=24.04) to install the base requirements.
```
sudo apt install git cmake clang curl wget libomp-dev zlib1g-dev libxml2-dev liblzma-dev libdbus-1-dev libssl-dev libgl1-mesa-dev libreadline-dev libpulse0 mesa-vulkan-drivers
```
Or type the following on Fedora 40.
```
sudo dnf install git cmake clang curl wget libomp-devel zlib-ng-compat-devel libxml2-devel dbus-devel openssl-devel mesa-libGL-devel readline-devel pulseaudio-libs mesa-vulkan-drivers
```
If your distribution doesn't provide the Qt-6.9 libraries, you have to install them using the Qt Online Installer.

## Build on Unix from source

Clone the sources
```
git clone https://github.com/janbar/osmin.git
```
Move to the sources path and configure the build.
```
cd osmin
git submodule init
git submodule update --force
mkdir build
export QT_ROOT=$HOME/Qt/6.9.0/gcc_64
```
To build arch x86_64 for desktop including the simulation tool, use the following command.
```
cmake -B build -DBUILD_SIMULATOR=ON -DCMAKE_C_COMPILER=/usr/bin/clang -DCMAKE_CXX_COMPILER=/usr/bin/clang++ -DCMAKE_PREFIX_PATH=${QT_ROOT} -DCMAKE_FIND_ROOT_PATH=${QT_ROOT} .
```
If the target device is a mobile, you should use the following command to **enable behaviors for mobile**.
```
cmake -B build -DBUILD_DEVICE_MOBILE=ON -DCMAKE_C_COMPILER=/usr/bin/clang -DCMAKE_CXX_COMPILER=/usr/bin/clang++ -DCMAKE_PREFIX_PATH=${QT_ROOT} -DCMAKE_FIND_ROOT_PATH=${QT_ROOT} .
```
You could mix the options `BUILD_SIMULATOR` and `BUILD_DEVICE_MOBILE` to test the behaviors for mobile device with the simulation tool.
Finally build it to make the target binary `osmin`.
```
cmake --build build/ -j8
```
Running Osmin outside the build tree, you have to install the required assets. 
```
sudo cmake --build build/ --target install
```
Uninstall can be done by the following `sudo cmake --build build/ --target uninstall`.

## About osmin on Android

Installation of the APK can be achieved without issue. From release 1.11.0, **the storage for maps is no longer permitted in the external storage** of the device, due to limitations with Android 14 and up. Therefore map databases are stored in the internal storage of the device, and user has no access to them. GPX files, Favorites file, and some configuration files are stored in user area, i.e `Android/data/io.github.janbar.osmin/files/`. This folder is accessible when the device is plugged to an USB port, or directly using the android file manager. So you could manage these contents yourself.

## About osmin on Unix desktop

On startup, the application creates storage folders in the user's home directory: `osmin`. It contains user data, configuration files, downloaded maps or voices, and resources.
The binary of the simulation tool is not installed in the system tree. It can be found in the build tree `simulator/osmin-simulator`.

