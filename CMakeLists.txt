set(PLUGIN_NAME plugin.sr_plugin${ILLIXR_BUILD_SUFFIX})
set(SR_PLUGIN_SOURCES plugin.cpp
                    plugin.hpp)

# get_external_for_plugin(ILLIXR_FFmpeg)

# add_library(sr_plugin sr_plugin.cpp)
add_library(${PLUGIN_NAME} SHARED ${SR_PLUGIN_SOURCES})

target_include_directories(${PLUGIN_NAME} PRIVATE ${ILLIXR_SOURCE_DIR}/include)

target_compile_features(${PLUGIN_NAME} PRIVATE cxx_std_17)

install(TARGETS ${PLUGIN_NAME} DESTINATION lib)