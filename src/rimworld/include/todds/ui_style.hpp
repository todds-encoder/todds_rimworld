/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

namespace rimworld::styles {

enum class style { dark_blue, deep_dark, grey, nord, num_styles };

void set_style(style style);
} // namespace rimworld::styles