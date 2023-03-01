# SPDX-FileCopyrightText: none
# SPDX-License-Identifier: BSD-3-Clause
set(testincidenceeditor_SRCS main.cpp)

add_executable(kincidenceeditor main.cpp)

target_link_libraries(kincidenceeditor
    KPim${KF_MAJOR_VERSION}::AkonadiCalendar
    KPim${KF_MAJOR_VERSION}::AkonadiCore
    KF${KF_MAJOR_VERSION}::CalendarCore
    KPim${KF_MAJOR_VERSION}::CalendarUtils
    KPim${KF_MAJOR_VERSION}::IncidenceEditor
    KPim${KF_MAJOR_VERSION}::CalendarSupport
    )

install(TARGETS kincidenceeditor ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
