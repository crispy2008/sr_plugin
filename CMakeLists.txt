set(PLUGIN_NAME plugin.sr_plugin${ILLIXR_BUILD_SUFFIX})
set(SR_PLUGIN_SOURCES plugin.cpp
                    plugin.hpp)

set(CMAKE_PREFIX_PATH ${CMAKE_SOURCE_DIR}/plugins/sr_plugin/portaudio ${CMAKE_PREFIX_PATH})
add_subdirectory("/scratch/yuanyi2/ILLIXR/plugins/sr_plugin/portaudio") #
add_library(${PLUGIN_NAME} SHARED ${SR_PLUGIN_SOURCES})


# to use the one installed in system: # we go with this 
# set(CMAKE_MODULE_PATH "/scratch/yuanyi2/portaudio")
find_package(PortAudio REQUIRED)
# target_link_libraries(${PLUGIN_NAME} PRIVATE CMAKE_MODULE_PATH)

# to get portaudio built as part of illixr
# set(PORTAUDIO_INCLUDE_DIRS "${CMAKE_CURRENT_BINARY_DIR}/portaudio/include")
# set(PORTAUDIO_LIBRARIES "${CMAKE_CURRENT_BINARY_DIR}/portaudio")
# target_include_directories(${PLUGIN_NAME} PRIVATE ${PORTAUDIO_INCLUDE_DIRS})
# target_link_libraries(${PLUGIN_NAME} ${PORTAUDIO_LIBRARIES})
target_link_libraries(${PLUGIN_NAME} PRIVATE portaudio) #

target_include_directories(${PLUGIN_NAME} PRIVATE ${ILLIXR_SOURCE_DIR}/include ${ILLIXR_SOURCE_DIR}/plugins/sr_plugin/portaudio/include)

target_compile_features(${PLUGIN_NAME} PRIVATE cxx_std_17)

install(TARGETS ${PLUGIN_NAME} DESTINATION lib)