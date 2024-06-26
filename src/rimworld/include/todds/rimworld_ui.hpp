/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <cstdint>
#include <memory>

namespace rimworld {

class execution_state;

class user_interface final {
public:
	explicit user_interface(const execution_state& state);
	user_interface(const user_interface&) = delete;
	user_interface(user_interface&&) = delete;
	user_interface& operator=(const user_interface&) = delete;
	user_interface& operator=(user_interface&&) = delete;
	~user_interface();

	static void setup_style(execution_state& state);
	static void setup_font(execution_state& state);
	void show_user_interface(std::uint32_t elapsed_milliseconds, execution_state& state);

private:
	std::unique_ptr<struct ui_impl> _pimpl;
};

} // namespace rimworld