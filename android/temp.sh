BUILD_DIR=build-x86-515
rm -rf $BUILD_DIR/*
mkdir -p $BUILD_DIR

export JAVA_HOME=/Library/Java/JavaVirtualMachines/jdk-17.jdk/Contents/Home
export ANDROID_SDK=/Users/jlb/Android/Sdk
export ANDROID_NDK=/Users/jlb/Android/Sdk/ndk/26.3.11579264
export ANDROID_NATIVE_API_LEVEL=30
export ANDROID_SDK_MINVER=24
export ANDROID_SDK_TARGET=33
export QT_DIR=/Users/jlb/Qt/6.7.2/android_x86_64
export QT_HOST_PATH=/Users/jlb/Qt/6.7.2/macos

cmake .. -B $BUILD_DIR -DCMAKE_SYSTEM_NAME=Android \
-DCMAKE_PREFIX_PATH=$QT_DIR \
-DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake \
-DCMAKE_MAKE_PROGRAM=$ANDROID_NDK/prebuilt/darwin-x86_64/bin/make \
-DCMAKE_BUILD_TYPE=Release \
-DANDROID_ABI="x86_64" \
-DANDROID_SDK_MINVER=$ANDROID_SDK_MINVER \
-DANDROID_SDK_TARGET=$ANDROID_SDK_TARGET \
-DANDROID_NATIVE_API_LEVEL=$ANDROID_NATIVE_API_LEVEL \
-DQT_ANDROID_SDK_ROOT=$ANDROID_SDK \
-DQT_ANDROID_NDK_ROOT=$ANDROID_NDK \
-DQT_ANDROID_SDK_BUILDTOOLS_REVISION="34.0.0" \
-DQT_ANDROID_PLATFORM_LEVEL=34 \
-DQt6_DIR=$QT_DIR/lib/cmake/Qt6 \
-DQt6Core_DIR=$QT_DIR/lib/cmake/Qt6Core \
-DQt6Gui_DIR=$QT_DIR/lib/cmake/Qt6Gui \
-DQt6Qml_DIR=$QT_DIR/lib/cmake/Qt6Qml \
-DQt6QmlIntegration_DIR=$QT_DIR/lib/cmake/Qt6QmlIntegration \
-DQt6QmlBuiltins_DIR=$QT_DIR/lib/cmake/Qt6QmlBuiltins \
-DQt6Network_DIR=$QT_DIR/lib/cmake/Qt6Network \
-DQt6OpenGL_DIR=$QT_DIR/lib/cmake/Qt6OpenGL \
-DQt6Core5Compat_DIR=$QT_DIR/lib/cmake/Qt6Core5Compat \
-DQt6QuickTemplates2_DIR=$QT_DIR/lib/cmake/Qt6QuickTemplates2 \
-DQt6Quick_DIR=$QT_DIR/lib/cmake/Qt6Quick \
-DQt6QuickControls2_DIR=$QT_DIR/lib/cmake/Qt6QuickControls2 \
-DQt6Xml_DIR=$QT_DIR/lib/cmake/Qt6Xml \
-DQt6Svg_DIR=$QT_DIR/lib/cmake/Qt6Svg \
-DQt6Widgets_DIR=$QT_DIR/lib/cmake/Qt6Widgets \
-DQt6Sensors_DIR=$QT_DIR/lib/cmake/Qt6Sensors \
-DQt6Positioning_DIR=$QT_DIR/lib/cmake/Qt6Positioning \
-DQt6Multimedia_DIR=$QT_DIR/lib/cmake/Qt6Multimedia \
-DQt6AndroidExtras_DIR=$QT_DIR/lib/cmake/Qt6AndroidExtras \
-DQt6QmlModels_DIR=$QT_DIR/lib/cmake/Qt6QmlModels \
-DQt6RemoteObjects_DIR=$QT_DIR/lib/cmake/Qt6RemoteObjects \
$@

[ $? -eq 0 ] && cmake --build $BUILD_DIR --parallel 4

