/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "todds/arguments.hpp"
#include "todds/report.hpp"
#include "todds/string.hpp"
#include "todds/vector.hpp"

#include <oneapi/tbb/info.h>
#include <oneapi/tbb/tick_count.h>

#include <cstdint>
#include <future>
#include <utility>

#include "ui_style.hpp"

namespace rimworld {

/** Representation of the encoding arguments and pipeline execution state. */
class execution_state final {
public:
	void reset_preferences();
	void load_preferences();
	void save_preferences();

	/**
	 * Updates state according to elapsed time.
	 * @param elapsed_milliseconds Elapsed milliseconds since the last call.
	 */
	void update(std::uint32_t elapsed_milliseconds);

	[[nodiscard]] bool should_update_style() noexcept;
	void set_style(styles::style style);
	[[nodiscard]] styles::style get_style() const noexcept;

	void set_can_change_font_size() noexcept;
	[[nodiscard]] bool can_change_font_size() const noexcept;
	[[nodiscard]] bool should_update_font() noexcept;
	void set_font_size(float size) noexcept;
	[[nodiscard]] float get_font_size() const noexcept;

	[[nodiscard]] std::pair<todds::string, bool> target_path() const;
	void set_target_path(const todds::string& path);
	void reset_target_path();

	[[nodiscard]] bool process_all_files() const noexcept;
	void set_process_all_files(bool value) noexcept;

	void set_threads(std::size_t threads) noexcept;
	[[nodiscard]] std::size_t get_threads() const noexcept;
	[[nodiscard]] std::size_t get_max_threads() const noexcept;
	[[nodiscard]] bool is_encoding() const noexcept;
	void set_encoding() noexcept;

	[[nodiscard]] bool is_cleaning() const noexcept;
	void set_cleaning() noexcept;

	[[nodiscard]] bool started() const noexcept;
	[[nodiscard]] bool finished() const noexcept;

	[[nodiscard]] bool retrieving_files() const noexcept;
	[[nodiscard]] std::size_t processed_files_during_retrieval() const noexcept;
	[[nodiscard]] std::size_t file_retrieval_milliseconds() const noexcept;
	[[nodiscard]] std::size_t total_files() const noexcept;
	[[nodiscard]] std::size_t current_files() const noexcept;
	[[nodiscard]] double encoding_time() const noexcept;
	[[nodiscard]] const todds::vector<todds::string>& errors() const noexcept;

	void stop_encoding() noexcept;
	[[nodiscard]] bool is_stopping() const noexcept;

	void close_window() noexcept;
	[[nodiscard]] bool should_close_window() const noexcept;

private:
	// Setup stage.
	styles::style _style{styles::style::dark_blue};
	bool _style_updated{};
	bool _can_change_font_size{};
	float _font_size{22.0F};
	bool _font_updated{};
	todds::string _target_path{};
	bool _valid_target_path{};
	bool _process_all_files{};
	bool _encoding{};
	bool _cleaning{};
	std::size_t _threads{static_cast<std::size_t>(oneapi::tbb::info::default_concurrency())};
	std::size_t _max_threads{static_cast<std::size_t>(oneapi::tbb::info::default_concurrency())};

	// Processing stage.
	oneapi::tbb::tick_count _start_encoding_time{};
	double _encoding_time{};
	todds::args::data _arguments{};
	std::future<void> _pipeline{};
	std::atomic<bool> _stopping{};
	todds::report_queue _updates{};

	bool _retrieving_files{};
	std::size_t _processed_files_during_retrieval{};
	std::size_t _file_retrieval_milliseconds{};
	std::size_t _total_files{};
	std::size_t _current_files{};
	todds::vector<todds::string> _errors{};
	bool _finished{};
	bool _close_window{};
};

} // namespace rimworld