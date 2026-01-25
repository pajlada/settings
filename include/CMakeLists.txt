target_sources(PajladaSettings PUBLIC
    FILE_SET headers TYPE HEADERS FILES
    pajlada/settings/backup.hpp
    pajlada/settings/common.hpp
    pajlada/settings/detail/realpath.hpp
    pajlada/settings/detail/rename.hpp
    pajlada/settings/equal.hpp
    pajlada/settings/internal.hpp
    pajlada/settings/loadoptions.hpp
    pajlada/settings/settingdata.hpp
    pajlada/settings/setting.hpp
    pajlada/settings/settinglistener.hpp
    pajlada/settings/settingmanager.hpp
    pajlada/settings/signalargs.hpp
    pajlada/settings.hpp
)

target_include_directories(PajladaSettings PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

if (PAJLADA_SETTINGS_INSTALL)
    install(TARGETS PajladaSettings PajladaSerialize PajladaSignals
        EXPORT SettingsTargets
        LIBRARY COMPONENT PajladaSettings_Runtime NAMELINK_COMPONENT PajladaSettings_Development
        ARCHIVE COMPONENT PajladaSettings_Development
        FILE_SET headers COMPONENT PajladaSettings_Development
        FILE_SET generated_headers COMPONENT PajladaSettings_Development
        RUNTIME COMPONENT PajladaSettings_Runtime
    )
endif ()
