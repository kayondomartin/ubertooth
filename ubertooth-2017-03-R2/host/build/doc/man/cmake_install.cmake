# Install script for directory: /home/mwnl/JWHUR/ubertooth/ubertooth-2017-03-R2/host/doc/man

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

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "doc")
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/man/man1" TYPE FILE FILES
    "/home/mwnl/JWHUR/ubertooth/ubertooth-2017-03-R2/host/doc/man/ubertooth-btle.1"
    "/home/mwnl/JWHUR/ubertooth/ubertooth-2017-03-R2/host/doc/man/ubertooth-dump.1"
    "/home/mwnl/JWHUR/ubertooth/ubertooth-2017-03-R2/host/doc/man/ubertooth-rx.1"
    "/home/mwnl/JWHUR/ubertooth/ubertooth-2017-03-R2/host/doc/man/ubertooth-specan.1"
    "/home/mwnl/JWHUR/ubertooth/ubertooth-2017-03-R2/host/doc/man/ubertooth-afh.1"
    "/home/mwnl/JWHUR/ubertooth/ubertooth-2017-03-R2/host/doc/man/ubertooth-dfu.1"
    "/home/mwnl/JWHUR/ubertooth/ubertooth-2017-03-R2/host/doc/man/ubertooth-ego.1"
    "/home/mwnl/JWHUR/ubertooth/ubertooth-2017-03-R2/host/doc/man/ubertooth-scan.1"
    "/home/mwnl/JWHUR/ubertooth/ubertooth-2017-03-R2/host/doc/man/ubertooth-util.1"
    )
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "doc")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "doc")
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/man/man7" TYPE FILE FILES "/home/mwnl/JWHUR/ubertooth/ubertooth-2017-03-R2/host/doc/man/ubertooth.7")
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "doc")

