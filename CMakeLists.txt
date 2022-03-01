cmake_minimum_required(VERSION 3.8.2)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

project(osmin C CXX)

option(INSTALL_TESTS "Install the tests on make install" OFF)
option(BUILD_SAILFISHOS "Build the SailfishOS application" OFF)

set(APP_VERSION       "1.7.0")
set(APP_VERSION_CODE  38)
set(APP_ID            "io.github.janbar.osmin")
set(APP_NAME          "osmin")

if(BUILD_SAILFISHOS)
    set(EXEC            "harbour-osmin")
    set(ICON_NAME       "${EXEC}")
    set(APP_DESKTOP_ID  "${EXEC}")
else()
    set(EXEC          	"osmin")
    set(ICON_NAME     	"${EXEC}")
    set(APP_DESKTOP_ID	"${EXEC}")
endif()

set(ICON_FILE         "osmin.png")
set(DESKTOP_FILE_NAME "osmin.desktop")
set(APPDATA_FILE_NAME "osmin.appdata.xml")

# Components PATH
execute_process(
    COMMAND dpkg-architecture -qDEB_HOST_MULTIARCH
    OUTPUT_VARIABLE ARCH_TRIPLET
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Set install paths
include(GNUInstallDirs)
set(APPDATA_DIR ${CMAKE_INSTALL_DATADIR}/metainfo)
set(DESKTOP_DIR ${CMAKE_INSTALL_DATADIR}/applications)
set(PLUGINS_DIR ${CMAKE_INSTALL_FULL_LIBDIR}/${EXEC})

if(BUILD_SAILFISHOS)
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/${APP_DESKTOP_ID}.desktop "[Desktop Entry]
Name=${APP_NAME}
Exec=${EXEC}
Icon=${ICON_NAME}
Type=Application
Categories=Maps;
X-Nemo-Application-Type=generic")

    install(
        FILES ${CMAKE_CURRENT_BINARY_DIR}/${APP_DESKTOP_ID}.desktop
        DESTINATION ${DESKTOP_DIR}
    )

    install(
        FILES "gui/images/${ICON_NAME}-86x86.png"
        DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/86x86/apps RENAME "${ICON_NAME}.png"
    )

    install(
        FILES "gui/images/${ICON_NAME}-108x108.png"
        DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/108x108/apps RENAME "${ICON_NAME}.png"
    )

    install(
        FILES "gui/images/${ICON_NAME}-128x128.png"
        DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/128x128/apps RENAME "${ICON_NAME}.png"
    )

    install(
        FILES "gui/images/${ICON_NAME}-172x172.png"
        DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/172x172/apps RENAME "${ICON_NAME}.png"
    )
else()
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/${APP_DESKTOP_ID}.desktop "[Desktop Entry]
Name=${APP_NAME}
Exec=${EXEC}
Icon=${ICON_NAME}
Terminal=false
Type=Application
Categories=Maps;")

    install(
        FILES ${CMAKE_CURRENT_BINARY_DIR}/${APP_DESKTOP_ID}.desktop
        DESTINATION ${DESKTOP_DIR}
    )

    install(
        FILES "gui/images/${ICON_FILE}"
        DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/256x256/apps RENAME "${ICON_NAME}.png"
    )

    configure_file(
        ${APPDATA_FILE_NAME}.in
        ${CMAKE_CURRENT_BINARY_DIR}/${APPDATA_FILE_NAME}
    )
endif()

if(NOT APPLE)
    # Provides OpenMP
    find_package(OpenMP REQUIRED)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
else()
    # force build all backend dependencies on apple
    option(BUILD_DEPENDENCIES "" ON)
endif()

# Workaround for QTBUG-87863 before calling add_subirectory
find_package(Qt5 COMPONENTS Core REQUIRED)

add_subdirectory(backend)

set(CMAKE_AUTOMOC ON)
find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Qml REQUIRED)
find_package(Qt5Quick REQUIRED)
if(BUILD_SAILFISHOS)
    find_package(PkgConfig)
    pkg_check_modules(LIBSAILFISHAPP REQUIRED sailfishapp)
else()
    set(QT_MIN_VERSION 5.12.0)
    find_package(Qt5QuickControls2 ${QT_MIN_VERSION} REQUIRED)
endif()
if(ANDROID)
    find_package(Qt5AndroidExtras REQUIRED)
endif()
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Xml REQUIRED)
find_package(Qt5Svg REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5Sensors REQUIRED)
find_package(Qt5Multimedia REQUIRED)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)

