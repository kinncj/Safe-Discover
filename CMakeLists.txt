cmake_minimum_required(VERSION 3.20)

project(safe-discover VERSION 0.1.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ECM 6.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMQmlModule)

find_package(Qt6 6.10 REQUIRED COMPONENTS Core Widgets Quick QuickControls2 Qml)
find_package(KF6 6.23 REQUIRED COMPONENTS Kirigami CoreAddons I18n Config NewStuff)
find_package(KF6KirigamiAddons 1.0 REQUIRED)

qt_policy(SET QTP0001 NEW)

add_subdirectory(src)

if(BUILD_TESTING)
    find_package(Qt6 6.10 REQUIRED COMPONENTS Test)
    add_subdirectory(tests)
endif()

install(FILES ca.kinncj.SafeDiscover.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES ca.kinncj.SafeDiscover.metainfo.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
install(FILES ca.kinncj.safediscover.policy DESTINATION ${KDE_INSTALL_DATADIR}/polkit-1/actions)
install(PROGRAMS safe-discover-helper.sh DESTINATION ${KDE_INSTALL_LIBEXECDIR})
