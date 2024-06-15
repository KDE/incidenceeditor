# SPDX-FileCopyrightText: none
# SPDX-License-Identifier: BSD-3-Clause
set(testincidenceeditor_SRCS main.cpp)

add_executable(kincidenceeditor main.cpp)

ecm_qt_declare_logging_category(kincidenceeditor
    HEADER kincidenceeditor_debug.h
    IDENTIFIER KINCIDENCEEDITOR_LOG
    CATEGORY_NAME org.kde.pim.kincidenceeditor
    DESCRIPTION "kincidenceeditor (kincidenceeditor)"
    EXPORT INCIDENCEEDITOR
)

target_link_libraries(kincidenceeditor
    KPim6::AkonadiCalendar
    KPim6::AkonadiCore
    KF6::CalendarCore
    KPim6::CalendarUtils
    KPim6::IncidenceEditor
    KPim6::CalendarSupport
)

install(TARGETS kincidenceeditor DESTINATION ${KDE_FULLINSTALL_LIBEXECDIR})
