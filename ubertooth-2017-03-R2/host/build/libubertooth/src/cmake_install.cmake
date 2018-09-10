# Install script for directory: /home/mwnl/JWHUR/ubertooth/ubertooth-2017-03-R2/host/libubertooth/src

# Set the install prefix
IF(NOT DEFINED CMAKE_INSTALL_PREFIX)
  SET(CMAKE_INSTALL_PREFIX "/usr/local")
ENDIF(NOT DEFINED CMAKE_INSTALL_PREFIX)
STRING(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
IF(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  IF(BUILD_TYPE)
    STRING(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  ELSE(BUILD_TYPE)
    SET(CMAKE_INSTALL_CONFIG_NAME "")
  ENDIF(BUILD_TYPE)
  MESSAGE(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
ENDIF(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)

# Set the component getting installed.
IF(NOT CMAKE_INSTALL_COMPONENT)
  IF(COMPONENT)
    MESSAGE(STATUS "Install component: \"${COMPONENT}\"")
    SET(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  ELSE(COMPONENT)
    SET(CMAKE_INSTALL_COMPONENT)
  ENDIF(COMPONENT)
ENDIF(NOT CMAKE_INSTALL_COMPONENT)

# Install shared libraries without execute permission?
IF(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  SET(CMAKE_INSTALL_SO_NO_EXE "1")
ENDIF(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "sharedlibs")
  FOREACH(file
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libubertooth.so.1.0"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libubertooth.so.1"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libubertooth.so"
      )
    IF(EXISTS "${file}" AND
       NOT IS_SYMLINK "${file}")
      FILE(RPATH_CHECK
           FILE "${file}"
           RPATH "")
    ENDIF()
  ENDFOREACH()
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES
    "/home/mwnl/JWHUR/ubertooth/ubertooth-2017-03-R2/host/build/libubertooth/src/libubertooth.so.1.0"
    "/home/mwnl/JWHUR/ubertooth/ubertooth-2017-03-R2/host/build/libubertooth/src/libubertooth.so.1"
    "/home/mwnl/JWHUR/ubertooth/ubertooth-2017-03-R2/host/build/libubertooth/src/libubertooth.so"
    )
  FOREACH(file
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libubertooth.so.1.0"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libubertooth.so.1"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libubertooth.so"
      )
    IF(EXISTS "${file}" AND
       NOT IS_SYMLINK "${file}")
      FILE(RPATH_REMOVE
           FILE "${file}")
      IF(CMAKE_INSTALL_DO_STRIP)
        EXECUTE_PROCESS(COMMAND "/usr/bin/strip" "${file}")
      ENDIF(CMAKE_INSTALL_DO_STRIP)
    ENDIF()
  ENDFOREACH()
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "sharedlibs")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "headers")
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include" TYPE FILE FILES
    "/home/mwnl/JWHUR/ubertooth/ubertooth-2017-03-R2/host/libubertooth/src/ubertooth.h"
    "/home/mwnl/JWHUR/ubertooth/ubertooth-2017-03-R2/host/libubertooth/src/ubertooth_callback.h"
    "/home/mwnl/JWHUR/ubertooth/ubertooth-2017-03-R2/host/libubertooth/src/ubertooth_control.h"
    "/home/mwnl/JWHUR/ubertooth/ubertooth-2017-03-R2/host/libubertooth/src/ubertooth_fifo.h"
    "/home/mwnl/JWHUR/ubertooth/ubertooth-2017-03-R2/host/libubertooth/src/ubertooth_interface.h"
    "/home/mwnl/JWHUR/ubertooth/ubertooth-2017-03-R2/host/libubertooth/src/procData.h"
    "/home/mwnl/JWHUR/ubertooth/ubertooth-2017-03-R2/host/libubertooth/src/bchEcc.h"
    "/home/mwnl/JWHUR/ubertooth/ubertooth-2017-03-R2/host/libubertooth/src/aesAlg.h"
    )
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "headers")

