# SPDX-FileCopyrightText: none
# SPDX-License-Identifier: BSD-3-Clause
set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

add_executable(resourcemanagement_gui resourcemanagment_gui.cpp)
target_link_libraries(resourcemanagement_gui
  Qt::Core
  Qt::Gui
  KPim${KF_MAJOR_VERSION}::Mime
  KPim${KF_MAJOR_VERSION}::IncidenceEditor
  KPim${KF_MAJOR_VERSION}::Libkdepim
  KPim${KF_MAJOR_VERSION}::Ldap
)

add_executable(individualmaildialog_test individualmaildialog_test.cpp)
target_link_libraries(individualmaildialog_test
    KPim${KF_MAJOR_VERSION}::IncidenceEditor
)
