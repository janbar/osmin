
### Requirements:

- Android NDK r18b (support C++17)
- Android SDK 26-28
- Java jdk 1.8
- Qt Creator 4.12

### Configure the build:

The template of the manifest must be created depending of QT/NDK versions.

- Qt 5.12 -> ndk18b -> AndroidManifest.xml.in.ndk18b
- Qt 5.15 -> ndk21e -> AndroidManifest.xml.in.ndk21e

cp AndroidManifest.xml.in.${NDK_REVISION} AndroidManifest.xml.in

source ./configure-${ARCH}.sh -DKEYSTORE_FILE=~/.android/janbar.keystore -DKEYSTORE_ALIAS=key0 -DKEYSTORE_PASSWORD=${PASSWORD}

