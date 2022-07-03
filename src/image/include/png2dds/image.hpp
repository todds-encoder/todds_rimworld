/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef PNG2DDS_IMAGE_HPP
#define PNG2DDS_IMAGE_HPP

#include <cstdint>
#include <vector>

namespace png2dds {

/**
 * Image loaded in memory. The size of the internal buffer is padded to ensure that the number of pixels in the width
 * and the height are divisible by 4.
 */
class image final {
public:
	static constexpr std::uint8_t bytes_per_pixel = 4U;

	image(std::size_t width, std::size_t height);
	image(const image&) = delete;
	image(image&&) noexcept = default;
	image& operator=(const image&) = delete;
	image& operator=(image&&) noexcept = default;
	~image() = default;

	/**
	 * Width of the image excluding extra columns.
	 * @return Real width.
	 */
	[[nodiscard]] std::size_t width() const noexcept;

	/**
	 * Height of the image excluding extra rows.
	 * @return Real height.
	 */
	[[nodiscard]] std::size_t height() const noexcept;

	/**
	 * Width of the memory buffer.
	 * @return Buffer width.
	 */
	[[nodiscard]] std::size_t padded_width() const noexcept;

	/**
	 * Height of the memory buffer.
	 * @return Buffer height.
	 */
	[[nodiscard]] std::size_t padded_height() const noexcept;

	/**
	 * Size of the internal memory buffer in bytes. Must be padded_width * padded_height * bytes_per_pixel.
	 * @return Size.
	 */
	[[nodiscard]] std::size_t size() const noexcept;

	/**
	 * Pointer to the first position of the memory buffer of the image.
	 * @return Start of the image.
	 */
	[[nodiscard]] std::uint8_t* buffer() noexcept;

	/**
	 * Pointer to the first position of the memory buffer of the image.
	 * @return Start of the image.
	 */
	[[nodiscard]] const std::uint8_t* buffer() const noexcept;

private:
	std::size_t _padded_width;
	std::size_t _padded_height;
	std::vector<std::uint8_t> _buffer;
	std::size_t _width;
	std::size_t _height;
};

} // namespace png2dds

#endif // PNG2DDS_IMAGE_HPP