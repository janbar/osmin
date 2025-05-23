cmake_minimum_required(VERSION 3.20)

# Requires:
# QT_VERSION_MAJOR        Qt used major version (5)
# QT_VERSION              Qt version match (5.15)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(MSVC)
  set(MSVC_DYNAMIC_RUNTIME ON)
endif()

option(BUILD_DEPENDENCIES "Build dependencies from sources" OFF)

# Variable QT_FOUND has been set previously, so I have to load all Qt
# requirements for submodule libosmscout myself
if(QT_VERSION_MAJOR EQUAL 5)
    find_package(Qt5 ${QT_VERSION} COMPONENTS
        Core Gui Qml Widgets Svg Network Quick Multimedia
        REQUIRED)
elseif(QT_VERSION_MAJOR EQUAL 6)
    find_package(Qt6 ${QT_VERSION} COMPONENTS
        Core Gui Qml Widgets Svg Network Quick Multimedia
        Core5Compat REQUIRED)
endif()

set(QML_IMPORT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/qml)
set(QML_IMPORT_PATH ${QML_IMPORT_DIRECTORY} CACHE STRING "QML Import path" FORCE)

include(DownloadProject)

# Provides dependency zlib
if(NOT BUILD_DEPENDENCIES)
    find_package(ZLIB QUIET)
endif()
if(NOT ZLIB_FOUND)
    download_project(PROJ zlib
        URL "${CMAKE_CURRENT_SOURCE_DIR}/zlib-1.3.1.tar.gz"
        URL_HASH MD5=9855b6d802d7fe5b7bd5b196a2271655
        PREFIX ${CMAKE_CURRENT_BINARY_DIR}
        UPDATE_DISCONNECTED ON)
    add_subdirectory(
        ${CMAKE_CURRENT_BINARY_DIR}/zlib-src
        ${CMAKE_CURRENT_BINARY_DIR}/zlib-build
        EXCLUDE_FROM_ALL
    )
    set(ZLIB_LIBRARY zlibstatic)
    set(ZLIB_LIBRARIES ${ZLIB_LIBRARY})
    set(ZLIB_INCLUDE_DIR "${zlib_BINARY_DIR}")
    set(ZLIB_INCLUDE_DIRS "${zlib_SOURCE_DIR}" "${zlib_BINARY_DIR}")
    set(ZLIB_FOUND ON)
    export(TARGETS ${ZLIB_LIBRARY} FILE ZLIBTargets.cmake)
    add_library(ZLIB::ZLIB ALIAS ${ZLIB_LIBRARY})
    message(STATUS "Build ZLIB: ${zlib_BINARY_DIR}")
endif()

# Provides internal iconv
if(NOT ICONV_FOUND)
    download_project(PROJ iconv
        URL "${CMAKE_CURRENT_SOURCE_DIR}/libiconv-1.17.tar.gz"
        URL_HASH MD5=afc24d7f498b1ad19e10780a98467dc5
        PREFIX ${CMAKE_CURRENT_BINARY_DIR}
        UPDATE_DISCONNECTED ON)
    set(BUILD_SHARED_LIBS OFF)
    set(SKIP_INSTALL_ALL ON)
    add_subdirectory(
        ${CMAKE_CURRENT_BINARY_DIR}/iconv-src
        ${CMAKE_CURRENT_BINARY_DIR}/iconv-build
        EXCLUDE_FROM_ALL
    )
    set(Iconv_LIBRARY iconv)
    set(Iconv_LIBRARIES ${Iconv_LIBRARY})
    set(Iconv_INCLUDE_DIR "${iconv_BINARY_DIR}/include")
    set(Iconv_INCLUDE_DIRS "${iconv_BINARY_DIR}/include")
    set(Iconv_FOUND ON)
    export(TARGETS iconv FILE IconvTargets.cmake)
    add_library(Iconv::Iconv ALIAS iconv)
    message(STATUS "Build iconv: ${iconv_BINARY_DIR}")
endif()

