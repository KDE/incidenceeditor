# SPDX-FileCopyrightText: none
# SPDX-License-Identifier: BSD-3-Clause
add_definitions(-DTRANSLATION_DOMAIN=\"libincidenceeditors6\")
add_library(KPim6IncidenceEditor)
add_library(KPim6::IncidenceEditor ALIAS KPim6IncidenceEditor)

target_sources(KPim6IncidenceEditor PRIVATE
  attachmenteditdialog.cpp
  attachmenticonview.cpp
  attendeedata.cpp
  attendeeline.cpp
  attendeecomboboxdelegate.cpp
  attendeelineeditdelegate.cpp
  attendeetablemodel.cpp
  attendeeeditor.cpp

  alarmpresets.cpp
  alarmdialog.cpp
  incidenceeditorsettings.cpp

  ktimezonecombobox.cpp

  # TODO: Move the next two to akonadi libs when finished
  editoritemmanager.cpp

  freebusyurldialog.cpp

  # Shared incidence editors code
  combinedincidenceeditor.cpp
  incidenceeditor.cpp

  # Specific editors
  incidencealarm.cpp
  incidenceattachment.cpp
  incidenceattendee.cpp
  incidencecategories.cpp
  incidencecompletionpriority.cpp
  incidencedescription.cpp
  incidencewhatwhere.cpp
  incidencedatetime.cpp
  incidencerecurrence.cpp
  incidenceresource.cpp
  incidencesecrecy.cpp

  freebusyganttproxymodel.cpp
  conflictresolver.cpp
  schedulingdialog.cpp

  incidencedefaults.cpp
  editorconfig.cpp
  korganizereditorconfig.cpp

  # Individual mail
  individualmailcomponentfactory.cpp
  individualmaildialog.cpp
  opencomposerjob.cpp

  # Resourcemanagement
  ldaputils.cpp
  resourcemanagement.cpp
  resourceitem.cpp
  resourcemodel.cpp
  kweekdaycheckcombo.cpp

  attendeeeditor.h
  editorconfig.h
  alarmpresets.h
  individualmaildialog.h
  attachmenteditdialog.h
  incidencecompletionpriority.h
  incidencewhatwhere.h
  visualfreebusywidget.h
  attachmenticonview.h
  incidenceeditor_private_export.h
  resourcemanagement.h
  ldaputils.h
  resourcemodel.h
  incidencealarm.h
  incidencedefaults.h
  freebusyganttproxymodel.h
  incidenceattendee.h
  korganizereditorconfig.h
  freebusyurldialog.h
  ktimezonecombobox.h
  incidencedescription.h
  conflictresolver.h
  editoritemmanager.h
  alarmdialog.h
  incidencesecrecy.h
  combinedincidenceeditor.h
  incidencedatetime.h
  incidencerecurrence.h
  attendeetablemodel.h
  incidenceresource.h
  individualmailcomponentfactory.h
  opencomposerjob.h
  schedulingdialog.h
  incidencedialog.h
  attendeecomboboxdelegate.h
  incidencedialogfactory.h
  templatemanagementdialog.h
  incidenceeditor-ng.h
  incidencecategories.h
  attendeedata.h
  resourceitem.h
  kweekdaycheckcombo.h
  incidenceattachment.h
  attendeelineeditdelegate.h
  incidenceeditorsettings.h
  attendeeline.h
)

ecm_qt_declare_logging_category(KPim6IncidenceEditor HEADER incidenceeditor_debug.h IDENTIFIER INCIDENCEEDITOR_LOG CATEGORY_NAME org.kde.pim.incidenceeditor
        OLD_CATEGORY_NAMES log_incidenceeditor
        DESCRIPTION "incidenceeditor (incidenceeditor)" EXPORT INCIDENCEEDITOR)

kconfig_add_kcfg_files(KPim6IncidenceEditor globalsettings_incidenceeditor.kcfgc)

### Build the desktop version

target_sources(KPim6IncidenceEditor PRIVATE
  templatemanagementdialog.cpp
  incidencedialogfactory.cpp
  incidencedialog.cpp
  visualfreebusywidget.cpp
  incidenceeditor.qrc
)

ki18n_wrap_ui(KPim6IncidenceEditor
  ui/template_management_dialog_base.ui
  ui/dialogdesktop.ui
  ui/alarmdialog.ui
  ui/attachmenteditdialog.ui
  ui/schedulingdialog.ui
  ui/resourcemanagement.ui
)

if (COMPILE_WITH_UNITY_CMAKE_SUPPORT)
    set_target_properties(KPim6IncidenceEditor PROPERTIES UNITY_BUILD ON)
endif()
ecm_generate_export_header(KPim6IncidenceEditor
    BASE_NAME incidenceeditor
    VERSION ${INCIDENCEEDITOR_VERSION}
    DEPRECATED_BASE_VERSION 0
    USE_VERSION_HEADER
)


target_link_libraries(KPim6IncidenceEditor
PUBLIC
  KPim6::AkonadiCalendar
  KPim6::AkonadiMime
  KF6::CalendarCore
  KPim6::CalendarUtils
  KPim6::CalendarSupport      # For KCalPrefs
  KPim6::EventViews
PRIVATE
  KGantt6             # For FreeBusy Editor
  KPim6::PimTextEdit
  KF6::Codecs
  KPim6::LdapWidgets
  KPim6::AkonadiWidgets
  KF6::IconThemes
  KF6::Completion
  KF6::KIOWidgets
  KF6::TextWidgets
  KF6::WidgetsAddons
  KF6::XmlGui
  KF6::Contacts
  KPim6::Libkdepim
  KPim6::PimCommonAkonadi
  KPim6::IdentityManagementCore
)
target_include_directories(KPim6IncidenceEditor INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR}/KPim6/IncidenceEditor/>")
target_include_directories(KPim6IncidenceEditor PUBLIC "$<BUILD_INTERFACE:${incidenceeditor_SOURCE_DIR}/src;${incidenceeditor_BINARY_DIR}/src;>")

set_target_properties(KPim6IncidenceEditor PROPERTIES
    VERSION ${INCIDENCEEDITOR_VERSION}
    SOVERSION ${INCIDENCEEDITOR_SOVERSION}
    EXPORT_NAME IncidenceEditor
)

install(TARGETS
    KPim6IncidenceEditor
    EXPORT KPim6IncidenceEditorTargets ${KDE_INSTALL_TARGETS_DEFAULT_ARGS}
)

ecm_generate_headers(IncidenceEditor_CamelCase_HEADERS
  HEADER_NAMES
  IncidenceDialog
  IncidenceDialogFactory
  IncidenceEditorSettings
  IncidenceDefaults
  IndividualMailComponentFactory
  EditorItemManager
  IncidenceEditor-Ng
  REQUIRED_HEADERS IncidenceEditor_HEADERS
  PREFIX IncidenceEditor
)

install(FILES
    ${IncidenceEditor_CamelCase_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/KPim6/IncidenceEditor/IncidenceEditor
    COMPONENT Devel
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/globalsettings_incidenceeditor.h
    ${CMAKE_CURRENT_BINARY_DIR}/incidenceeditor_export.h
    ${IncidenceEditor_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/KPim6/IncidenceEditor/incidenceeditor
    COMPONENT Devel
)


######
add_subdirectory(app)

if (BUILD_QCH)
    ecm_add_qch(
        KPim6IncidenceEditor_QCH
        NAME KIncidenceEditor
        BASE_NAME KPim6IncidenceEditor
        VERSION ${PIM_VERSION}
        ORG_DOMAIN org.kde
        SOURCES # using only public headers, to cover only public API
            ${IncidenceEditor_HEADERS}
        #MD_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md"
        #IMAGE_DIRS "${CMAKE_SOURCE_DIR}/docs/pics"
        LINK_QCHS
            Qt6Core_QCH
            Qt6Gui_QCH
            Qt6Widgets_QCH
        INCLUDE_DIRS
            ${CMAKE_CURRENT_BINARY_DIR}
        BLANK_MACROS
            INCIDENCEEDITOR_EXPORT
        TAGFILE_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        QCH_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        COMPONENT Devel
    )
endif()

ecm_qt_install_logging_categories(EXPORT INCIDENCEEDITOR FILE incidenceeditor.categories DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR})
