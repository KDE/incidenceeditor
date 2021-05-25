set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

include_directories(${CMAKE_SOURCE_DIR}/src)

set(grant_lib KGantt)

macro(IE_UNIT_TESTS)
  foreach(_testname ${ARGN})
    add_executable(${_testname} ${_testname}.cpp)
    add_test(NAME ${_testname} COMMAND ${_testname})
    ecm_mark_as_test(${_testname})
    target_link_libraries(${_testname}
      Qt::Test
      KF5::AkonadiCore
      ${grant_lib}
      KF5::CalendarUtils
      KF5::CalendarCore
      KF5::IncidenceEditor
      KF5::Libkdepim
    )
  endforeach()
endmacro()

macro(IE_EXECUTABLE_TESTS)
  foreach(_testname ${ARGN})
    add_executable(${_testname} NOGUI TEST ${_testname}.cpp)
    target_link_libraries(${_testname} KF5::CalendarCore Qt::Test KF5::IncidenceEditor Qt::Widgets)
  endforeach()
endmacro()

ie_unit_tests(
  conflictresolvertest
  testfreebusyganttproxymodel
)

########### KTimeZoneComboBox unit test #############
add_executable(ktimezonecomboboxtest ktimezonecomboboxtest.cpp)
add_test(NAME ktimezonecomboboxtest COMMAND ktimezonecomboboxtest)
ecm_mark_as_test(ktimezonecomboboxtest)

target_link_libraries(ktimezonecomboboxtest
  KF5::CalendarCore
  Qt::Test
  KF5::IncidenceEditor
  KF5::Completion
)

add_executable(testindividualmaildialog testindividualmaildialog.cpp)
ecm_mark_nongui_executable(testindividualmaildialog)
add_test(NAME testindividualmaildialog COMMAND testindividualmaildialog)
ecm_mark_as_test(testindividualmaildialog)
target_link_libraries(testindividualmaildialog
  KF5::CalendarCore
  Qt::Test
  Qt::Widgets
  KF5::WidgetsAddons
  KF5::I18n
  KF5::IncidenceEditor
)

if (KDEPIM_RUN_AKONADI_TEST)
# The IncidenceCategories subeditor of the dialog starts Akonadi running, so
# set up an environment.
set(AKONADI_RUN_MYSQL_ISOLATED_TESTS FALSE)
add_akonadi_isolated_test(
  SOURCE incidencedatetimetest.cpp
  LINK_LIBRARIES Qt::Test
  Qt::Widgets
  KF5::AkonadiWidgets
  KF5::Completion
  KF5::IncidenceEditor
  KF5::Libkdepim
  KF5::WidgetsAddons
)
endif()