# Provides internal marisa-trie
if(NOT MARISA_FOUND)
    download_project(PROJ marisa
        URL "${CMAKE_CURRENT_SOURCE_DIR}/marisa-0.2.6.tar.gz"
        URL_HASH MD5=caffa344057def4e5e81b6cff178e76f
        PREFIX ${CMAKE_CURRENT_BINARY_DIR}
        UPDATE_DISCONNECTED ON)
    set(BUILD_SHARED_LIBS OFF)
    set(SKIP_INSTALL_ALL ON)
    add_subdirectory(
        ${CMAKE_CURRENT_BINARY_DIR}/marisa-src
        ${CMAKE_CURRENT_BINARY_DIR}/marisa-build
        EXCLUDE_FROM_ALL
    )
    set(MARISA_LIBRARY marisa)
    set(MARISA_LIBRARIES marisa)
    set(MARISA_INCLUDE_DIR "${marisa_BINARY_DIR}")
    set(MARISA_INCLUDE_DIRS "${marisa_BINARY_DIR}")
    set(MARISA_FOUND ON)
    export(TARGETS marisa FILE MarisaTargets.cmake)
    add_library(Marisa::Marisa ALIAS marisa)
    message(STATUS "Build MARISA: ${marisa_BINARY_DIR}")
endif()

# Provides dependency xml2
if(NOT BUILD_DEPENDENCIES)
    find_package(LibXml2 QUIET)
endif()
if(NOT LibXml2_FOUND)
    download_project(PROJ xml2
        URL "${CMAKE_CURRENT_SOURCE_DIR}/libxml2-2.9.14.tar.gz"
        URL_HASH MD5=7eba7ae2403717d7d611b957b528eff7
        PREFIX ${CMAKE_CURRENT_BINARY_DIR}
        UPDATE_DISCONNECTED ON)
    set(BUILD_SHARED_LIBS OFF)
    set(LIBXML2_WITH_ZLIB ON)
    set(LIBXML2_WITH_LZMA OFF)
    set(LIBXML2_WITH_ICONV ON)
    set(LIBXML2_WITH_ICU OFF)
    set(LIBXML2_WITH_THREADS OFF)
    set(LIBXML2_WITH_PYTHON OFF)
    set(LIBXML2_WITH_HTTP OFF)
    set(LIBXML2_WITH_FTP OFF)
    set(LIBXML2_WITH_DEBUG OFF)
    set(LIBXML2_WITH_PROGRAMS OFF)
    set(LIBXML2_WITH_TESTS OFF)
    set(SKIP_INSTALL_ALL ON)
    add_subdirectory(
        ${CMAKE_CURRENT_BINARY_DIR}/xml2-src
        ${CMAKE_CURRENT_BINARY_DIR}/xml2-build
        EXCLUDE_FROM_ALL
    )
    set(LIBXML2_LIBRARY xml2)
    set(LIBXML2_LIBRARIES xml2)
    set(LIBXML2_INCLUDE_DIR "${xml2_BINARY_DIR}")
    set(LIBXML2_INCLUDE_DIRS "${xml2_BINARY_DIR}")
    set(LibXml2_FOUND ON)
    set(PC_LIBXML_STATIC_LIBRARIES ${ZLIB_LIRARIES} ${Iconv_LIBRARIES})
    add_library(LibXml2::LibXml2 ALIAS xml2)
    message(STATUS "Build xml2: ${xml2_BINARY_DIR}")
endif()

