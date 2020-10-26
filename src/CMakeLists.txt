
add_definitions(-DTRANSLATION_DOMAIN=\"libincidenceeditors\")

set(incidenceeditors_ng_shared_LIB_SRCS
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

  autochecktreewidget.cpp
  categoryeditdialog.cpp
  categoryselectdialog.cpp
  categorydialog.cpp

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
)
qt5_add_resources(incidenceeditors_ng_shared_LIB_SRCS incidenceeditor.qrc)

ecm_qt_declare_logging_category(incidenceeditors_ng_shared_LIB_SRCS HEADER incidenceeditor_debug.h IDENTIFIER INCIDENCEEDITOR_LOG CATEGORY_NAME org.kde.pim.incidenceeditor
        OLD_CATEGORY_NAMES log_incidenceeditor
        DESCRIPTION "incidenceeditor (incidenceeditor)" EXPORT INCIDENCEEDITOR)

kconfig_add_kcfg_files(incidenceeditors_ng_shared_LIB_SRCS globalsettings_incidenceeditor.kcfgc)

### Build the desktop version

set(incidenceeditors_ng_desktop_LIB_SRCS
  ${incidenceeditors_ng_shared_LIB_SRCS}
  templatemanagementdialog.cpp
  incidencedialogfactory.cpp
  incidencedialog.cpp
  visualfreebusywidget.cpp
)

ki18n_wrap_ui(incidenceeditors_ng_desktop_LIB_SRCS
  ui/categoryeditdialog_base.ui
  ui/categoryselectdialog_base.ui
  ui/categorydialog_base.ui
  ui/template_management_dialog_base.ui
  ui/dialogdesktop.ui
  ui/alarmdialog.ui
  ui/attachmenteditdialog.ui
  ui/schedulingdialog.ui
  ui/resourcemanagement.ui
)

add_library(KF5IncidenceEditor ${incidenceeditors_ng_desktop_LIB_SRCS})
generate_export_header(KF5IncidenceEditor BASE_NAME incidenceeditor)

add_library(KF5::IncidenceEditor ALIAS KF5IncidenceEditor)



target_link_libraries(KF5IncidenceEditor
PUBLIC
  KF5::AkonadiCalendar
  KF5::AkonadiMime
  KF5::CalendarCore
  KF5::CalendarUtils
  KF5::MailTransportAkonadi
  KF5::CalendarSupport      # For KCalPrefs
  KF5::EventViews
PRIVATE
  KGantt              # For FreeBusy Editor
  KF5::Codecs
  KF5::Ldap
  KF5::AkonadiWidgets
  KF5::IconThemes
  KF5::Completion
  KF5::KIOWidgets
  KF5::TextWidgets
  KF5::WidgetsAddons
  KF5::XmlGui
  KF5::Contacts
  KF5::Libkdepim
  KF5::PimCommonAkonadi
)
target_include_directories(KF5IncidenceEditor INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF5}/IncidenceEditor/;${KDE_INSTALL_INCLUDEDIR_KF5}/incidenceeditor>")
target_include_directories(KF5IncidenceEditor PUBLIC "$<BUILD_INTERFACE:${incidenceeditor_SOURCE_DIR}/src;${incidenceeditor_BINARY_DIR}/src;>")

set_target_properties(KF5IncidenceEditor PROPERTIES
    VERSION ${INCIDENCEEDITOR_VERSION_STRING}
    SOVERSION ${INCIDENCEEDITOR_SOVERSION}
    EXPORT_NAME IncidenceEditor
)

install(TARGETS
    KF5IncidenceEditor
    EXPORT KF5IncidenceEditorTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS} 
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
    LIB_NAME KF5IncidenceEditor
    DEPS "AkonadiCalendar AkonadiMime KCalendarCore CalendarUtils KMailTransport CalendarSupport EventViews" FILENAME_VAR PRI_FILENAME INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR_KF5}/IncidenceEditor
)

install(FILES
    ${IncidenceEditor_CamelCase_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/IncidenceEditor
    COMPONENT Devel
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/globalsettings_incidenceeditor.h
    ${CMAKE_CURRENT_BINARY_DIR}/incidenceeditor_export.h
    ${IncidenceEditor_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/incidenceeditor
    COMPONENT Devel
)
install(FILES
    ${PRI_FILENAME}
    DESTINATION ${ECM_MKSPECS_INSTALL_DIR})



######
if (BUILD_TESTING)
   add_subdirectory(tests)
endif()

ecm_qt_install_logging_categories(EXPORT INCIDENCEEDITOR FILE incidenceeditor.categories DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR})
