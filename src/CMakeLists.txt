include(ECMQmlModule)

add_library(xspeedtest)
ecm_add_qml_module(xspeedtest
    URI "org.xspeedtest"
    VERSION 1.0
    GENERATE_PLUGIN_SOURCE
    DEPENDENCIES QtQuick
)

target_sources(xspeedtest PRIVATE
    historymodel.cpp
    paths.cpp
    plasmoidinstaller.cpp
    speedsettings.cpp
    speedtestrunner.cpp
    windowblur.cpp
)

set_source_files_properties(qml/Stream.qml qml/Styles.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)
ecm_target_qml_sources(xspeedtest SOURCES
    qml/ActionButton.qml
    qml/CenterLayout.qml
    qml/CompactLayout.qml
    qml/DualLayout.qml
    qml/HistoryPage.qml
    qml/MinimalLayout.qml
    qml/Readout.qml
    qml/SettingsPage.qml
    qml/SettingsRow.qml
    qml/SpeedPanel.qml
    qml/SpeedView.qml
    qml/SplitLayout.qml
    qml/StatChips.qml
    qml/StyleCanvas.qml
    qml/Styles.qml
    qml/Stream.qml
    qml/Visualizers.js
    qml/WideLayout.qml
)

target_compile_definitions(xspeedtest PRIVATE
    XSPEEDTEST_HELPER_INSTALL_PATH="${CMAKE_INSTALL_PREFIX}/${XSPEEDTEST_HELPER_INSTALL_DIR}/xspeedtest-helper"
    XSPEEDTEST_PLASMOID_INSTALL_PATH="${CMAKE_INSTALL_PREFIX}/${XSPEEDTEST_PLASMOID_INSTALL_DIR}"
    XSPEEDTEST_MODULE_INSTALL_PATH="${CMAKE_INSTALL_PREFIX}/${KDE_INSTALL_QMLDIR}/org/xspeedtest"
)
target_link_libraries(xspeedtest PRIVATE Qt6::Core Qt6::Gui Qt6::Qml Qt6::Quick KF6::WindowSystem)

ecm_finalize_qml_module(xspeedtest DESTINATION ${KDE_INSTALL_QMLDIR})
set_target_properties(xspeedtestplugin PROPERTIES INSTALL_RPATH "$ORIGIN")
install(TARGETS xspeedtest ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
