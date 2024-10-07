cmake_minimum_required(VERSION 3.8.2)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

project(osmin C CXX)

option(INSTALL_TESTS "Install the tests on make install" OFF)
option(BUILD_DEVICE_MOBILE "Enable behaviors for mobile devices" OFF)
option(BUILD_SIMULATOR "Build the simulation tool for OSMIN" OFF)

set(APP_VERSION       "1.13.2")
set(APP_VERSION_CODE  82)
set(APP_ID            "io.github.janbar.osmin")
set(APP_NAME          "osmin")

set(EXEC              "osmin")
set(ICON_NAME         "${EXEC}")
set(APP_DESKTOP_ID    "${EXEC}")
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

install(FILES "gui/icons/${ICON_NAME}-8x8.png"
  DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/8x8/apps RENAME "${ICON_NAME}.png")
install(FILES "gui/icons/${ICON_NAME}-16x16.png"
  DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/16x16/apps RENAME "${ICON_NAME}.png")
install(FILES "gui/icons/${ICON_NAME}-22x22.png"
  DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/22x22/apps RENAME "${ICON_NAME}.png")
install(FILES "gui/icons/${ICON_NAME}-24x24.png"
  DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/24x24/apps RENAME "${ICON_NAME}.png")
install(FILES "gui/icons/${ICON_NAME}-32x32.png"
  DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/32x32/apps RENAME "${ICON_NAME}.png")
install(FILES "gui/icons/${ICON_NAME}-36x36.png"
  DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/36x36/apps RENAME "${ICON_NAME}.png")
install(FILES "gui/icons/${ICON_NAME}-42x42.png"
  DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/42x42/apps RENAME "${ICON_NAME}.png")
install(FILES "gui/icons/${ICON_NAME}-48x48.png"
  DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/48x48/apps RENAME "${ICON_NAME}.png")
install(FILES "gui/icons/${ICON_NAME}-64x64.png"
  DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/64x64/apps RENAME "${ICON_NAME}.png")
install(FILES "gui/icons/${ICON_NAME}-72x72.png"
  DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/72x72/apps RENAME "${ICON_NAME}.png")
install(FILES "gui/icons/${ICON_NAME}-80x80.png"
  DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/80x80/apps RENAME "${ICON_NAME}.png")
install(FILES "gui/icons/${ICON_NAME}-96x96.png"
  DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/96x96/apps RENAME "${ICON_NAME}.png")
install(FILES "gui/icons/${ICON_NAME}-128x128.png"
  DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/128x128/apps RENAME "${ICON_NAME}.png")
install(FILES "gui/icons/${ICON_NAME}-192x192.png"
  DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/192x192/apps RENAME "${ICON_NAME}.png")
install(FILES "gui/icons/${ICON_NAME}-256x256.png"
  DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/256x256/apps RENAME "${ICON_NAME}.png")
install(FILES "gui/icons/${ICON_NAME}-512x512.png"
  DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/512x512/apps RENAME "${ICON_NAME}.png")

configure_file(
    ${APPDATA_FILE_NAME}.in
    ${CMAKE_CURRENT_BINARY_DIR}/${APPDATA_FILE_NAME}
)

if(NOT APPLE)
    # Provides OpenMP
    find_package(OpenMP REQUIRED)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
else()
    # force build all backend dependencies on apple
    set(BUILD_DEPENDENCIES ON CACHE BOOL "Build dependencies" FORCE)
    # ignore macports
    set(CMAKE_IGNORE_PATH "/opt/local/bin;/opt/local/include;/opt/local/lib")
endif()

set(QT_VERSION_PREFERRED "5" CACHE STRING "preferred Qt version" FORCE)
include(qtchooser)
if(NOT QT_FOUND)
    message(FATAL_ERROR "Qt${QT_VERSION_PREFERRED} NOT found")
elseif(Qt5Core_VERSION VERSION_LESS 5.15.2)
    message(FATAL_ERROR "Qt version not supported")
endif()

add_subdirectory(backend)

set(CMAKE_AUTOMOC ON)
find_package(Qt5 ${QT_VERSION} COMPONENTS
    Core Gui Qml Quick QuickControls2 Widgets Xml Svg Network Sensors
    Multimedia RemoteObjects Positioning REQUIRED)

if(ANDROID)
    find_package(Qt5AndroidExtras REQUIRED)
elseif(UNIX AND NOT APPLE)
    find_library(DBUS_LIBRARIES dbus-1)
    if(DBUS_LIBRARIES)
        find_package(Qt5 ${QT_VERSION} COMPONENTS DBus REQUIRED)
        set(HAVE_DBUS 1)
    endif()
