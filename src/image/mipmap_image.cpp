/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "png2dds/mipmap_image.hpp"

#include <cassert>

namespace png2dds {
mipmap_image::mipmap_image(std::size_t file_index, std::size_t width, std::size_t height, bool mipmaps)
	: _file_index{file_index}
	, _data{}
	, _images{} {
	std::size_t pixels_required{};

	// Handle images with a width or height of 1. The initial image should always be included, regardless of minimum_size.
	if (width > 0ULL && height > 0ULL) {
		_images.emplace_back(width, height);
		pixels_required += _images.back().padded_width() * _images.back().padded_height();
		// Prepare sizes for the next iteration.
		width >>= 1ULL;
		height >>= 1ULL;
	}

	if (mipmaps) {
		constexpr std::size_t minimum_size = 1ULL;
		while (width > minimum_size || height > minimum_size) {
			_images.emplace_back(width, height);
			pixels_required += _images.back().padded_width() * _images.back().padded_height();
			// Prepare sizes for the next iteration.
			if (width > minimum_size) { width >>= 1ULL; }
			if (height > minimum_size) { height >>= 1ULL; }
		}
	}

	if (pixels_required == 0ULL) { return; }

	// Allocate the memory required for every image in a single contiguous array.
	_data = std::vector<std::uint8_t>(pixels_required * image::bytes_per_pixel);

	std::size_t memory_start = 0ULL;
	// Point each image to its memory chunk.
	for (image& current : _images) {
		const std::size_t required_memory = current.padded_width() * current.padded_height() * image::bytes_per_pixel;
		current.set_data(std::span<std::uint8_t>(&_data[memory_start], required_memory));
		memory_start += required_memory;
	}

	assert(memory_start == _data.size());
}

[[nodiscard]] std::size_t mipmap_image::file_index() const noexcept { return _file_index; }

[[nodiscard]] std::size_t mipmap_image::mipmap_count() const noexcept { return _images.size(); }

[[nodiscard]] const png2dds::image& mipmap_image::get_image(std::size_t index) const noexcept { return _images[index]; }

[[nodiscard]] png2dds::image& mipmap_image::get_image(std::size_t index) noexcept { return _images[index]; }

[[nodiscard]] std::size_t mipmap_image::data_size() const noexcept { return _data.size(); }

} // namespace png2dds
