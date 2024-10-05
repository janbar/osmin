cmake_minimum_required(VERSION 3.8.2)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

unset(CMAKE_IGNORE_PATH)

set(OSMIN_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../src)

include_directories(BEFORE SYSTEM
  ${CMAKE_CURRENT_BINARY_DIR}
  ${OSMIN_SRC_DIR}
  ${LIBOSMSCOUT_INCLUDE_DIRS}
)

option(BUILD_SIMULATOR_WITH_READLINE "Enable Readline support" ON)

if(BUILD_SIMULATOR_WITH_READLINE)
  find_package(Readline QUIET)
  if(READLINE_FOUND)
    include_directories(${Readline_INCLUDE_DIR})
    add_definitions("-DHAVE_READLINE")
  endif()
endif()

set(simulator_SOURCES main.cpp
    commandline.cpp
    globalazimuth.cpp
    globalposition.cpp
    gpxrunner.cpp
    scriptrunner.cpp
    simulatedcompass.cpp
    simulatedpositionsource.cpp
    simulatedsensorplugin.cpp
    simulator.cpp
    ${OSMIN_SRC_DIR}/converter.cpp
    ${OSMIN_SRC_DIR}/csvparser.cpp
    ${OSMIN_SRC_DIR}/gpxfilemodel.cpp
    ${OSMIN_SRC_DIR}/tracker.cpp
    ${OSMIN_SRC_DIR}/service.cpp
    ${OSMIN_SRC_DIR}/signalhandler.cpp
    ${OSMIN_SRC_DIR}/utils.cpp
    ${OSMIN_SRC_DIR}/compass/genericcompass.cpp
    ${OSMIN_SRC_DIR}/compass/plugin.cpp
)
set(simulator_HEADERS
    commandline.h
    globalazimuth.h
    globalposition.h
    gpxrunner.h
    scriptrunner.h
    simulatedcompass.h
    simulatedpositionsource.h
    simulatedsensorplugin.h
    simulator.h
    simulatorbreaker.h
    ${OSMIN_SRC_DIR}/converter.h
    ${OSMIN_SRC_DIR}/csvparser.h
    ${OSMIN_SRC_DIR}/gpxfilemodel.h
    ${OSMIN_SRC_DIR}/tracker.h
    ${OSMIN_SRC_DIR}/service.h
    ${OSMIN_SRC_DIR}/signalhandler.h
    ${OSMIN_SRC_DIR}/utils.h
)

qt5_generate_repc(simulator_SOURCES
                  ${OSMIN_SRC_DIR}/servicemessenger.rep SOURCE)
qt5_generate_repc(simulator_SOURCES
                  ${OSMIN_SRC_DIR}/servicemessenger.rep REPLICA)

add_executable(simulator ${simulator_SOURCES} ${simulator_HEADERS})

target_link_libraries(simulator PRIVATE
  ${LIBOSMSCOUT_LIBRARIES}
  Qt5::RemoteObjects Qt5::Positioning
  Qt5::Sensors Qt5::Core
)

if(READLINE_FOUND)
  target_link_libraries(simulator PRIVATE ${Readline_LIBRARY})
endif()

set_target_properties(simulator PROPERTIES OUTPUT_NAME "osmin-simulator")
