cmake_minimum_required(VERSION 3.8.2)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(MSVC)
  set(MSVC_DYNAMIC_RUNTIME ON)
endif()

option(BUILD_DEPENDENCIES "Build dependencies from sources" OFF)

find_package(Qt5Core REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Qml REQUIRED)
find_package(Qt5Svg REQUIRED)
find_package(Qt5Network REQUIRED)


set(QML_IMPORT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/qml)
set(QML_IMPORT_PATH ${QML_IMPORT_DIRECTORY} CACHE STRING "QML Import path" FORCE)

message(STATUS "Found Qt5 version ${Qt5Core_VERSION}")

include(DownloadProject)

# Provides dependency zlib
if(NOT BUILD_DEPENDENCIES)
    find_package(ZLIB QUIET)
endif()
if(NOT ZLIB_FOUND)
    download_project(PROJ zlib
        URL "${CMAKE_CURRENT_SOURCE_DIR}/zlib-1.2.11.tar.gz"
        URL_HASH MD5=d8c654d5d16f31de88db6b8c6a1bdc48
        PREFIX ${CMAKE_CURRENT_BINARY_DIR}
        UPDATE_DISCONNECTED ON)
    set(BUILD_SHARED_LIBS OFF)
    set(SKIP_INSTALL_ALL ON)
    add_subdirectory(
        ${CMAKE_CURRENT_BINARY_DIR}/zlib-src
        ${CMAKE_CURRENT_BINARY_DIR}/zlib-build
        EXCLUDE_FROM_ALL
    )
    set(ZLIB_LIBRARIES zlib)
    set(ZLIB_INCLUDE_DIRS "${zlib_SOURCE_DIR}" "${zlib_BINARY_DIR}")
    set(ZLIB_FOUND ON)
    message(STATUS "Build ZLIB: ${zlib_BINARY_DIR}")
endif()

# Provides dependency openssl
if(NOT BUILD_DEPENDENCIES)
    find_package(OpenSSL QUIET)
endif()
if(NOT OPENSSL_FOUND)
    if(Qt5Core_VERSION VERSION_LESS 5.12.4)
        set(OPENSSL_DL_ABI "1.0")
    endif()
    if(OPENSSL_DL_ABI STREQUAL "1.0")
        download_project(PROJ "openssl-${OPENSSL_DL_ABI}"
            GIT_REPOSITORY https://github.com/janbar/openssl-cmake.git
            GIT_TAG 1.0.2p-20220130
            GIT_SHALLOW 1
            PREFIX ${CMAKE_CURRENT_BINARY_DIR}
            UPDATE_DISCONNECTED ON)
    else()
        set(OPENSSL_DL_ABI "1.1")
        download_project(PROJ "openssl-${OPENSSL_DL_ABI}"
            GIT_REPOSITORY https://github.com/janbar/openssl-cmake.git
            GIT_TAG 1.1.1s-20221201
            GIT_SHALLOW 1
            PREFIX ${CMAKE_CURRENT_BINARY_DIR}
            UPDATE_DISCONNECTED ON)
    endif()
    if(ANDROID OR MSVC)
        set(BUILD_SHARED_LIBS ON)
    else()
        set(BUILD_SHARED_LIBS OFF)
    endif()
    add_subdirectory(
        ${CMAKE_CURRENT_BINARY_DIR}/openssl-${OPENSSL_DL_ABI}-src
        ${CMAKE_CURRENT_BINARY_DIR}/openssl-${OPENSSL_DL_ABI}-build
        EXCLUDE_FROM_ALL
    )
    set(OPENSSL_SSL_LIBRARY ssl PARENT_SCOPE)
    set(OPENSSL_CRYPTO_LIBRARY crypto PARENT_SCOPE)
    set(OPENSSL_INCLUDE_DIR "${openssl_BINARY_DIR}/include" "${openssl_BINARY_DIR}" PARENT_SCOPE)
    set(OPENSSL_FOUND ON PARENT_SCOPE)
    message(STATUS "Build OpenSSL: ${openssl_BINARY_DIR}")
endif()

# Provides internal iconv
if(NOT ICONV_FOUND)
    download_project(PROJ iconv
        URL "${CMAKE_CURRENT_SOURCE_DIR}/libiconv-1.16.tar.gz"
        URL_HASH MD5=739c5e8f9e29fb95c0a1ad67becb80a7
        PREFIX ${CMAKE_CURRENT_BINARY_DIR}
        UPDATE_DISCONNECTED ON)
    set(BUILD_SHARED_LIBS OFF)
    set(SKIP_INSTALL_ALL ON)
    add_subdirectory(
        ${CMAKE_CURRENT_BINARY_DIR}/iconv-src
        ${CMAKE_CURRENT_BINARY_DIR}/iconv-build
        EXCLUDE_FROM_ALL
    )
    set(ICONV_LIBRARIES iconv charset)
    set(ICONV_INCLUDE_DIRS "${iconv_BINARY_DIR}/include")
    set(ICONV_FOUND ON)
    message(STATUS "Build iconv: ${iconv_BINARY_DIR}")
