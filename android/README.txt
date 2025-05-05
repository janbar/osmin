
### Requirements:

- Android SDK 35
- Java jdk 21
- Qt Creator 16

- Qt 6.9.0 C++17: Android NDK 26.3.11579264
- Qt 6.9.0 C++20: Android NDK 27.2.12479018

- ANDROID_NATIVE_API_LEVEL >= 24
- ANDROID_SDK_MINVER >= 26
- ANDROID_SDK_TARGET >= 26

### Configure the build:

The template of the manifest must be created depending of QT/NDK versions.

source ./linux-build.sh -DKEYSTORE_FILE=~/.android/janbar.keystore -DKEYSTORE_ALIAS=key0 -DKEYSTORE_PASSWORD=${PASSWORD}

