# Copyright (c) 2017, 2021, Oracle and/or its affiliates.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License, version 2.0,
# as published by the Free Software Foundation.
#
# This program is also distributed with certain software (including
# but not limited to OpenSSL) that is licensed under separate terms,
# as designated in a particular file or component or in included license
# documentation.  The authors of MySQL hereby grant you an additional
# permission to link the program and your derivative works with the
# separately licensed software that they have included with MySQL.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License, version 2.0, for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA

DISABLE_MISSING_PROFILE_WARNING()

FIND_LIBRARY(MICRO_HTTPD_LIB
  NAMES libmicrohttpd_d libmicrohttpd libmicrohttpd-dll_d libmicrohttpd-dll
   PATHS ${WITH_MICROHTTPD}
   NO_DEFAULT_PATH
   NO_CMAKE_PATH)

FIND_PATH(MICRO_HTTPD_INCLUDE
  NAMES microhttpd.h
  HINTS ${WITH_MICROHTTPD})

MYSQL_ADD_COMPONENT(httpd_table
  httpd_table.cc
  MODULE_ONLY
  TEST_ONLY
  )

TARGET_INCLUDE_DIRECTORIES(component_httpd_table PRIVATE ${MICRO_HTTPD_INCLUDE})
TARGET_LINK_LIBRARIES(component_httpd_table PRIVATE ${MICRO_HTTPD_LIB})

