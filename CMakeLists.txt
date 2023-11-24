# SPDX-FileCopyrightText: none
# SPDX-License-Identifier: BSD-3-Clause
cmake_minimum_required(VERSION 3.16 FATAL_ERROR)
set(PIM_VERSION "5.240.81")

project(incidenceeditor VERSION ${PIM_VERSION})

set(KF_MIN_VERSION "5.245.0")
set(QT_REQUIRED_VERSION "6.5.0")

find_package(ECM ${KF_MIN_VERSION} CONFIG REQUIRED)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

include(ECMGenerateExportHeader)
include(ECMSetupVersion)
include(ECMGenerateHeaders)

include(FeatureSummary)
include(KDEGitCommitHooks)
include(KDEClangFormat)
file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h *.c)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})
include(ECMQtDeclareLoggingCategory)
include(ECMDeprecationSettings)
include(ECMAddTests)
include(ECMAddQch)
option(BUILD_QCH "Build API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)" OFF)
add_feature_info(QCH ${BUILD_QCH} "API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)")


set(INCIDENCEEDITOR_LIB_VERSION ${PIM_VERSION})
set(AKONADI_MIMELIB_VERSION "5.240.81")
set(KDEPIM_LIB_VERSION "5.240.81")
set(KMIME_LIB_VERSION "5.240.81")
set(KLDAP_LIB_VERSION "5.240.81")
set(CALENDARUTILS_LIB_VERSION "5.240.81")
set(CALENDARSUPPORT_LIB_VERSION "5.240.81")
set(LIBKDEPIM_LIB_VERSION "5.240.81")
set(EVENTVIEW_LIB_VERSION "5.240.81")
set(AKONADI_VERSION "5.240.81")
set(AKONADICONTACT_LIB_VERSION "5.240.45")
set(AKONADICALENDAR_LIB_VERSION "5.240.81")
set(PIMCOMMON_LIB_VERSION "5.240.81")
set(PIMTEXTEDIT_LIB_VERSION "5.240.81")
set(IDENTITYMANAGEMENT_LIB_VERSION "5.240.81")

set(KDIAGRAM_LIB_VERSION "3.0.0")
find_package(KGantt6 ${KDIAGRAM_LIB_VERSION} CONFIG REQUIRED)

find_package(Qt6 ${QT_REQUIRED_VERSION} CONFIG REQUIRED Widgets)

find_package(KF6CalendarCore ${KF_MIN_VERSION} CONFIG REQUIRED)
find_package(KF6Codecs ${KF_MIN_VERSION} CONFIG REQUIRED)
find_package(KF6I18n ${KF_MIN_VERSION} CONFIG REQUIRED)
find_package(KF6IconThemes ${KF_MIN_VERSION} CONFIG REQUIRED)
find_package(KF6KIO ${KF_MIN_VERSION} CONFIG REQUIRED)
find_package(KF6TextWidgets ${KF_MIN_VERSION} CONFIG REQUIRED)

find_package(KPim6Akonadi ${AKONADI_VERSION} CONFIG REQUIRED)
find_package(KPim6LdapWidgets ${KLDAP_LIB_VERSION} CONFIG REQUIRED)
find_package(KPim6CalendarSupport ${CALENDARSUPPORT_LIB_VERSION} CONFIG REQUIRED)
find_package(KPim6EventViews ${EVENTVIEW_LIB_VERSION} CONFIG REQUIRED)
find_package(KPim6Libkdepim ${LIBKDEPIM_LIB_VERSION} CONFIG REQUIRED)
find_package(KPim6CalendarUtils ${CALENDARUTILS_LIB_VERSION} CONFIG REQUIRED)
find_package(KPim6AkonadiCalendar ${AKONADICALENDAR_LIB_VERSION} CONFIG REQUIRED)
find_package(KPim6AkonadiMime ${AKONADI_MIMELIB_VERSION} CONFIG REQUIRED)
find_package(KPim6CalendarSupport ${CALENDARSUPPORT_LIB_VERSION} CONFIG REQUIRED)
find_package(KPim6CalendarUtils ${CALENDARUTILS_LIB_VERSION} CONFIG REQUIRED)
find_package(KPim6EventViews ${EVENTVIEW_LIB_VERSION} CONFIG REQUIRED)
find_package(KPim6Libkdepim ${LIBKDEPIM_LIB_VERSION} CONFIG REQUIRED)
find_package(KPim6Mime ${KMIME_LIB_VERSION} CONFIG REQUIRED)
find_package(KPim6PimCommonAkonadi ${PIMCOMMON_LIB_VERSION} CONFIG REQUIRED)
find_package(KPim6TextEdit ${PIMTEXTEDIT_LIB_VERSION} CONFIG REQUIRED)
find_package(KPim6IdentityManagementCore ${IDENTITYMANAGEMENT_LIB_VERSION} CONFIG REQUIRED)

ecm_setup_version(PROJECT VARIABLE_PREFIX INCIDENCEEDITOR
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/src/incidenceeditor_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KPim6IncidenceEditorConfigVersion.cmake"
                        SOVERSION 6
)

option(KDEPIM_ENTERPRISE_BUILD "Enable features specific to the enterprise branch, which are normally disabled. Also, it disables many components not needed for Kontact such as the Kolab client." FALSE)

# config-enterprise.h is needed for both ENTERPRISE_BUILD and BUILD_EVERYTHING
configure_file(config-enterprise.h.in ${CMAKE_CURRENT_BINARY_DIR}/src/config-enterprise.h )

########### Targets ###########


add_definitions(-DQT_NO_CONTEXTLESS_CONNECT)
ecm_set_disabled_deprecation_versions(QT 6.6  KF 5.245.0)


option(USE_UNITY_CMAKE_SUPPORT "Use UNITY cmake support (speedup compile time)" OFF)

set(COMPILE_WITH_UNITY_CMAKE_SUPPORT OFF)
if (USE_UNITY_CMAKE_SUPPORT)
    set(COMPILE_WITH_UNITY_CMAKE_SUPPORT ON)
    add_definitions(-DCOMPILE_WITH_UNITY_CMAKE_SUPPORT)
endif()

if(BUILD_TESTING)
  add_definitions(-DBUILD_TESTING)
endif()
add_subdirectory(src)

########### CMake Config Files ###########
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KPim6IncidenceEditor")
if (BUILD_QCH)
    ecm_install_qch_export(
        TARGETS KPim6IncidenceEditor_QCH
        FILE KPim6IncidenceEditorQchTargets.cmake
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel
    )
    set(PACKAGE_INCLUDE_QCHTARGETS "include(\"\${CMAKE_CURRENT_LIST_DIR}/KPim6IncidenceEditorQchTargets.cmake\")")
endif()
configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KPimIncidenceEditorConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KPim6IncidenceEditorConfig.cmake"
  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KPim6IncidenceEditorConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KPim6IncidenceEditorConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT KPim6IncidenceEditorTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
    FILE KPim6IncidenceEditorTargets.cmake NAMESPACE KPim6::)

install(FILES
   ${CMAKE_CURRENT_BINARY_DIR}/src/incidenceeditor_version.h
  DESTINATION ${KDE_INSTALL_INCLUDEDIR}/KPim6/IncidenceEditor/ COMPONENT Devel
)

if (BUILD_TESTING)
   find_package(Qt6 ${QT_REQUIRED_VERSION} CONFIG REQUIRED Test)

   add_subdirectory(autotests)
   add_subdirectory(tests)
endif()

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
ki18n_install(po)
feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
