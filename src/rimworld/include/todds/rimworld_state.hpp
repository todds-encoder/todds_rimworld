/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "todds/arguments.hpp"
#include "todds/report.hpp"
#include "todds/string.hpp"
#include "todds/vector.hpp"

#include <oneapi/tbb/tick_count.h>

#include <cstdint>
#include <future>
#include <utility>

namespace rimworld {

enum class font_size {
	small = 1U,
	medium,
	large,
	huge,
};

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

	void set_font_size(font_size size) noexcept;
	font_size get_font_size() const noexcept;

	[[nodiscard]] std::pair<todds::string, bool> target_path() const;
	void set_target_path(const todds::string& path);
	void reset_target_path();

	bool process_all_files() const noexcept;
	void set_process_all_files(bool value) noexcept;

	bool is_encoding() const noexcept;
	void set_encoding() noexcept;

	bool is_cleaning() const noexcept;
	void set_cleaning() noexcept;

	bool started() const noexcept;
	bool finished() const noexcept;

	bool retrieving_files() const noexcept;
	std::size_t file_retrieval_milliseconds() const noexcept;
	std::size_t total_files() const noexcept;
	std::size_t current_files() const noexcept;
	double encoding_time() const noexcept;
	const todds::vector<todds::string>& errors() const noexcept;

	void stop_encoding() noexcept;
	bool is_stopping() const noexcept;

	void close_window() noexcept;
	bool should_close_window() const noexcept;

private:
	// Setup stage.
	font_size _font_size{font_size::medium};
	todds::string _target_path{};
	bool _valid_target_path{};
	bool _process_all_files{};
	bool _encoding{};
	bool _cleaning{};

	// Processing stage.
	oneapi::tbb::tick_count _start_encoding_time{};
	double _encoding_time{};
	todds::args::data _arguments{};
	std::future<void> _pipeline{};
	std::atomic<bool> _stopping{};
	todds::report_queue _updates{};

	bool _retrieving_files{};
	std::size_t _file_retrieval_milliseconds{};
	std::size_t _total_files{};
	std::size_t _current_files{};
	todds::vector<todds::string> _errors{};
	bool _finished{};
	bool _close_window{};
};

} // namespace rimworld