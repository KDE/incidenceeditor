cmake_minimum_required(VERSION 2.8.12)

project(incidenceeditor)

set(KF5_VERSION "5.25.0")
find_package(ECM ${KF5_VERSION} CONFIG REQUIRED)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
set(LIBRARY_NAMELINK)
include(GenerateExportHeader)
include(ECMSetupVersion)
include(ECMGenerateHeaders)
include(ECMGeneratePriFile)
include(ECMPackageConfigHelpers)
include(ECMSetupVersion)
include(FeatureSummary)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)
include(ECMQtDeclareLoggingCategory)
include(ECMAddTests)

set(PIM_VERSION "5.3.41")

set(INCIDENCEEDITOR_LIB_VERSION ${PIM_VERSION})
set(AKONADI_MIMELIB_VERSION "5.3.40")
set(KDEPIM_LIB_VERSION "5.3.40")
set(QT_REQUIRED_VERSION "5.5.0")
set(KMIME_LIB_VERSION "5.3.40")
set(KMAILTRANSPORT_LIB_VERSION "5.3.40")
set(KLDAP_LIB_VERSION "5.3.40")
set(CALENDARUTILS_LIB_VERSION "5.3.40")
set(KCALENDARCORE_LIB_VERSION "5.3.41")
set(CALENDARSUPPORT_LIB_VERSION "5.3.42")
set(LIBKDEPIM_LIB_VERSION "5.3.40")
set(EVENTVIEW_LIB_VERSION "5.3.40")
set(AKONADI_VERSION "5.3.40")


set(KDIAGRAM_LIB_VERSION "1.4.0")
find_package(KGantt ${KDIAGRAM_LIB_VERSION} CONFIG REQUIRED)

find_package(KF5Akonadi ${AKONADI_VERSION} CONFIG REQUIRED)
find_package(Qt5 ${QT_REQUIRED_VERSION} CONFIG REQUIRED Widgets Test)
find_package(KF5I18n ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5Mime ${KMIME_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5AkonadiMime ${AKONADI_MIMELIB_VERSION} CONFIG REQUIRED)

find_package(KF5Ldap ${KLDAP_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5Codecs ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5CalendarSupport ${CALENDARSUPPORT_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5EventViews ${EVENTVIEW_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5Libkdepim ${LIBKDEPIM_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5KdepimDBusInterfaces ${KDEPIM_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5CalendarUtils ${CALENDARUTILS_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5CalendarCore ${KCALENDARCORE_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5MailTransport ${KMAILTRANSPORT_LIB_VERSION} CONFIG REQUIRED)

ecm_setup_version(${INCIDENCEEDITOR_LIB_VERSION} VARIABLE_PREFIX INCIDENCEEDITOR
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/incidenceeditor_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5IncidenceEditorConfigVersion.cmake"
                        SOVERSION 5
)

option(KDEPIM_ENTERPRISE_BUILD "Enable features specific to the enterprise branch, which are normally disabled. Also, it disables many components not needed for Kontact such as the Kolab client." FALSE)


# config-enterprise.h is needed for both ENTERPRISE_BUILD and BUILD_EVERYTHING
configure_file(config-enterprise.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/src/config-enterprise.h )

########### Targets ###########

add_definitions("-DQT_NO_URL_CAST_FROM_STRING")

add_definitions("-DQT_NO_CAST_FROM_ASCII -DQT_NO_CAST_TO_ASCII")
########### CMake Config Files ###########
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF5IncidenceEditor")

ecm_configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KF5IncidenceEditorConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5IncidenceEditorConfig.cmake"
  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KF5IncidenceEditorConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5IncidenceEditorConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT KF5IncidenceEditorTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KF5IncidenceEditorTargets.cmake NAMESPACE KF5::)

install(FILES
   ${CMAKE_CURRENT_BINARY_DIR}/incidenceeditor_version.h
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5} COMPONENT Devel
)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

install( FILES incidenceeditor.categories DESTINATION ${KDE_INSTALL_CONFDIR} )

add_subdirectory(src)
if (BUILD_TESTING)
   add_subdirectory(autotests)
   add_subdirectory(tests)
endif()

