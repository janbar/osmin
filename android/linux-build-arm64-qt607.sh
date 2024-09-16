BUILD_DIR=build-arm64-607
rm -rf $BUILD_DIR/*
mkdir -p $BUILD_DIR

export JAVA_HOME=$HOME/bin/java/jdk-21.0.4
export ANDROID_SDK_ROOT=$HOME/bin/android/sdk
export ANDROID_NDK=$HOME/bin/android/sdk/ndk/26.3.11579264
export QT_DIR=$HOME/bin/Qt/6.7.2/android_arm64_v8a
export QT_HOST_PATH=$HOME/bin/Qt/6.7.2/gcc_64

cmake .. -B $BUILD_DIR -DCMAKE_SYSTEM_NAME=Android \
-DCMAKE_PREFIX_PATH=${QT_DIR} \
-DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake \
-DCMAKE_MAKE_PROGRAM=$ANDROID_NDK/prebuilt/linux-x86_64/bin/make \
-DCMAKE_BUILD_TYPE=Release \
-DANDROID_ABI="arm64-v8a" \
-DANDROID_SDK_MINVER=26 \
-DANDROID_SDK_TARGET=26 \
-DANDROID_NATIVE_API_LEVEL=24 \
-DANDROID_SDK_BUILD_TOOLS_REVISION="31.0.0" \
-DANDROID_SDK_ROOT=${ANDROID_SDK_ROOT} \
-DANDROID_NDK=${ANDROID_NDK} \
-DQT_ANDROID_PLATFORM_LEVEL=34 \
-DQt6_DIR=${QT_DIR}/lib/cmake/Qt6 \
-DQt6Core_DIR=${QT_DIR}/lib/cmake/Qt6Core \
-DQt6ZlibPrivate_DIR=${QT_DIR}/lib/cmake/Qt6ZlibPrivate \
-DQt6Gui_DIR=${QT_DIR}/lib/cmake/Qt6Gui \
-DQt6Qml_DIR=${QT_DIR}/lib/cmake/Qt6Qml \
-DQt6QmlIntegration_DIR=${QT_DIR}/lib/cmake/Qt6QmlIntegration \
-DQt6QmlBuiltins_DIR=${QT_DIR}/lib/cmake/Qt6QmlBuiltins \
-DQt6Network_DIR=${QT_DIR}/lib/cmake/Qt6Network \
-DQt6OpenGL_DIR=${QT_DIR}/lib/cmake/Qt6OpenGL \
-DQt6Core5Compat_DIR=${QT_DIR}/lib/cmake/Qt6Core5Compat \
-DQt6QuickTemplates2_DIR=${QT_DIR}/lib/cmake/Qt6QuickTemplates2 \
-DQt6Quick_DIR=${QT_DIR}/lib/cmake/Qt6Quick \
-DQt6QuickControls2_DIR=${QT_DIR}/lib/cmake/Qt6QuickControls2 \
-DQt6Xml_DIR=${QT_DIR}/lib/cmake/Qt6Xml \
-DQt6Svg_DIR=${QT_DIR}/lib/cmake/Qt6Svg \
-DQt6Widgets_DIR=${QT_DIR}/lib/cmake/Qt6Widgets \
-DQt6Sensors_DIR=${QT_DIR}/lib/cmake/Qt6Sensors \
-DQt6Positioning_DIR=${QT_DIR}/lib/cmake/Qt6Positioning \
-DQt6Multimedia_DIR=${QT_DIR}/lib/cmake/Qt6Multimedia \
-DQt6QmlModels_DIR=${QT_DIR}/lib/cmake/Qt6QmlModels \
-DQt6RemoteObjects_DIR=${QT_DIR}/lib/cmake/Qt6RemoteObjects \
$@

[ $? -eq 0 ] && cmake --build $BUILD_DIR --parallel 8