# Provides dependency libosmscout
set(libosmscout_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libosmscout")
set(libosmscout_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/libosmscout")

# First apply our patches in the source of lib
list(APPEND libosmscout_patches
)
foreach(patch IN LISTS libosmscout_patches)
    message(STATUS "Patch ${patch} : CHECK")
    execute_process(
        COMMAND git apply --check ${CMAKE_CURRENT_SOURCE_DIR}/${patch}
        WORKING_DIRECTORY ${libosmscout_SOURCE_DIR}
        RESULT_VARIABLE presult
        OUTPUT_QUIET ERROR_QUIET)
    if(presult)
        message(STATUS "Patch ${patch} : REVERT")
        execute_process(
            COMMAND git apply --reverse ${CMAKE_CURRENT_SOURCE_DIR}/${patch}
            WORKING_DIRECTORY ${libosmscout_SOURCE_DIR}
            RESULT_VARIABLE presult
            OUTPUT_QUIET ERROR_QUIET)
        if(presult)
            message(FATAL_ERROR "Patch ${patch} failed")
        endif()
    endif()
    message(STATUS "Patch ${patch} : APPLY")
    execute_process(
        COMMAND git apply ${CMAKE_CURRENT_SOURCE_DIR}/${patch}
        WORKING_DIRECTORY ${libosmscout_SOURCE_DIR})
endforeach()

set(BUILD_SHARED_LIBS               OFF CACHE BOOL "" FORCE)
set(OSMSCOUT_HAVE_LIB_MARISA        ON  CACHE BOOL "" FORCE)
if(ANDROID)
set(OSMSCOUT_ENABLE_SSE             OFF  CACHE BOOL "" FORCE)
else()
set(OSMSCOUT_ENABLE_SSE             ON  CACHE BOOL "" FORCE)
endif()
set(OSMSCOUT_BUILD_BINDING_CSHARP   OFF CACHE BOOL "" FORCE)
set(OSMSCOUT_BUILD_BINDING_JAVA     OFF CACHE BOOL "" FORCE)
set(OSMSCOUT_BUILD_CLIENT_QT        ON  CACHE BOOL "" FORCE)
set(OSMSCOUT_BUILD_DEMOS            OFF CACHE BOOL "" FORCE)
set(OSMSCOUT_BUILD_DOC_API          OFF CACHE BOOL "" FORCE)
set(OSMSCOUT_BUILD_EXTERN_MATLAB    OFF CACHE BOOL "" FORCE)
set(OSMSCOUT_BUILD_MAP_AGG          OFF CACHE BOOL "" FORCE)
set(OSMSCOUT_BUILD_MAP_CAIRO        OFF CACHE BOOL "" FORCE)
set(OSMSCOUT_BUILD_MAP_DIRECTX      OFF CACHE BOOL "" FORCE)
set(OSMSCOUT_BUILD_MAP_OPENGL       OFF CACHE BOOL "" FORCE)
set(OSMSCOUT_BUILD_MAP_QT           ON  CACHE BOOL "" FORCE)
set(OSMSCOUT_BUILD_MAP_SVG          OFF CACHE BOOL "" FORCE)
set(OSMSCOUT_BUILD_TESTS            OFF CACHE BOOL "" FORCE)
set(OSMSCOUT_BUILD_TOOL_DUMPDATA    OFF CACHE BOOL "" FORCE)
set(OSMSCOUT_BUILD_TOOL_IMPORT      ON  CACHE BOOL "" FORCE)
set(OSMSCOUT_BUILD_TOOL_OSMSCOUT2   OFF CACHE BOOL "" FORCE)
set(OSMSCOUT_BUILD_TOOL_OSMSCOUTOPENGL OFF CACHE BOOL "" FORCE)
set(OSMSCOUT_BUILD_TOOL_STYLEEDITOR ON  CACHE BOOL "" FORCE)
set(OSMSCOUT_BUILD_TOOL_PUBLICTRANSPORTMAP OFF CACHE BOOL "" FORCE)
set(OSMSCOUT_BUILD_WEBPAGE          OFF CACHE BOOL "" FORCE)
add_subdirectory(libosmscout EXCLUDE_FROM_ALL)

set(LIBOSMSCOUT_LIBRARIES
    OSMScout OSMScoutGPX OSMScoutMap OSMScoutMapQt OSMScoutClientQt
    PARENT_SCOPE
)

#"${libosmscout_BINARY_DIR}/libosmscout/privateinclude"
set(LIBOSMSCOUT_INCLUDE_DIRS
    "${libosmscout_BINARY_DIR}/libosmscout/include"
    "${libosmscout_BINARY_DIR}/libosmscout-client-qt/include"
    "${libosmscout_BINARY_DIR}/libosmscout-gpx/include"
    "${libosmscout_BINARY_DIR}/libosmscout-import/include"
    "${libosmscout_BINARY_DIR}/libosmscout-map/include"
    "${libosmscout_BINARY_DIR}/libosmscout-map-qt/include"
    "${libosmscout_SOURCE_DIR}/libosmscout/include"
    "${libosmscout_SOURCE_DIR}/libosmscout-client-qt/include"
    "${libosmscout_SOURCE_DIR}/libosmscout-gpx/include"
    "${libosmscout_SOURCE_DIR}/libosmscout-import/include"
    "${libosmscout_SOURCE_DIR}/libosmscout-map-qt/include"
    "${libosmscout_SOURCE_DIR}/libosmscout-map/include"
    "${MARISA_INCLUDE_DIRS}"
    PARENT_SCOPE
)
