# SPDX-FileCopyrightText: none
# SPDX-License-Identifier: BSD-3-Clause
add_definitions(-DTRANSLATION_DOMAIN=\"libincidenceeditors\")
add_library(KPim${KF_MAJOR_VERSION}IncidenceEditor)
add_library(KPim${KF_MAJOR_VERSION}::IncidenceEditor ALIAS KPim${KF_MAJOR_VERSION}IncidenceEditor)

target_sources(KPim${KF_MAJOR_VERSION}IncidenceEditor PRIVATE
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
  groupwareuidelegate.cpp

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
  groupwareuidelegate.h
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

ecm_qt_declare_logging_category(KPim${KF_MAJOR_VERSION}IncidenceEditor HEADER incidenceeditor_debug.h IDENTIFIER INCIDENCEEDITOR_LOG CATEGORY_NAME org.kde.pim.incidenceeditor
        OLD_CATEGORY_NAMES log_incidenceeditor
        DESCRIPTION "incidenceeditor (incidenceeditor)" EXPORT INCIDENCEEDITOR)

kconfig_add_kcfg_files(KPim${KF_MAJOR_VERSION}IncidenceEditor globalsettings_incidenceeditor.kcfgc)

### Build the desktop version

target_sources(KPim${KF_MAJOR_VERSION}IncidenceEditor PRIVATE
  templatemanagementdialog.cpp
  incidencedialogfactory.cpp
  incidencedialog.cpp
  visualfreebusywidget.cpp
  incidenceeditor.qrc
)

ki18n_wrap_ui(KPim${KF_MAJOR_VERSION}IncidenceEditor
  ui/template_management_dialog_base.ui
  ui/dialogdesktop.ui
  ui/alarmdialog.ui
  ui/attachmenteditdialog.ui
  ui/schedulingdialog.ui
  ui/resourcemanagement.ui
)

if (COMPILE_WITH_UNITY_CMAKE_SUPPORT)
    set_target_properties(KPim${KF_MAJOR_VERSION}IncidenceEditor PROPERTIES UNITY_BUILD ON)
endif()
generate_export_header(KPim${KF_MAJOR_VERSION}IncidenceEditor BASE_NAME incidenceeditor)


target_link_libraries(KPim${KF_MAJOR_VERSION}IncidenceEditor
PUBLIC
  KPim${KF_MAJOR_VERSION}::AkonadiCalendar
  KPim${KF_MAJOR_VERSION}::AkonadiMime
  KF${KF_MAJOR_VERSION}::CalendarCore
  KPim${KF_MAJOR_VERSION}::CalendarUtils
  KPim${KF_MAJOR_VERSION}::CalendarSupport      # For KCalPrefs
  KPim${KF_MAJOR_VERSION}::EventViews
PRIVATE
  KGantt              # For FreeBusy Editor
  KPim${KF_MAJOR_VERSION}::PimTextEdit
  KF${KF_MAJOR_VERSION}::Codecs
  KPim${KF_MAJOR_VERSION}::Ldap
  KPim${KF_MAJOR_VERSION}::AkonadiWidgets
  KF${KF_MAJOR_VERSION}::IconThemes
  KF${KF_MAJOR_VERSION}::Completion
  KF${KF_MAJOR_VERSION}::KIOWidgets
  KF${KF_MAJOR_VERSION}::TextWidgets
  KF${KF_MAJOR_VERSION}::WidgetsAddons
  KF${KF_MAJOR_VERSION}::XmlGui
  KF${KF_MAJOR_VERSION}::Contacts
  KPim${KF_MAJOR_VERSION}::Libkdepim
  KPim${KF_MAJOR_VERSION}::PimCommonAkonadi
)
target_include_directories(KPim${KF_MAJOR_VERSION}IncidenceEditor INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR}/KPim${KF_MAJOR_VERSION}/IncidenceEditor/>")
target_include_directories(KPim${KF_MAJOR_VERSION}IncidenceEditor PUBLIC "$<BUILD_INTERFACE:${incidenceeditor_SOURCE_DIR}/src;${incidenceeditor_BINARY_DIR}/src;>")

set_target_properties(KPim${KF_MAJOR_VERSION}IncidenceEditor PROPERTIES
    VERSION ${INCIDENCEEDITOR_VERSION}
    SOVERSION ${INCIDENCEEDITOR_SOVERSION}
    EXPORT_NAME IncidenceEditor
)

install(TARGETS
    KPim${KF_MAJOR_VERSION}IncidenceEditor
    EXPORT KPim${KF_MAJOR_VERSION}IncidenceEditorTargets ${KDE_INSTALL_TARGETS_DEFAULT_ARGS}
)

ecm_generate_headers(IncidenceEditor_CamelCase_HEADERS
  HEADER_NAMES
  IncidenceDialog
  IncidenceDialogFactory
  IncidenceEditorSettings
  IncidenceDefaults
  IndividualMailComponentFactory
  GroupwareUiDelegate
  EditorItemManager
  IncidenceEditor-Ng
  REQUIRED_HEADERS IncidenceEditor_HEADERS
  PREFIX IncidenceEditor
)

ecm_generate_pri_file(BASE_NAME IncidenceEditor
    LIB_NAME KPim${KF_MAJOR_VERSION}IncidenceEditor
    DEPS "AkonadiCalendar AkonadiMime KCalendarCore CalendarUtils CalendarSupport EventViews" FILENAME_VAR PRI_FILENAME INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR}/KPim${KF_MAJOR_VERSION}/IncidenceEditor
)

install(FILES
    ${IncidenceEditor_CamelCase_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/KPim${KF_MAJOR_VERSION}/IncidenceEditor/IncidenceEditor
    COMPONENT Devel
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/globalsettings_incidenceeditor.h
    ${CMAKE_CURRENT_BINARY_DIR}/incidenceeditor_export.h
    ${IncidenceEditor_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/KPim${KF_MAJOR_VERSION}/IncidenceEditor/incidenceeditor
    COMPONENT Devel
)
install(FILES
    ${PRI_FILENAME}
    DESTINATION ${ECM_MKSPECS_INSTALL_DIR})



######
if (BUILD_TESTING)
   add_subdirectory(tests)
endif()

if (BUILD_QCH)
    ecm_add_qch(
        KPim${KF_MAJOR_VERSION}IncidenceEditor_QCH
        NAME KIncidenceEditor
        BASE_NAME KPim${KF_MAJOR_VERSION}IncidenceEditor
        VERSION ${PIM_VERSION}
        ORG_DOMAIN org.kde
        SOURCES # using only public headers, to cover only public API
            ${IncidenceEditor_HEADERS}
        #MD_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md"
        #IMAGE_DIRS "${CMAKE_SOURCE_DIR}/docs/pics"
        LINK_QCHS
            Qt${QT_MAJOR_VERSION}Core_QCH
            Qt${QT_MAJOR_VERSION}Gui_QCH
            Qt${QT_MAJOR_VERSION}Widgets_QCH
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
