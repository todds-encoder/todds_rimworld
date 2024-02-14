/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "todds/rimworld_util.hpp"

#include "todds/project.hpp"
#include "todds/rimworld_log.hpp"

#include <boost/filesystem/operations.hpp>
#include <fmt/format.h>
#include <sago/platform_folders.h>

namespace rimworld {

boost::filesystem::path config_folder() {
	boost::filesystem::path result = (boost::filesystem::path{sago::getDataHome()} / todds::project::name());
	result += "_RimWorld";
	if (!boost::filesystem::is_directory(result)) {
		boost::filesystem::create_directories(result);
		// It is not possible to log this function because it is called prior to the log initialization.
	}

	return result;
}

boost::filesystem::path config_file_path() { return config_folder() / "ToDDS_RimWorld.json"; }

} // namespace rimworld
