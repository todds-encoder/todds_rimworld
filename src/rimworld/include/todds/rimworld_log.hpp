/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "todds/string.hpp"

namespace rimworld::log {

void initialize();
void info(const todds::string& message);
void warn(const todds::string& message);
void error(const todds::string& message);

todds::string full_log();

} // namespace rimworld::log
