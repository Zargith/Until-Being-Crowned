cmake_minimum_required(VERSION 3.15)
project(UBC)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find dependencies
find_package(SFML 2.5 COMPONENTS graphics audio window system CONFIG REQUIRED)
find_package(TGUI 0.9 CONFIG REQUIRED)

set(UBC_SRC
	src/Utils.cpp
	src/Utils.hpp
	src/main.cpp
	src/Logger.cpp
	src/Logger.hpp
	src/Exceptions.cpp
	src/Exceptions.hpp
	src/Menu/Exceptions.hpp
	src/Menu/Menu.cpp
	src/Menu/Menu.hpp
	src/Menu/MenuMgr.cpp
	src/Menu/MenuMgr.hpp
	src/Input/Input.cpp
	src/Input/Input.hpp
	src/Input/SFMLJoypad.cpp
	src/Input/SFMLJoypad.hpp
	src/Input/SFMLKeyboard.cpp
	src/Input/SFMLKeyboard.hpp
	src/Rendering/Screen.cpp
	src/Rendering/Screen.hpp
	src/Resources/Logger.hpp
	src/DataType/Rect.hpp
	src/DataType/Vector.hpp
	src/Resources/Logger.cpp
	src/Resources/version.hpp
	src/Resources/Game.hpp
	src/Resources/State.hpp
	src/Resources/Resources.cpp
	src/Resources/Resources.hpp
	src/Loader.cpp
	src/Logger.hpp
	src/Resources/Settings.hpp
	src/Menu/InGameMenu.cpp
	src/Menu/InGameMenu.hpp
	src/Menu/MainMenu.cpp
	src/Menu/MainMenu.hpp
	src/QuestMgr.cpp
	src/QuestMgr.hpp
	src/Resources/GameState.hpp
	src/Menu/OptionMenu.cpp
	src/Menu/OptionMenu.hpp
	src/Menu/LoadingMenu.cpp
	src/Menu/LoadingMenu.hpp
	src/Menu/GenderMenu.cpp
	src/Menu/GenderMenu.hpp
	src/DialogMgr.cpp
	src/DialogMgr.hpp
	src/Menu/DialogMenu.cpp
	src/Menu/DialogMenu.hpp
	src/Menu/VictoryMenu.cpp
	src/Menu/VictoryMenu.hpp
	src/Menu/GameoverMenu.cpp
	src/Menu/GameoverMenu.hpp
)

if(WIN32)
	list(APPEND UBC_SRC icon.rc)
endif()

add_executable(UBC ${UBC_SRC})

# Include local directories
target_include_directories(UBC PRIVATE nlohmann)

# Link libraries
if(TARGET TGUI::TGUI)
    target_link_libraries(UBC PRIVATE sfml-graphics sfml-audio sfml-window sfml-system TGUI::TGUI)
elseif(TARGET tgui)
    target_link_libraries(UBC PRIVATE sfml-graphics sfml-audio sfml-window sfml-system tgui)
else()
    target_link_libraries(UBC PRIVATE sfml-graphics sfml-audio sfml-window sfml-system TGUI)
endif()

# Copy runtime assets to the build directory after each build
add_custom_command(TARGET UBC POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/gui"
        "$<TARGET_FILE_DIR:UBC>/gui"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:UBC>/assets"
    COMMENT "Copying assets and gui to build directory"
)

# Compiler options
if(MSVC)
    target_compile_options(UBC PRIVATE /W4)
    # Release: no console window, optimized (generator expression for multi-config generators)
    set_target_properties(UBC PROPERTIES
        WIN32_EXECUTABLE $<CONFIG:Release>
    )
    target_link_options(UBC PRIVATE $<$<CONFIG:Release>:/SUBSYSTEM:WINDOWS> $<$<CONFIG:Release>:/ENTRY:mainCRTStartup>)
    target_compile_options(UBC PRIVATE $<$<CONFIG:Release>:/O2>)
else()
    target_compile_options(UBC PRIVATE -Wall -Wextra)
    if(WIN32)
        target_link_options(UBC PRIVATE $<$<CONFIG:Release>:-mwindows>)
    endif()
endif()