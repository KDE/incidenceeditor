cmake_minimum_required(VERSION 2.8.12)

project(incidenceeditor)

find_package(ECM 5.19.0 CONFIG REQUIRED)
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


set(KF5_VERSION "5.19.0")
set(INCIDENCEEDITOR_LIB_VERSION "5.1.80")
set(KDEPIMLIBS_LIB_VERSION "5.1.80")
set(KDEPIM_LIB_VERSION "5.1.80")
set(QT_REQUIRED_VERSION "5.4.0")
set(KMIME_LIB_VERSION "5.1.80")
set(KMAILTRANSPORT_LIB_VERSION "5.1.80")
set(KLDAP_LIB_VERSION "5.1.80")
set(CALENDARUTILS_LIB_VERSION "5.1.80")
set(KCALENDARCORE_LIB_VERSION "5.1.80")
set(CALENDARSUPPORT_LIB_VERSION "5.1.80")
set(LIBKDEPIM_LIB_VERSION "5.1.80")
set(KDGANTT2_LIB_VERSION "5.1.80")
set(EVENTVIEW_LIB_VERSION "5.1.80")

find_package(KF5Akonadi ${KDEPIMLIBS_LIB_VERSION} CONFIG REQUIRED)
find_package(Qt5 ${QT_REQUIRED_VERSION} CONFIG REQUIRED Widgets Test UiTools WebKitWidgets)
find_package(KF5I18n ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5Mime ${KMIME_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5AkonadiMime ${KDEPIMLIBS_LIB_VERSION} CONFIG REQUIRED)

find_package(KF5Ldap ${KLDAP_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5Codecs ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5CalendarSupport ${CALENDARSUPPORT_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5EventViews ${EVENTVIEW_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5Libkdepim ${LIBKDEPIM_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5KdepimDBusInterfaces ${KDEPIM_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5KDGantt2 ${KDGANTT2_LIB_VERSION} CONFIG REQUIRED)
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

#REACTIVE IT
#add_definitions("-DQT_NO_CAST_FROM_ASCII -DQT_NO_CAST_TO_ASCII")
#TODO verify it
remove_definitions("-DQT_NO_URL_CAST_FROM_STRING")
remove_definitions( -DQT_NO_CAST_FROM_BYTEARRAY )
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

add_subdirectory(src)
if (BUILD_TESTING)
   add_subdirectory(autotests)
   add_subdirectory(tests)
endif()

