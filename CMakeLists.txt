cmake_minimum_required(VERSION 3.5)
set(PIM_VERSION "5.10.40")

project(incidenceeditor VERSION ${PIM_VERSION})

set(KF5_MIN_VERSION "5.55.0")

find_package(ECM ${KF5_MIN_VERSION} CONFIG REQUIRED)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
set(LIBRARY_NAMELINK)
if (POLICY CMP0053)
    cmake_policy(SET CMP0053 NEW)
endif()
include(GenerateExportHeader)
include(ECMSetupVersion)
include(ECMGenerateHeaders)
include(ECMGeneratePriFile)

include(ECMSetupVersion)
include(FeatureSummary)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)
include(ECMQtDeclareLoggingCategory)
include(ECMAddTests)




set(INCIDENCEEDITOR_LIB_VERSION ${PIM_VERSION})
set(AKONADI_MIMELIB_VERSION "5.10.40")
set(KDEPIM_LIB_VERSION "5.10.40")
set(QT_REQUIRED_VERSION "5.10.0")
set(KMIME_LIB_VERSION "5.10.40")
set(KMAILTRANSPORT_LIB_VERSION "5.10.40")
set(KLDAP_LIB_VERSION "5.10.40")
set(CALENDARUTILS_LIB_VERSION "5.10.40")
set(KCALENDARCORE_LIB_VERSION "5.10.40")
set(CALENDARSUPPORT_LIB_VERSION "5.10.40")
set(LIBKDEPIM_LIB_VERSION "5.10.40")
set(EVENTVIEW_LIB_VERSION "5.10.40")
set(AKONADI_VERSION "5.10.40")


set(KDIAGRAM_LIB_VERSION "1.4.0")
find_package(KGantt ${KDIAGRAM_LIB_VERSION} CONFIG REQUIRED)

find_package(KF5Akonadi ${AKONADI_VERSION} CONFIG REQUIRED)
find_package(Qt5 ${QT_REQUIRED_VERSION} CONFIG REQUIRED Widgets)
find_package(KF5I18n ${KF5_MIN_VERSION} CONFIG REQUIRED)
find_package(KF5IconThemes ${KF5_MIN_VERSION} CONFIG REQUIRED)
find_package(KF5KIO ${KF5_MIN_VERSION} CONFIG REQUIRED)
find_package(KF5Mime ${KMIME_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5AkonadiMime ${AKONADI_MIMELIB_VERSION} CONFIG REQUIRED)

find_package(KF5Ldap ${KLDAP_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5Codecs ${KF5_MIN_VERSION} CONFIG REQUIRED)
find_package(KF5CalendarSupport ${CALENDARSUPPORT_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5EventViews ${EVENTVIEW_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5LibkdepimAkonadi ${LIBKDEPIM_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5KdepimDBusInterfaces ${KDEPIM_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5CalendarUtils ${CALENDARUTILS_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5CalendarCore ${KCALENDARCORE_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5MailTransportAkonadi ${KMAILTRANSPORT_LIB_VERSION} CONFIG REQUIRED)

ecm_setup_version(PROJECT VARIABLE_PREFIX INCIDENCEEDITOR
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/incidenceeditor_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5IncidenceEditorConfigVersion.cmake"
                        SOVERSION 5
)

option(KDEPIM_ENTERPRISE_BUILD "Enable features specific to the enterprise branch, which are normally disabled. Also, it disables many components not needed for Kontact such as the Kolab client." FALSE)


# config-enterprise.h is needed for both ENTERPRISE_BUILD and BUILD_EVERYTHING
configure_file(config-enterprise.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/src/config-enterprise.h )

########### Targets ###########


add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x060000)
########### CMake Config Files ###########
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF5IncidenceEditor")

configure_package_config_file(
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

install( FILES incidenceeditor.categories incidenceeditor.renamecategories DESTINATION ${KDE_INSTALL_CONFDIR} )

add_subdirectory(src)
if (BUILD_TESTING)
   find_package(Qt5 ${QT_REQUIRED_VERSION} CONFIG REQUIRED Test)

   add_subdirectory(autotests)
   add_subdirectory(tests)
endif()

