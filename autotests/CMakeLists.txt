# SPDX-FileCopyrightText: none
# SPDX-License-Identifier: BSD-3-Clause
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

include_directories(${CMAKE_SOURCE_DIR}/src)

set(grant_lib KGantt6)

macro(IE_UNIT_TESTS)
    foreach(_testname ${ARGN})
        add_executable(${_testname} ${_testname}.cpp ${_testname}.h)
        add_test(NAME ${_testname} COMMAND ${_testname})
        ecm_mark_as_test(${_testname})
        target_link_libraries(
            ${_testname}
            Qt::Test
            KPim6::AkonadiCore
            ${grant_lib}
            KPim6::CalendarUtils
            KF6::CalendarCore
            KPim6::IncidenceEditor
            KPim6::Libkdepim
        )
    endforeach()
endmacro()

macro(IE_EXECUTABLE_TESTS)
    foreach(_testname ${ARGN})
        add_executable(${_testname} NOGUI TEST ${_testname}.cpp ${_testname}.h)
        target_link_libraries(
            ${_testname}
            KF6::CalendarCore
            Qt::Test
            KPim6::IncidenceEditor
            Qt::Widgets
        )
    endforeach()
endmacro()

ie_unit_tests(
  conflictresolvertest
  testfreebusyganttproxymodel
)

########### KTimeZoneComboBox unit test #############
add_executable(
    ktimezonecomboboxtest
    ktimezonecomboboxtest.cpp
    ktimezonecomboboxtest.h
)
add_test(NAME ktimezonecomboboxtest COMMAND ktimezonecomboboxtest)
ecm_mark_as_test(ktimezonecomboboxtest)

target_link_libraries(
    ktimezonecomboboxtest
    KF6::CalendarCore
    Qt::Test
    KPim6::IncidenceEditor
    KF6::Completion
)

add_executable(testindividualmaildialog testindividualmaildialog.cpp)
ecm_mark_nongui_executable(testindividualmaildialog)
add_test(NAME testindividualmaildialog COMMAND testindividualmaildialog)
ecm_mark_as_test(testindividualmaildialog)
target_link_libraries(
    testindividualmaildialog
    KF6::CalendarCore
    Qt::Test
    Qt::Widgets
    KF6::WidgetsAddons
    KF6::I18n
    KPim6::IncidenceEditor
)

# The IncidenceCategories subeditor of the dialog starts Akonadi running, so
# set up an environment.
add_akonadi_isolated_test(
  SOURCE incidencedatetimetest.cpp
  LINK_LIBRARIES Qt::Test
  Qt::Widgets
  KPim6::AkonadiWidgets
  KF6::Completion
  KPim6::IncidenceEditor
  KPim6::PimTextEdit
  KPim6::Libkdepim
  KF6::WidgetsAddons
)
