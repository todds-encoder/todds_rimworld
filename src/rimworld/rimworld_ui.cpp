/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "todds/rimworld_ui.hpp"

#include "todds/manrope_font.hpp"
#include "todds/rimworld_log.hpp"
#include "todds/rimworld_project.hpp"
#include "todds/rimworld_state.hpp"

#include <boost/predef.h>
#include <clipboardxx.hpp>
#include <fmt/format.h>
#include <imgui-SFML.h>
#include <imgui.h>
#include <imgui_stdlib.h>

constexpr ImU32 error_color = IM_COL32(255U, 0U, 0U, 255U);

void help_marker(const char* desc) {
	ImGui::TextDisabled("(?)");
	if (!ImGui::BeginItemTooltip()) { return; }
	ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0F);
	ImGui::TextUnformatted(desc);
	ImGui::PopTextWrapPos();
	ImGui::EndTooltip();
}

#if BOOST_OS_WINDOWS
constexpr const char* path_help_marker =
	"Path containing mods to be encoded. By default it points to the default folder for the RimWorld Steam Workshop. It "
	"must be modified if RimWorld was installed to a custom location, or when using non-Steam versions of the game. Use "
	"the reset button to return to the default value. This value will be stored and reused in the future.";
#else
constexpr const char* path_help_marker =
	"Path containing mods to be encoded. It must be set to the folder containing the RimWorld Steam Workshop or your "
	"local mods folder. Use the reset button to return to the default value. This value will be stored and reused in the "
	"future.";
#endif

