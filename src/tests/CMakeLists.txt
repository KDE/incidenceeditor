# SPDX-FileCopyrightText: none
# SPDX-License-Identifier: BSD-3-Clause
set(testincidenceeditor_SRCS main.cpp)

add_executable(kincidenceeditor main.cpp)

target_link_libraries(kincidenceeditor
    KPim6::AkonadiCalendar
    KPim6::AkonadiCore
    KF6::CalendarCore
    KPim6::CalendarUtils
    KPim6::IncidenceEditor
    KPim6::CalendarSupport
    )

#install(TARGETS kincidenceeditor ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
