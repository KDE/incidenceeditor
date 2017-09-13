set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

include_directories(${CMAKE_SOURCE_DIR}/src)

set(grant_lib KGantt)

macro(IE_UNIT_TESTS)
  foreach(_testname ${ARGN})
    add_executable(${_testname} ${_testname}.cpp modeltest.cpp)
    add_test(NAME ${_testname} COMMAND ${_testname})
    ecm_mark_as_test(${_testname})
    target_link_libraries(${_testname}
      Qt5::Test
      KF5::AkonadiCore
      ${grant_lib}
      KF5::CalendarUtils
      KF5::CalendarCore
      KF5::IncidenceEditor
      KF5::KdepimDBusInterfaces
      KF5::Libkdepim
    )
  endforeach()
endmacro()

macro(IE_EXECUTABLE_TESTS)
  foreach(_testname ${ARGN})
    add_executable(${_testname} NOGUI TEST ${_testname}.cpp modeltest.cpp)
    target_link_libraries(${_testname} KF5::CalendarCore Qt5::Test KF5::IncidenceEditor Qt5::Widgets)
  endforeach()
endmacro()

ie_unit_tests(
  conflictresolvertest
  testfreebusyganttproxymodel
)

########### KTimeZoneComboBox unit test #############
set(ktimezonecomboboxtest_SRCS ktimezonecomboboxtest.cpp)

add_executable(ktimezonecomboboxtest ${ktimezonecomboboxtest_SRCS})
add_test(NAME ktimezonecomboboxtest COMMAND ktimezonecomboboxtest)
ecm_mark_as_test(ktimezonecomboboxtest)

target_link_libraries(ktimezonecomboboxtest
  KF5::CalendarCore
  Qt5::Test
  KF5::IncidenceEditor
  KF5::Completion
)

add_executable(testindividualmaildialog testindividualmaildialog.cpp ../src/individualmaildialog.cpp)
ecm_mark_nongui_executable(testindividualmaildialog)
add_test(NAME testindividualmaildialog COMMAND testindividualmaildialog)
ecm_mark_as_test(testindividualmaildialog)
target_link_libraries(testindividualmaildialog
  KF5::CalendarCore
  Qt5::Test
  Qt5::Widgets
  KF5::WidgetsAddons
  KF5::I18n
)
