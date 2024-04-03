/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "todds/rimworld_log.hpp"
#include "todds/rimworld_project.hpp"
#include "todds/rimworld_state.hpp"
#include "todds/rimworld_ui.hpp"

#include <SFML/Graphics.hpp>

#include <boost/nowide/filesystem.hpp>
#include <boost/predef.h>
#include <imgui-SFML.h>

#include <array>

#if BOOST_OS_WINDOWS
#include <Windows.h>
#endif // BOOST_OS_WINDOWS

int main() {
	// Use UTF-8 as the default encoding for Boost.Filesystem and the global C++ locale.
	std::locale::global(boost::nowide::nowide_filesystem());

	rimworld::log::initialize();
	rimworld::log::info("Initialized log system.");

	// Window initialization.
	constexpr int initial_width = 1024;
	constexpr int initial_height = 800;
	sf::RenderWindow window(sf::VideoMode(initial_width, initial_height), rimworld::app_name());

#if BOOST_OS_WINDOWS
	::ShowWindow(window.getSystemHandle(), SW_MAXIMIZE);
#endif // BOOST_OS_WINDOWS

	constexpr int frame_rate_limit = 60;
	window.setFramerateLimit(frame_rate_limit);
	if (!ImGui::SFML::Init(window)) {
		rimworld::log::error("Could not initialize window.");
		return EXIT_FAILURE;
	}

	rimworld::execution_state state{};
	state.load_preferences();

	rimworld::user_interface user_interface{};
	sf::Clock delta_clock{};
	while (window.isOpen()) {
		sf::Event event{};
		while (window.pollEvent(event)) {
			ImGui::SFML::ProcessEvent(window, event);

			if (event.type == sf::Event::Closed) {
				rimworld::log::info("Processing main window close event.");
				window.close();
			}
		}
		if (state.should_update_font()) { rimworld::user_interface::setup_font(state); }

		const auto elapsed_milliseconds = static_cast<std::uint32_t>(delta_clock.getElapsedTime().asMilliseconds());
		ImGui::SFML::Update(window, delta_clock.restart());

		// The UI might make some changes to the state, but nothing that will affect the pipeline.
		user_interface.show_user_interface(elapsed_milliseconds, state);

		window.clear(sf::Color::Blue);
		ImGui::SFML::Render(window);
		window.display();
		// After UI is drawn, start the pipeline is needed, and then process all updates coming from it.
		state.update(elapsed_milliseconds);
		if (state.should_close_window()) {
			rimworld::log::info("Processing close event dictated by the state.");
			window.close();
		}
	}
	ImGui::SFML::Shutdown();

	// At this point, the pipeline will only be enabled if the user stopped the process or closed the window.
	// Stop the pipeline as soon as possible.
	state.stop_encoding();
	state.save_preferences();
	rimworld::log::info("Application shutdown.");

	return EXIT_SUCCESS;
}
