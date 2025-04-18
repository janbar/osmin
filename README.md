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

## Install osmin for Android
You can install osmin through the [IzzyOnDroid](https://apt.izzysoft.de/fdroid/index/apk/io.github.janbar.osmin) F-Droid repo, which should be available by enabling IzzyOnDroid from Settings > Repositories, otherwise, please refer to instructions at [IzzyOnDroid](https://apt.izzysoft.de/fdroid/) main page.
Alternatively the APKs are available on the Release page, for armv7(32bits) and arm64(64bits) devices.

## Develop/debug osmin
You can build and test osmin on Unix (Linux, BSD) supported by Qt 5.15. Before build from source you have to install the following dependencies.

git, cmake (**>=3.20), clang/clang++ (**>=18.0** standard C++20), OpenMP, Qt5Core (**>=5.15**), Qt5Gui, Qt5Qml, Qt5Quick, Qt5QuickControls2, Qt5Widgets, Qt5Xml, Qt5Svg, Qt5Network, Qt5Sensors, Qt5Multimedia, Qt5RemoteObjects, Qt5Positioning, Qt5DBus, libdbus-1, zlib1g, libxml2, liblzma, OpenSSL


As example type the following on Ubuntu (>=24.04) to install all requirements.
```
sudo apt install git cmake clang curl wget libomp-dev zlib1g-dev libxml2-dev liblzma-dev libdbus-1-dev libssl-dev libreadline-dev libqt5remoteobjects5-bin libqt5quickwidgets5 libqt5quickcontrols2-5 libqt5qmlmodels5 libqt5qml5 libqt5positioning5 libqt5remoteobjects5-dev libqt5svg5-dev libqt5sensors5-dev libqt5dbus5 qtquickcontrols2-5-dev qtmultimedia5-dev qtpositioning5-dev qml-module-qtgraphicaleffects qml-module-qtquick2 qml-module-qtquick-layouts qml-module-qtquick-controls2 qml-module-qt-labs-settings
```
Or type the following on Fedora 40.
```
sudo dnf install git cmake clang curl wget libomp-devel zlib-ng-compat-devel libxml2-devel dbus-devel readline-devel openssl-devel qt5-qtbase-devel qt5-qtremoteobjects-devel qt5-qtsensors-devel qt5-qtsvg-devel qt5-qtdeclarative-devel qt5-qtmultimedia-devel qt5-qtquickcontrols2-devel qt5-qtlocation-devel qt5-qtdeclarative qt5-qtbase qt5-qtsvg qt5-qtbase-gui qt5-qtgraphicaleffects qt5-qtremoteobjects qt5-qtsensors qt5-qtquickcontrols2 qt5-qtimageformats qt5-qtlocation qt5-qtmultimedia qt5-qttools qt5-qtxmlpatterns qt5-qtmultimedia-devel qt5-qtlocation-devel qt5-linguist qt5-qttranslations
```

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
```
To build for desktop including the simulation tool, use the following command.
```
cmake -B build -DBUILD_SIMULATOR=ON -DCMAKE_C_COMPILER=/usr/bin/clang -DCMAKE_CXX_COMPILER=/usr/bin/clang++ .
```
If the target device is a mobile, you should use the following command to **enable behaviors for mobile**.
```
cmake -B build -DBUILD_DEVICE_MOBILE=ON -DCMAKE_C_COMPILER=/usr/bin/clang -DCMAKE_CXX_COMPILER=/usr/bin/clang++ .
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

