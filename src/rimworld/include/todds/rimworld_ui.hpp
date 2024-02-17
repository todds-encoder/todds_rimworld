/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <cstdint>

namespace rimworld {

class execution_state;

void initialize_interface();

void show_user_interface(std::uint32_t elapsed_milliseconds, execution_state& state);

} // namespace rimworld