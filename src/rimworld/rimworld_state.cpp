/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "todds/rimworld_state.hpp"

#include "todds/rimworld_log.hpp"
#include "todds/rimworld_util.hpp"
#include "todds/task.hpp"

#include <boost/filesystem.hpp>
#include <boost/nowide/fstream.hpp>
#include <boost/predef.h>
#include <fmt/format.h>
#include <nlohmann/json.hpp>
#include <oneapi/tbb/info.h>

namespace fs = boost::filesystem;
namespace nl = nlohmann;
namespace nw = boost::nowide;

constexpr std::string_view font_size_key = "font_size";
constexpr std::string_view target_path_key = "target_path";
constexpr std::string_view process_all_files_key = "process_all_files";

std::string default_mods_folder() {
#if BOOST_OS_WINDOWS
	return R"(C:\Program Files (x86)\Steam\steamapps\workshop\content\294100)";
#else
	return "";
#endif // BOOST_OS_WINDOWS
}

todds::args::data arguments(const rimworld::execution_state& state) {
	// Values coming from state are addressed first.
	todds::args::data data{};
	// Input is assumed to be valid. The UI prevents the processing from starting in this case.
	data.input = state.target_path().first;
	data.overwrite = state.process_all_files();
	data.overwrite_new = !data.overwrite;
	data.clean = state.is_cleaning();

	// Default values without UI options.
	data.format = todds::format::type::bc1;
	data.alpha_format = todds::format::type::bc7;
	data.quality = todds::format::quality::really_slow;
	data.fix_size = true;
	data.mipmaps = true;
	data.mipmap_filter = todds::filter::type::lanczos;
	data.mipmap_blur = 0.55;
	data.scale = 100U;
	data.scale_filter = todds::filter::type::lanczos;
	data.threads = static_cast<std::size_t>(oneapi::tbb::info::default_concurrency());
	data.depth = std::numeric_limits<std::size_t>::max();
	data.overwrite_new = true;
	data.vflip = true;
	data.time = true;
	data.substring = "Textures";
	data.progress = true;

	return data;
}

namespace rimworld {

void execution_state::reset_preferences() {
	_font_size = font_size::medium;
	_process_all_files = false;
	set_target_path(default_mods_folder());
}

void execution_state::load_preferences() {
	const auto config_path = config_file_path();

	if (fs::is_regular_file(config_path)) {
		try {
			nw::ifstream ifs{config_path};
			const nl::json json = nl::json::parse(ifs, nullptr, false, true);
			if (json.is_discarded()) {
				rimworld::log::warn("Could not load config file.");
				reset_preferences();
				return;
			}
			if (json.contains(font_size_key)) { set_font_size(json.at(font_size_key)); }
			if (json.contains(target_path_key)) { set_target_path(json.at(target_path_key)); }
			if (json.contains(process_all_files_key)) { set_process_all_files(json.at(process_all_files_key)); }
		} catch (const std::exception& exc) {
			log::error(fmt::format("Could not load preferences because of an exception: {:s}", exc.what()));
		}
	}

	if (!_valid_target_path) {
		reset_target_path();
	} else {
		rimworld::log::info("Loaded preferences.");
	}
}

void execution_state::save_preferences() {
	nl::json json;
	json[font_size_key] = _font_size;
	json[target_path_key] = _target_path;
	json[process_all_files_key] = _process_all_files;
	const auto config_path = config_file_path();
	nw::ofstream ofs(config_path);
	ofs << std::setw(4U) << json << '\n';
	rimworld::log::info("Saved preferences.");
}

void execution_state::update(std::uint32_t /*elapsed_milliseconds*/) {
	if (!is_encoding() && !is_cleaning()) { return; }

	if (!_pipeline.valid()) {
		_arguments = arguments(*this);

		rimworld::log::info("Launching pipeline.");
		_pipeline = todds::run(_arguments, _stopping, _updates);
		return;
	}

	todds::report update{};
	while (_updates.try_pop(update)) {
		switch (update.type()) {
		case todds::report_type::RETRIEVING_FILES:
			rimworld::log::info("Received RETRIEVING_FILES report.");
			_retrieving_files = true;
			break;
		case todds::report_type::FILE_RETRIEVAL_TIME:
			_file_retrieval_milliseconds = update.value();
			rimworld::log::info(fmt::format("Received FILE_RETRIEVAL_TIME report: {:d}", _file_retrieval_milliseconds));
			break;
		case todds::report_type::FILE_VERBOSE: break;
		case todds::report_type::PROCESS_STARTED:
			_start_encoding_time = oneapi::tbb::tick_count::now();
			_total_files = update.value();
			rimworld::log::info(fmt::format("Received PROCESS_STARTED report: {:d}", _total_files));
			break;
		case todds::report_type::ENCODING_PROGRESS: ++_current_files; break;
		case todds::report_type::PIPELINE_ERROR:
			rimworld::log::error(fmt::format("Received PIPELINE_ERROR report: {:s}", update.data()));
			_errors.emplace_back(update.data());
			break;
		}
	}

	// The processing is not set to finished until all events have been processed.
	if (!_finished && _pipeline.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
		_finished = true;
		_encoding_time = (oneapi::tbb::tick_count::now() - _start_encoding_time).seconds();
		rimworld::log::info("Pipeline finished.");
	}
}

void execution_state::set_font_size(font_size size) noexcept { _font_size = size; }
font_size execution_state::get_font_size() const noexcept { return _font_size; }

std::pair<todds::string, bool> execution_state::target_path() const { return {_target_path, _valid_target_path}; }

void execution_state::set_target_path(const todds::string& path) {
	_target_path = path;
	_valid_target_path = fs::is_directory(_target_path);
}

void execution_state::reset_target_path() {
	rimworld::log::info("Reset preferences.");
	set_target_path(default_mods_folder());
}

bool execution_state::process_all_files() const noexcept { return _process_all_files; }

void execution_state::set_process_all_files(bool value) noexcept { _process_all_files = value; }

bool execution_state::is_encoding() const noexcept { return _encoding; }
void execution_state::set_encoding() noexcept {
	rimworld::log::info("Launching encoding process.");
	_encoding = true;
}

bool execution_state::is_cleaning() const noexcept { return _cleaning; }
void execution_state::set_cleaning() noexcept {
	rimworld::log::info("Launching cleaning process.");
	set_process_all_files(true);
	_cleaning = true;
}

bool execution_state::started() const noexcept { return _pipeline.valid(); }
bool execution_state::finished() const noexcept { return _finished; }

bool execution_state::retrieving_files() const noexcept { return _retrieving_files; }
std::size_t execution_state::file_retrieval_milliseconds() const noexcept { return _file_retrieval_milliseconds; }
std::size_t execution_state::total_files() const noexcept { return _total_files; }
std::size_t execution_state::current_files() const noexcept { return _current_files; }
double execution_state::encoding_time() const noexcept { return _encoding_time; }
const todds::vector<todds::string>& execution_state::errors() const noexcept { return _errors; }

void execution_state::stop_encoding() noexcept { _stopping = true; }
bool execution_state::is_stopping() const noexcept { return _stopping; }

void execution_state::close_window() noexcept { _close_window = true; }
bool execution_state::should_close_window() const noexcept { return _close_window; }

} // namespace rimworld
