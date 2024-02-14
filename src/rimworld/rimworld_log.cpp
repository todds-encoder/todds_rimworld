/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "todds/rimworld_log.hpp"

#include "todds/rimworld_util.hpp"

#include <boost/nowide/fstream.hpp>
#include <fmt/chrono.h>

#include <ctime>
#include <iomanip>
#include <sstream>

void add_to_stream(std::ostream& stream, const char* level, const todds::string& message) {
	stream << fmt::format("[{:%Y-%m-%d %H:%M:%S}] [{:s}] {:s}\n", fmt::localtime(std::time(nullptr)), level, message);
}

class logger final {
public:
	void add(const char* level, const todds::string& message) {
		add_to_stream(_file, level, message);
		_file.flush();
		add_to_stream(oss, level, message);
	}

	todds::string get() { return oss.str(); }

private:
	boost::nowide::ofstream _file{rimworld::config_folder() / "ToDDS_RimWorld.log"};
	std::ostringstream oss{};
};

namespace rimworld::log {

logger& get_log() {
	static logger log;
	return log;
}

void initialize() { (void)get_log(); }

void info(const todds::string& message) { get_log().add("INFO", message); }
void warn(const todds::string& message) { get_log().add("WARN", message); }
void error(const todds::string& message) { get_log().add("ERROR", message); }

todds::string full_log() { return get_log().get(); }

} // namespace rimworld::log
