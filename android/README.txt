
### Requirements:

- Android SDK 29
- Java jdk 1.8
- Qt Creator 6.0

- Qt = 5.15 : Android NDK 22.1.7171670

### Configure the build:

The template of the manifest must be created depending of QT/NDK versions.

- Qt 5.15 -> ndk21 -> AndroidManifest.xml.in.ndk21

source ./linux-build.sh -DKEYSTORE_FILE=~/.android/janbar.keystore -DKEYSTORE_ALIAS=key0 -DKEYSTORE_PASSWORD=${PASSWORD}

### Extras

- Setup for marshmallow:
  ANDROID_NATIVE_API_LEVEL=23, ANDROID_SDK_MINVER=23, ANDROID_SDK_TARGET=23

