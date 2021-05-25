
set(testincidenceeditor_SRCS main.cpp)

add_executable(kincidenceeditor main.cpp)

target_link_libraries(kincidenceeditor
    KF5::AkonadiCalendar
    KF5::AkonadiCore
    KF5::CalendarCore
    KF5::CalendarUtils
    KF5::IncidenceEditor
    KF5::CalendarSupport
    )

install(TARGETS kincidenceeditor ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
