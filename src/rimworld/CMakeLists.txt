# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

find_package(ImGui-SFML)
find_package(nlohmann_json)
find_package(platform_folders)
find_package(unofficial-clipboardxx)

configure_file(include/todds/rimworld_project.hpp.in include/todds/rimworld_project.hpp @ONLY)
configure_file(${PROJECT_SOURCE_DIR}/thirdparty/manrope/manrope.ttf ${CMAKE_CURRENT_BINARY_DIR}/manrope.ttf COPYONLY)

add_executable(rimworld_todds
	include/todds/rimworld_log.hpp
	include/todds/rimworld_state.hpp
	include/todds/rimworld_ui.hpp
	include/todds/rimworld_util.hpp
	include/todds/ui_style.hpp
	include/todds/ui_style_candy.hpp
	rimworld_main.cpp
	rimworld_log.cpp
	rimworld_state.cpp
	rimworld_ui.cpp
	rimworld_util.cpp
	ui_style.cpp
	ui_style_candy.cpp
)

target_include_directories(rimworld_todds PRIVATE
	# todds is retained as the include folder name to avoid changes to .clang-tidy.
	$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>/include
	$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>/include
)

target_compile_options(rimworld_todds PRIVATE ${TODDS_CPP_WARNING_FLAGS})

target_compile_definitions(rimworld_todds PUBLIC IMGUI_DISABLE_OBSOLETE_FUNCTIONS)

target_link_libraries(rimworld_todds PRIVATE
	todds_project
	todds_arguments
	todds_task
	Boost::nowide
	fmt::fmt
	TBB::tbb
	ImGui-SFML::ImGui-SFML
	nlohmann_json::nlohmann_json
	sago::platform_folders
	unofficial::ClipboardXX
)

install(TARGETS rimworld_todds RUNTIME)
