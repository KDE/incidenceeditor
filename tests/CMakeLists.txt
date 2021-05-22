set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

add_executable(resourcemanagement_gui resourcemanagment_gui.cpp)
target_link_libraries(resourcemanagement_gui
  Qt::Core
  Qt::Gui
  KF5::Mime
  KF5::IncidenceEditor
  KF5::Libkdepim
  KF5::Ldap
)

add_executable(individualmaildialog_test individualmaildialog_test.cpp)
target_link_libraries(individualmaildialog_test
    KF5::IncidenceEditor
)
