# Copyright (C) 2015 SINTEF ICT,
# Applied Mathematics, Norway.
# 
# Contact information:
# E-mail: eivind.fonn@sintef.no
# SINTEF ICT, Department of Applied Mathematics,
# P.O. Box 4760 Sluppen,
# 7045 Trondheim, Norway.
# 
# This file is part of SplineView.
# 
# SplineView is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as
# published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.
# 
# SplineView is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU Affero General Public License for more details.
# 
# You should have received a copy of the GNU Affero General Public
# License along with SplineView. If not, see
# <http://www.gnu.org/licenses/>.
# 
# In accordance with Section 7(b) of the GNU Affero General Public
# License, a covered work must retain the producer line in every data
# file that is created or manipulated using SplineView.
# 
# Other Usage
# You can be released from the requirements of the license by purchasing
# a commercial license. Buying such a license is mandatory as soon as you
# develop commercial activities involving the SplineView library without
# disclosing the source code of your own applications.
# 
# This file may be used in accordance with the terms contained in a
# written agreement between you and SINTEF ICT.


cmake_minimum_required(VERSION 2.8.11)

project(SplineView CXX C)


# Custom modules

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")


# SplineView version

set(SV_VERSION_MAJOR 0)
set(SV_VERSION_MINOR 0)
set(SV_VERSION_PATCH 1)


# MOC is needed for Qt

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)


# We want C++11, because it's cool

include(TestCXXAcceptsFlag)
check_cxx_accepts_flag("-std=c++11" CXX_FLAG_CXX11)
if(CXX_FLAG_CXX11)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
else()
  check_cxx_accepts_flag("-std=gnu++0x" CXX_FLAG_GNU)
  if(CXX_FLAG_GNU)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++0x")
  else()
    message(FATAL_ERROR "C++11 support is required, bailing")
  endif()
endif()


# Include and build Ogre

find_package(OGRE REQUIRED)


# Find Qt5, GoTools and OpenGL

find_package(Qt5Core REQUIRED)
find_package(Qt5Widgets REQUIRED)


# Configure files

configure_file("${CMAKE_CURRENT_LIST_DIR}/src/main.h.in" "${PROJECT_BINARY_DIR}/main.h")
configure_file("${CMAKE_CURRENT_LIST_DIR}/ogre/plugins.cfg.in" "${PROJECT_BINARY_DIR}/ogre/plugins.cfg")


# Includes

include_directories(${PROJECT_BINARY_DIR})
include_directories(${OGRE_INCLUDE_DIRS})
include_directories(src)


# Executable SplineView

add_executable(SplineView
  src/main.cpp
  src/MainWindow.cpp
  src/QOgreWindow.cpp
  src/SdkQtCameraMan.h
  )

qt5_use_modules(SplineView Core Widgets)

target_link_libraries(SplineView
  ${OGRE_LIBRARIES}
  )


# Install

install(TARGETS SplineView RUNTIME DESTINATION bin)