set(osmin_SOURCES src/osmin.cpp src/converter.cpp src/platformextras.cpp
    src/utils.cpp
    src/csvparser.cpp
    src/qmlfilterbehavior.cpp
    src/qmlsortbehavior.cpp
    src/qmlsortfiltermodel.cpp
    src/favoritesmodel.cpp
    src/gpxlistmodel.cpp
    src/gpxfilemodel.cpp
    src/tracker.cpp
    src/compass/genericcompass.cpp
    src/compass/plugin.cpp
)
set(osmin_HEADERS src/converter.h src/platformextras.h
    src/locked.h
    src/utils.h
    src/csvparser.h
    src/qmlfilterbehavior.h
    src/qmlsortbehavior.h
    src/qmlsortfiltermodel.h
    src/favoritesmodel.h
    src/gpxlistmodel.h
    src/gpxlistmodel.h
    src/tracker.h
    src/compass/genericcompass.h
    src/compass/plugin.h
)

if(UNIX)
    list(APPEND osmin_SOURCES src/signalhandler.cpp)
    list(APPEND osmin_HEADERS src/signalhandler.h)
endif()

add_definitions(-DAPP_VERSION="${APP_VERSION}")

if(BUILD_SAILFISHOS)
    qt5_add_resources(osmin-resources gui/osmin_silica.qrc)
elseif(Qt5Core_VERSION VERSION_LESS 5.15.0)
    qt5_add_resources(osmin-resources gui/osmin_controls2_509.qrc)
else()
    qt5_add_resources(osmin-resources gui/osmin_controls2_515.qrc)
endif()

include_directories(${LIBOSMSCOUT_INCLUDE_DIRS})

if(ANDROID)
    add_library(osmin SHARED ${osmin_SOURCES} ${osmin_HEADERS} ${osmin-resources})
else()
    add_executable(osmin ${osmin_SOURCES} ${osmin_HEADERS} ${osmin-resources})
endif()

target_link_libraries(osmin ${LIBOSMSCOUT_LIBRARIES})

add_definitions(-DQT_STATICPLUGIN)

if(BUILD_SAILFISHOS)
    add_definitions(-DSAILFISHOS)
    include_directories(${LIBSAILFISHAPP_INCLUDE_DIRS})
    target_link_libraries(osmin ${LIBSAILFISHAPP_LIBRARIES}
                          Qt5::Qml Qt5::Quick Qt5::Widgets Qt5::Gui Qt5::Xml Qt5::Svg Qt5::Network Qt5::Sensors Qt5::Multimedia Qt5::Core)
elseif(ANDROID)
    add_definitions(-frtti) # dynamic_cast: https://github.com/android/ndk/issues/519
    target_link_libraries(osmin Qt5::QuickControls2 Qt5::AndroidExtras android log
                          Qt5::Qml Qt5::Quick Qt5::Widgets Qt5::Gui Qt5::Xml Qt5::Svg Qt5::Network Qt5::Sensors Qt5::Multimedia Qt5::Core)
else()
    target_link_libraries(osmin Qt5::QuickControls2
                          Qt5::Qml Qt5::Quick Qt5::Widgets Qt5::Gui Qt5::Xml Qt5::Svg Qt5::Network Qt5::Sensors Qt5::Multimedia Qt5::Core)
endif()

if(ANDROID)
    # Fix import of shared omp with android ndk rev 21
    if(OpenMP_CXX_LIBRARIES MATCHES libomp.so)
       set(SHARED_LIBOMP ${OpenMP_CXX_LIBRARIES})
    endif()
    add_dependencies(osmin ssl crypto)
    include(android/AddQtAndroidApk.cmake)
    add_qt_android_apk(osmin-apk osmin
        NAME ${APP_NAME}
        VERSION_CODE ${APP_VERSION_CODE}
        PACKAGE_NAME ${APP_ID}
        PACKAGE_SOURCES ${CMAKE_SOURCE_DIR}/android/package-sources
        BUILDTOOLS_REVISION ${QT_ANDROID_SDK_BUILDTOOLS_REVISION}
        DEPENDS ssl crypto ${SHARED_LIBOMP}
        PLUGINS ${CMAKE_SOURCE_DIR}/resources
        KEYSTORE ${KEYSTORE_FILE} ${KEYSTORE_ALIAS}
        KEYSTORE_PASSWORD ${KEYSTORE_PASSWORD}
    )
else()
    set_target_properties(osmin PROPERTIES OUTPUT_NAME "${EXEC}")
    install(TARGETS osmin RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
    file(COPY "resources" DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
    install(
        DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/resources
        DESTINATION ${CMAKE_INSTALL_DATADIR}/${EXEC}
    )
endif()

add_custom_target(
    "run"
    QML2_IMPORT_PATH=${CMAKE_BINARY_DIR}/backend/qml ${CMAKE_BINARY_DIR}/${EXEC} --debug
    DEPENDS osmin
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake IMMEDIATE @ONLY
)

add_custom_target(
    "uninstall"
    ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake
)