endif()

# Provides internal marisa-trie
if(NOT MARISA_FOUND)
    download_project(PROJ marisa
        URL "${CMAKE_CURRENT_SOURCE_DIR}/marisa-0.2.5.tar.gz"
        URL_HASH MD5=036ab06e7edf0059d602264cb6b26aca
        PREFIX ${CMAKE_CURRENT_BINARY_DIR}
        UPDATE_DISCONNECTED ON)
    set(BUILD_SHARED_LIBS OFF)
    set(SKIP_INSTALL_ALL ON)
    add_subdirectory(
        ${CMAKE_CURRENT_BINARY_DIR}/marisa-src
        ${CMAKE_CURRENT_BINARY_DIR}/marisa-build
        EXCLUDE_FROM_ALL
    )
    set(MARISA_LIBRARIES marisa)
    set(MARISA_INCLUDE_DIRS "${marisa_BINARY_DIR}")
    set(MARISA_FOUND ON)
    message(STATUS "Build MARISA: ${marisa_BINARY_DIR}")
endif()

# Provides dependency xml2
if(NOT BUILD_DEPENDENCIES)
    find_package(LibXml2 QUIET)
endif()
if(NOT LibXml2_FOUND)
    download_project(PROJ xml2
        URL "${CMAKE_CURRENT_SOURCE_DIR}/libxml2-2.9.9.tar.gz"
        URL_HASH MD5=df97dd45b18cfd6cc75633172d507f5e
        PREFIX ${CMAKE_CURRENT_BINARY_DIR}
        UPDATE_DISCONNECTED ON)
    set(BUILD_SHARED_LIBS OFF)
    set(SKIP_INSTALL_ALL ON)
    set(LIBXML2_WITH_ZLIB ON)
    set(LIBXML2_WITH_LZMA OFF)
    set(LIBXML2_WITH_ICONV ON)
    set(LIBXML2_WITH_ICU OFF)
    set(LIBXML2_WITH_THREADS OFF)
    add_subdirectory(
        ${CMAKE_CURRENT_BINARY_DIR}/xml2-src
        ${CMAKE_CURRENT_BINARY_DIR}/xml2-build
        EXCLUDE_FROM_ALL
    )
    set(LIBXML2_LIBRARY xml2)
    set(LIBXML2_INCLUDE_DIR "${xml2_BINARY_DIR}")
    set(LibXml2_FOUND ON)
    set(PC_LIBXML_STATIC_LIBRARIES ${ZLIB_LIRARIES} ${ICONV_LIBRARIES})
    message(STATUS "Build xml2: ${xml2_BINARY_DIR}")
endif()

# Provides dependency libosmscout
set(libosmscout_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libosmscout")
set(libosmscout_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/libosmscout")
execute_process(
    COMMAND git apply ${CMAKE_CURRENT_SOURCE_DIR}/libosmscout.1.patch
    WORKING_DIRECTORY ${libosmscout_SOURCE_DIR})
execute_process(
    COMMAND git apply ${CMAKE_CURRENT_SOURCE_DIR}/libosmscout.2.patch
    WORKING_DIRECTORY ${libosmscout_SOURCE_DIR})

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
set(OSMSCOUT_BUILD_WEBPAGE          OFF CACHE BOOL "" FORCE)
add_subdirectory(libosmscout EXCLUDE_FROM_ALL)

set(LIBOSMSCOUT_LIBRARIES
    OSMScout OSMScoutGPX OSMScoutMap OSMScoutMapQt OSMScoutClientQt
    PARENT_SCOPE
)

set(LIBOSMSCOUT_INCLUDE_DIRS
    "${libosmscout_BINARY_DIR}/libosmscout/privateinclude"
    "${libosmscout_BINARY_DIR}/libosmscout-client-qt/include"
    "${libosmscout_BINARY_DIR}/libosmscout-gpx/include"
    "${libosmscout_BINARY_DIR}/libosmscout/include"
    "${libosmscout_BINARY_DIR}/libosmscout-map/include"
    "${libosmscout_BINARY_DIR}/libosmscout-map-qt/include"
    "${libosmscout_SOURCE_DIR}/libosmscout-client-qt/include"
    "${libosmscout_SOURCE_DIR}/libosmscout-map-qt/include"
    "${libosmscout_SOURCE_DIR}/libosmscout-map/include"
    "${libosmscout_SOURCE_DIR}/libosmscout/include"
    "${MARISA_INCLUDE_DIRS}"
    PARENT_SCOPE
)
