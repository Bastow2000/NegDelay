target_sources("${PROJECT_NAME}"
        PRIVATE
        PluginEditor.cpp
        PluginProcessor.cpp
	ParamIds.cpp
	)

target_include_directories("${PROJECT_NAME}"
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/SubModules
	${CMAKE_CURRENT_SOURCE_DIR}/DSPTools
	${CMAKE_CURRENT_SOURCE_DIR}/LookAndFeel)

add_subdirectory(SubModules)
add_subdirectory(DSPTools)
add_subdirectory(LookAndFeel)


