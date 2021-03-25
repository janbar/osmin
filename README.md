# osmin
A GPS navigator On-Road/Off-Road for Android device and SailfishOS.

Strongly inspired by osmscout by Karry, it uses a fork of libosmscout as backend. It includes features such as generic compass, tracker, GPX reader/writer, road router, POI database.

<p align="center">
  <img src="https://github.com/janbar/osmin/raw/master/osmin-screenshot.png"/>
<p>

## Routing with your Android device
To have the best routing experience, you should turn off location improvement with Google services. So using only GPS data. By default, Osmin will prefer GPS data but external services may interfere. 

## Build from source
**The build can be achieve only with the clang compiler.**

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
