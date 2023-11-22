
# Add libraries first
add_subdirectory(ultrasoniclib)

# Versions to build:
set(BUILD_PLUGIN_FORMATS "")
if (BUILD_PLUGIN_FORMAT_VST2)
    list(APPEND BUILD_PLUGIN_FORMATS VST)
    message("--  Building VST versions")
endif()
if (BUILD_PLUGIN_FORMAT_VST3) 
    list(APPEND BUILD_PLUGIN_FORMATS VST3)
    message("--  Building VST3 versions")
endif()
if (BUILD_PLUGIN_FORMAT_AU) 
    list(APPEND BUILD_PLUGIN_FORMATS AU)
    message("--  Building AU versions")
endif()
if (BUILD_PLUGIN_FORMAT_LV2) 
    list(APPEND BUILD_PLUGIN_FORMATS LV2)
    message("--  Building LV2 versions")
endif()

# Then the plugins
add_subdirectory(_UltrasonicSuperHearing_)