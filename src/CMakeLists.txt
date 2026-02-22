add_executable(safe-discover
    main.cpp
    core/commandrunner.cpp
    core/packagemodel.cpp
    core/logmanager.cpp
    core/toolchecker.cpp
    backends/pacmanbackend.cpp
    backends/aurbackend.cpp
    backends/flatpakbackend.cpp
    backends/firmwarebackend.cpp
    backends/updatemanager.cpp
)

kconfig_add_kcfg_files(safe-discover config/safediscoverconfig.kcfgc)

ecm_add_qml_module(safe-discover
    URI ca.kinncj.SafeDiscover
    QML_FILES
        qml/Main.qml
        qml/pages/RepoPage.qml
        qml/pages/AurPage.qml
        qml/pages/FlatpakPage.qml
        qml/pages/FirmwarePage.qml
        qml/pages/UpdatesPage.qml
        qml/components/PackageSearchBar.qml
        qml/components/PackageListView.qml
        qml/components/PackageDetailsPane.qml
        qml/components/ActionConfirmDialog.qml
        qml/components/AurRemovalDialog.qml
        qml/components/OutputLogPanel.qml
        qml/components/StatusBanner.qml
        qml/components/FirmwareDeviceCard.qml
        qml/components/UpdateSection.qml
        qml/pages/KdeAddonsPage.qml
        qml/settings/SettingsPage.qml
)

target_link_libraries(safe-discover PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Qml
    KF6::Kirigami
    KF6::CoreAddons
    KF6::I18n
    KF6::ConfigCore
    KF6::ConfigGui
    KF6::NewStuffCore
)

target_include_directories(safe-discover PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

install(TARGETS safe-discover ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
