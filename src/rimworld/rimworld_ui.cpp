/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "todds/rimworld_ui.hpp"

#include "todds/rimworld_log.hpp"
#include "todds/rimworld_project.hpp"
#include "todds/rimworld_state.hpp"
#include "todds/ui_style.hpp"

#include <boost/predef.h>
#include <clipboardxx.hpp>
#include <fmt/format.h>
#include <imgui-SFML.h>
#include <imgui.h>
#include <imgui_stdlib.h>

#include <array>

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

todds::string progress_text(const char* original_string, std::uint32_t total_millis, bool finished) {
	if (finished) { return fmt::format("{:s}...", original_string); }
	constexpr std::uint32_t milliseconds_per_change = 500U;
	switch ((total_millis / milliseconds_per_change) % 4) {
	default: return original_string;
	case 1U: return fmt::format("{:s}.", original_string);
	case 2U: return fmt::format("{:s}..", original_string);
	case 3U: return fmt::format("{:s}...", original_string);
	}
}

namespace rimworld {

struct ui_impl final {
	std::uint32_t total_millis{};
	styles::style style{};
};

user_interface::user_interface(const execution_state& state)
	: _pimpl{std::make_unique<ui_impl>()} {
	_pimpl->style = state.get_style();
	styles::set_style(state.get_style());
}

user_interface::~user_interface() = default;

void user_interface::setup_style(execution_state& state) { styles::set_style(state.get_style()); }

void user_interface::setup_font(execution_state& state) {
	constexpr const char* default_font = "./manrope.ttf";
	if (!boost::filesystem::exists(default_font)) { return; }
	state.set_can_change_font_size();
	ImGui::GetIO().Fonts->Clear();
	(void)ImGui::GetIO().Fonts->AddFontFromFileTTF(default_font, state.get_font_size());
	(void)ImGui::SFML::UpdateFontTexture();
}

void show_setup_interface(execution_state& state, ui_impl& ui_data) {
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

	if (!valid_path) { ImGui::BeginDisabled(true); }
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
	if (!valid_path) { ImGui::EndDisabled(); }
	ImGui::NewLine();
	ImGui::NewLine();

	ImGui::SeparatorText("Number of threads");
	int threads = static_cast<int>(state.get_threads());
	ImGui::SliderInt("##threads", &threads, 1, static_cast<int>(state.get_max_threads()), "%d");
	state.set_threads(static_cast<std::size_t>(threads));
	ImGui::SameLine();
	help_marker("Number of threads to use during the encoding process. By default this is set to the maximum value. "
							"Reducing this value will increase the time required to finish the process, but it will also make the "
							"computer less prone to freezing while ToDDS is working.");
	ImGui::NewLine();
	ImGui::NewLine();

	ImGui::SeparatorText("User interface");
	if (state.can_change_font_size()) {
		int font_size = static_cast<int>(state.get_font_size());
		const int prev_font_size = font_size;
		ImGui::TextUnformatted("Font size");
		ImGui::SliderInt("##font_size", &font_size, 10, 64, "%d");
		if (prev_font_size != font_size) { state.set_font_size(static_cast<float>(font_size)); }
		ImGui::SameLine();
		help_marker("Font size used by ToDDS. Has no impact on encoded textures.");
		ImGui::NewLine();
	}

	ImGui::TextUnformatted("Theme");
	// Using the generic BeginCombo() API, you have full control over how to display the combo contents.
	// (your selection data could be an index, a pointer to the object, an id for the object, a flag intrusively
	// stored in the object itself, etc.)
	constexpr std::array<const char*, static_cast<std::size_t>(styles::style::num_styles)> style_labels{
		"Dark blue", "Deep dark", "Grey", "Nord"};

	const auto selected_style_index = static_cast<std::size_t>(ui_data.style);
	const char* combo_preview_value = style_labels[selected_style_index];
	if (ImGui::BeginCombo("##Style", combo_preview_value)) {
		for (std::size_t index = 0U; index < style_labels.size(); index++) {
			const bool is_selected = (selected_style_index == index);
			if (ImGui::Selectable(style_labels[index], is_selected)) {
				ui_data.style = static_cast<styles::style>(index);
				styles::set_style(ui_data.style);
				state.set_style(ui_data.style);
			}
			if (is_selected) { ImGui::SetItemDefaultFocus(); }
		}
		ImGui::EndCombo();
	}
	ImGui::SameLine();
	help_marker("User interface theme used by ToDDS. Has no impact on encoded textures.");
}

void show_processing_interface(execution_state& state, ui_impl& ui_data) {
	if (!state.retrieving_files()) {
		ImGui::SeparatorText("Process started");
		return;
	}

	ImGui::SeparatorText("File retrieval");
	const auto file_retrieval_milliseconds = state.file_retrieval_milliseconds();
	if (file_retrieval_milliseconds == 0U) {
		ImGui::TextUnformatted(progress_text("In progress", ui_data.total_millis, state.finished()).c_str());
		ImGui::TextUnformatted(
			fmt::format("Processed filesystem entries: {:d}.", state.processed_files_during_retrieval()).c_str());

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
																		 progress_text(fmt::format("Cleaning up {:d} textures", total_files).c_str(),
																			 ui_data.total_millis, state.finished());
		ImGui::TextUnformatted(processing_textures_str.c_str());
	} else {
		ImGui::SeparatorText("Texture encoding");
		if (total_files != 0U) {
			ImGui::TextUnformatted(progress_text("In progress", ui_data.total_millis, state.finished()).c_str());
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
			ImGui::PushStyleColor(ImGuiCol_Text, error_color);
			ImGui::Text("The following errors have been detected.");
			ImGui::PopStyleColor();
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

void user_interface::show_user_interface(std::uint32_t elapsed_milliseconds, execution_state& state) {
	_pimpl->total_millis += elapsed_milliseconds;

	// UI takes all available space left by SFML.
	constexpr ImGuiWindowFlags flags =
		static_cast<unsigned int>(ImGuiWindowFlags_NoDecoration) | static_cast<unsigned int>(ImGuiWindowFlags_NoMove) |
		static_cast<unsigned int>(ImGuiWindowFlags_NoResize) | static_cast<unsigned int>(ImGuiWindowFlags_NoSavedSettings);
	ImGui::SetNextWindowPos(ImVec2(0, 0));
	auto& imgui_io = ImGui::GetIO();
	ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
	if (!ImGui::Begin(rimworld::app_name(), nullptr, flags)) { return; }
	ImGui::PushFont(imgui_io.Fonts->Fonts[0]);

	if (state.started()) {
		show_processing_interface(state, *_pimpl);
	} else {
		show_setup_interface(state, *_pimpl);
	}

	ImGui::PopFont();
	ImGui::End();
}

} // namespace rimworld
