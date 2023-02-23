# SPDX-FileCopyrightText: none
# SPDX-License-Identifier: BSD-3-Clause
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

include_directories(${CMAKE_SOURCE_DIR}/src)

set(grant_lib KGantt)

macro(IE_UNIT_TESTS)
  foreach(_testname ${ARGN})
    add_executable(${_testname} ${_testname}.cpp ${_testname}.h)
    add_test(NAME ${_testname} COMMAND ${_testname})
    ecm_mark_as_test(${_testname})
    target_link_libraries(${_testname}
      Qt::Test
      KF5::AkonadiCore
      ${grant_lib}
      KPim${KF_MAJOR_VERSION}::CalendarUtils
      KF${KF_MAJOR_VERSION}::CalendarCore
      KPim${KF_MAJOR_VERSION}::IncidenceEditor
      KPim${KF_MAJOR_VERSION}::Libkdepim
    )
  endforeach()
endmacro()

macro(IE_EXECUTABLE_TESTS)
  foreach(_testname ${ARGN})
    add_executable(${_testname} NOGUI TEST ${_testname}.cpp ${_testname}.h)
    target_link_libraries(${_testname} KF${KF_MAJOR_VERSION}::CalendarCore Qt::Test KF5::IncidenceEditor Qt::Widgets)
  endforeach()
endmacro()

ie_unit_tests(
  conflictresolvertest
  testfreebusyganttproxymodel
)

########### KTimeZoneComboBox unit test #############
add_executable(ktimezonecomboboxtest ktimezonecomboboxtest.cpp ktimezonecomboboxtest.h)
add_test(NAME ktimezonecomboboxtest COMMAND ktimezonecomboboxtest)
ecm_mark_as_test(ktimezonecomboboxtest)

target_link_libraries(ktimezonecomboboxtest
  KF${KF_MAJOR_VERSION}::CalendarCore
  Qt::Test
  KPim${KF_MAJOR_VERSION}::IncidenceEditor
  KF${KF_MAJOR_VERSION}::Completion
)

add_executable(testindividualmaildialog testindividualmaildialog.cpp)
ecm_mark_nongui_executable(testindividualmaildialog)
add_test(NAME testindividualmaildialog COMMAND testindividualmaildialog)
ecm_mark_as_test(testindividualmaildialog)
target_link_libraries(testindividualmaildialog
  KF${KF_MAJOR_VERSION}::CalendarCore
  Qt::Test
  Qt::Widgets
  KF${KF_MAJOR_VERSION}::WidgetsAddons
  KF${KF_MAJOR_VERSION}::I18n
  KPim${KF_MAJOR_VERSION}::IncidenceEditor
)

# The IncidenceCategories subeditor of the dialog starts Akonadi running, so
# set up an environment.
add_akonadi_isolated_test(
  SOURCE incidencedatetimetest.cpp
  LINK_LIBRARIES Qt::Test
  Qt::Widgets
  KF5::AkonadiWidgets
  KF${KF_MAJOR_VERSION}::Completion
  KPim${KF_MAJOR_VERSION}::IncidenceEditor
  KPim${KF_MAJOR_VERSION}::PimTextEdit
  KPim${KF_MAJOR_VERSION}::Libkdepim
  KF${KF_MAJOR_VERSION}::WidgetsAddons
)