endif()

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
    src/service.cpp
    src/servicefrontend.cpp
    src/remoteservice.cpp
    src/remotecompass.cpp
    src/remotepositionsource.cpp
    src/remotetracker.cpp
    src/mapextras.cpp
)
set(osmin_HEADERS src/converter.h src/platformextras.h
    src/locked.h
    src/utils.h
    src/csvparser.h
    src/qmlfilterbehavior.h
    src/qmlsortbehavior.h
    src/qmlsortfiltermodel.h
    src/favoritesmodel.h
    src/gpxfilemodel.h
    src/gpxlistmodel.h
    src/tracker.h
    src/compass/genericcompass.h
    src/compass/plugin.h
    src/service.h
    src/servicefrontend.h
    src/remote.h
    src/remoteservice.h
    src/remotecompass.h
    src/remotepositionsource.h
    src/remotetracker.h
    src/mapextras.h
)

if(UNIX)
    list(APPEND osmin_SOURCES src/signalhandler.cpp)
    list(APPEND osmin_HEADERS src/signalhandler.h)
endif()

add_definitions(-DAPP_VERSION="${APP_VERSION}")
add_definitions(-DQT_STATICPLUGIN)

include_directories(${PROJECT_BINARY_DIR} ${LIBOSMSCOUT_INCLUDE_DIRS})

qt5_generate_repc(osmin_SOURCES src/servicemessenger.rep SOURCE)
qt5_generate_repc(osmin_SOURCES src/servicemessenger.rep REPLICA)
qt5_add_resources(osmin-resources gui/osmin_controls2_515.qrc)

if(ANDROID)
    add_library(osmin SHARED ${osmin_SOURCES} ${osmin_HEADERS} ${osmin-resources})
else()
    add_executable(osmin ${osmin_SOURCES} ${osmin_HEADERS} ${osmin-resources})
endif()

target_link_libraries(osmin PRIVATE ${LIBOSMSCOUT_LIBRARIES})
target_link_libraries(osmin PRIVATE
                      Qt5::QuickControls2 Qt5::RemoteObjects Qt5::Positioning
                      Qt5::Qml Qt5::Quick Qt5::Widgets Qt5::Gui Qt5::Xml Qt5::Svg
                      Qt5::Network Qt5::Sensors Qt5::Multimedia Qt5::Core)

if(ANDROID)
    add_definitions(-DDEVICE_MOBILE)
    add_definitions(-frtti) # dynamic_cast: https://github.com/android/ndk/issues/519
    target_link_libraries(osmin PRIVATE Qt5::AndroidExtras android log)
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
        PACKAGE_SOURCES ${PROJECT_SOURCE_DIR}/android/package-sources
        DEPENDS ssl crypto ${SHARED_LIBOMP}
        ASSETS ${PROJECT_SOURCE_DIR}/resources
        KEYSTORE ${KEYSTORE_FILE} ${KEYSTORE_ALIAS}
        KEYSTORE_PASSWORD ${KEYSTORE_PASSWORD}
    )
else()
    if(BUILD_DEVICE_MOBILE)
        message(STATUS "Enable Device Mobile")
        add_definitions(-DDEVICE_MOBILE)
    endif()
    if(HAVE_DBUS)
        message(STATUS "Enable DBus interfaces")
        add_definitions(-DHAVE_DBUS)
        target_link_libraries(osmin PRIVATE Qt5::DBus)
    endif()

    set_target_properties(osmin PROPERTIES OUTPUT_NAME "${EXEC}")

    if(UNIX AND BUILD_SIMULATOR)
      add_subdirectory(simulator)
    endif()

    install(TARGETS osmin RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

     # set build commands to clean staged assets
    list(APPEND OSMIN_ASSETS_COMMANDS
        COMMAND ${CMAKE_COMMAND} -E
        remove_directory "${CMAKE_CURRENT_BINARY_DIR}/resources")
    list(APPEND OSMIN_ASSETS_COMMANDS
        COMMAND ${CMAKE_COMMAND} -E
        make_directory "${CMAKE_CURRENT_BINARY_DIR}/resources")
    list(APPEND OSMIN_ASSETS_COMMANDS
        COMMAND ${CMAKE_COMMAND} -E
        copy_directory "${PROJECT_SOURCE_DIR}/resources"
                       "${CMAKE_CURRENT_BINARY_DIR}/resources")
    add_custom_target(osmin_assets ALL ${OSMIN_ASSETS_COMMANDS})
    add_dependencies(osmin osmin_assets)

    install(
        DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/resources
        DESTINATION ${CMAKE_INSTALL_DATADIR}/${EXEC}
    )
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
endif()