namespace rimworld {

void setup_font(execution_state& state) {
	ImGui::GetIO().Fonts->Clear();
	(void)ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(
		manrope_regular_compressed_data.data(), manrope_regular_compressed_size, state.get_font_size());
	ImGui::SFML::UpdateFontTexture();
}

std::uint8_t& progress_text_state() {
	static std::uint8_t value;
	return value;
}

std::uint32_t& progress_text_timer() {
	static std::uint32_t value;
	return value;
}

void update_progress_text(std::uint32_t elapsed_milliseconds) {
	std::uint32_t& timer = progress_text_timer();
	timer += elapsed_milliseconds;
	constexpr std::uint32_t milliseconds_per_change = 500U;
	if (timer > milliseconds_per_change) {
		timer %= milliseconds_per_change;
		std::uint8_t& state = progress_text_state();
		++state;
		state %= 4U;
	}
}

todds::string progress_text(const char* original_string) {
	switch (progress_text_state()) {
	default: return original_string;
	case 1U: return fmt::format("{:s}.", original_string);
	case 2U: return fmt::format("{:s}..", original_string);
	case 3U: return fmt::format("{:s}...", original_string);
	}
}

void show_setup_interface(execution_state& state) {
	auto [path, valid_path] = state.target_path();
	ImGui::SeparatorText("Mods to process");
	if (!valid_path) { ImGui::PushStyleColor(ImGuiCol_Text, error_color); }
	ImGui::InputText("##input_path", &path);
	if (!valid_path) { ImGui::PopStyleColor(); }
	state.set_target_path(path);
	ImGui::SameLine();
	help_marker(path_help_marker);
	ImGui::SameLine();
	if (ImGui::Button("Reset")) { state.reset_target_path(); }
	ImGui::NewLine();
	ImGui::NewLine();

	int process_all_files_int = state.process_all_files() ? 1 : 0;
	ImGui::SeparatorText("Files to be processed");
	ImGui::RadioButton("Process new files only", &process_all_files_int, 0);
	ImGui::SameLine();
	ImGui::RadioButton("Process all files", &process_all_files_int, 1);
	state.set_process_all_files(process_all_files_int == 1);
	ImGui::SameLine();
	help_marker("Change this setting to process all files if you want to overwrite existing encoded textures. If process "
							"new files only is enabled, only new or modified textures will be encoded.");
	ImGui::NewLine();
	ImGui::NewLine();

	constexpr const char* encode_new = "Encode new textures";
	constexpr const char* encode_all = "Encode all textures";
	ImGui::SeparatorText("Launch");
	if (ImGui::Button(process_all_files_int == 1 ? encode_all : encode_new) && valid_path) { state.set_encoding(); }
	ImGui::SameLine();
	help_marker("Launch the encoding process. After some initial processing time, a progress bar will be shown. It is "
							"possible to pause the process at any time. Encoding textures using will make your game load faster and "
							"reduce VRAM usage. It will also make textures look better in-game at default zoom levels, and make them "
							"less blurry when zooming out. RimWorld can't load encoded textures by default. You will need to have a "
							"mod that enables DDS textures in your modlist, such as Performance Fish or Graphics Settings+.");

	ImGui::SameLine();
	if (ImGui::Button("Clean textures") && valid_path) { state.set_cleaning(); }
	ImGui::SameLine();
	help_marker("Delete any textures encoded previously. This only affects encoded versions; original textures are left "
							"untouched.");
	ImGui::NewLine();
	ImGui::NewLine();

	ImGui::SeparatorText("Font size");
	int font_size = static_cast<int>(state.get_font_size());
	const int prev_font_size = font_size;
	ImGui::SliderInt("##font_size", &font_size, 10, 64, "%d");
	if (prev_font_size != font_size) { state.set_font_size(static_cast<float>(font_size)); }
}

void show_processing_interface(execution_state& state) {
	if (!state.retrieving_files()) {
		ImGui::SeparatorText("Process started");
		return;
	}

	ImGui::SeparatorText("File retrieval");
	const auto file_retrieval_milliseconds = state.file_retrieval_milliseconds();
	if (file_retrieval_milliseconds == 0U) {
		const auto retrieval_progress_text = progress_text("In progress");
		ImGui::TextUnformatted(retrieval_progress_text.c_str());
		return;
	}
	const auto file_retrieval_seconds = static_cast<double>(file_retrieval_milliseconds) / 1000.0;
	const auto file_retrieval_str = fmt::format("File retrieval time: {:.3f} seconds.", file_retrieval_seconds);
	ImGui::TextUnformatted(file_retrieval_str.c_str());
	ImGui::NewLine();
	ImGui::NewLine();

	const auto total_files = state.total_files();
	if (total_files == 0U && !state.finished()) { return; }

	const bool is_cleaning = state.is_cleaning();
	const bool finished = state.finished();
	if (is_cleaning) {
		ImGui::SeparatorText("Texture cleaning");
		auto processing_textures_str = finished ?
																		 fmt::format("Cleaned up {:d} textures.", total_files) :
																		 progress_text(fmt::format("Cleaning up {:d} textures", total_files).c_str());
		ImGui::TextUnformatted(processing_textures_str.c_str());
	} else {
		ImGui::SeparatorText("Texture encoding");
		if (total_files != 0U) {
			const auto current_files = state.current_files();
			const float fraction = static_cast<float>(current_files) / static_cast<float>(total_files);
			const auto overlay_str = fmt::format("{:d} / {:d}", current_files, total_files);
			ImGui::ProgressBar(fraction, ImVec2{}, overlay_str.c_str());
		} else {
			ImGui::Text("There are no textures to process.");
		}
	}

	if (finished) {
		const auto encoding_time_str = fmt::format("Texture processing time: {:.3f} seconds.", state.encoding_time());
		ImGui::NewLine();
		ImGui::TextUnformatted(encoding_time_str.c_str());
	}

	if (!is_cleaning && !finished) {
		ImGui::NewLine();
		ImGui::NewLine();
		ImGui::SeparatorText("Actions");
		if (!state.is_stopping()) {
			if (ImGui::Button("Stop")) {
				rimworld::log::info("User stopped the process.");
				state.stop_encoding();
			}
			ImGui::SameLine();
			help_marker(
				"After this button is pressed, the program will finish encoding any opened textures and stop the process. "
				"Encoding can be resumed by launching the program again and choosing the \"Process new files only\" option.");
		}
	}

	if (finished) {
		ImGui::NewLine();
		ImGui::NewLine();
		ImGui::SeparatorText("Report");
		if (total_files != 0U) {
			const auto total_time_str =
				fmt::format("Total time: {:.3f} seconds.", file_retrieval_seconds + state.encoding_time());
			ImGui::NewLine();
			ImGui::TextUnformatted(total_time_str.c_str());
		}
		if (state.is_stopping()) {
			ImGui::NewLine();
			ImGui::Text("Texture processing was stopped manually.");
			ImGui::Text("To resume it, launch the program again with the \"Process new files only\" option enabled.");
		}

		ImGui::NewLine();
		if (ImGui::Button("Close")) { state.close_window(); }
		ImGui::SameLine();
		help_marker("Texture processing has finished. It is now safe to close the application.");
		ImGui::SameLine();
		if (ImGui::Button("Copy log to clipboard")) {
			clipboardxx::clipboard clip{};
			clip << rimworld::log::full_log().c_str();
		}
		ImGui::SameLine();
		help_marker("Copy the full logs of the application (including any encoding errors) to the clipboard. Useful for "
								"sharing bug reports.");

		const auto& errors = state.errors();
		if (!errors.empty()) {
			ImGui::NewLine();
			ImGui::Text("The following errors have been detected.");
			ImGui::NewLine();

			const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
			if (ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve), ImGuiChildFlags_None,
						ImGuiWindowFlags_HorizontalScrollbar)) {
				for (const auto& error : errors) { ImGui::TextUnformatted(error.c_str()); }
			}
			ImGui::EndChild();
		}
	}
}

void show_user_interface(std::uint32_t elapsed_milliseconds, execution_state& state) {
	// UI takes all available space left by SFML.
	constexpr ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
																		 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;
	ImGui::SetNextWindowPos(ImVec2(0, 0));
	auto& imgui_io = ImGui::GetIO();
	ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
	if (!ImGui::Begin(rimworld::app_name(), nullptr, flags)) { return; }
	ImGui::PushFont(imgui_io.Fonts->Fonts[0]);

	update_progress_text(elapsed_milliseconds);
	if (state.started()) {
		show_processing_interface(state);
	} else {
		show_setup_interface(state);
	}

	ImGui::PopFont();
	ImGui::End();
}

} // namespace rimworld
