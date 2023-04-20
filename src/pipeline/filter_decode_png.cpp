/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "filter_decode_png.hpp"

#include "todds/filter.hpp"
#include "todds/mipmap_image.hpp"
#include "todds/png.hpp"
#include "todds/profiler.hpp"
#include "todds/util.hpp"

#include <fmt/format.h>

namespace todds::pipeline::impl {

static std::unique_ptr<mipmap_image> fix_image_size(mipmap_image& original, bool mipmaps) {
	const auto& original_img = original.get_image(0UL);
	const auto new_width = util::next_divisible_by_4(original_img.width());
	const auto new_height = util::next_divisible_by_4(original_img.height());
	std::unique_ptr<mipmap_image> resized =
		std::make_unique<mipmap_image>(original.file_index(), new_width, new_height, mipmaps);

	auto& resized_img = resized->get_image(0UL);
	for (std::size_t row = 0UL; row < original_img.height(); ++row) {
		const std::uint8_t* original_start = &original_img.row_start(row);
		const std::uint8_t* original_end = original_start + original_img.width() * image::bytes_per_pixel;
		std::copy(original_start, original_end, &resized_img.row_start(row));
	}
	return resized;
}

class decode_png final {
public:
	explicit decode_png(vector<file_data>& files_data, const paths_vector& paths, bool vflip, bool mipmaps, bool fix_size,
		error_queue& errors) noexcept
		: _files_data{files_data}
		, _paths{paths}
		, _vflip{vflip}
		, _errors{errors}
		, _mipmaps{mipmaps}
		, _fix_size{fix_size} {}

	std::unique_ptr<mipmap_image> operator()(const png_file& file) const {
		TracyZoneScopedN("decode");
		TracyZoneFileIndex(file.file_index);
		std::unique_ptr<mipmap_image> result{};

		// If the data is empty, assume that load_png_file already reported an error.
		if (!file.buffer.empty()) [[likely]] {
			const std::string& path = _paths[file.file_index].first.string();
			try {
				auto& file_data = _files_data[file.file_index];
				// Load the first image of the mipmap image and reserve the memory for the rest of the images.
				result = png::decode(file.file_index, path, file.buffer, _vflip, file_data.width, file_data.height, _mipmaps);
				const auto& first = result->get_image(0UL);
				if (_fix_size && (first.width() % 4 != 0 || first.height() % 4 != 0)) [[unlikely]] {
					result = fix_image_size(*result, _mipmaps);
					const auto& new_first = result->get_image(0UL);
					file_data.width = new_first.width();
					file_data.height = new_first.height();
				}
				file_data.mipmaps = result->mipmap_count();
			} catch (const std::runtime_error& exc) {
				_errors.push(fmt::format("PNG Decoding error {:s} -> {:s}", path, exc.what()));
			}
		}

		return result;
	}

private:
	vector<file_data>& _files_data;
	const paths_vector& _paths;
	bool _vflip;
	error_queue& _errors;
	bool _mipmaps;
	bool _fix_size;
};

oneapi::tbb::filter<png_file, std::unique_ptr<mipmap_image>> decode_png_filter(vector<file_data>& files_data,
	const paths_vector& paths, bool vflip, bool mipmaps, bool fix_size, error_queue& errors) {
	return oneapi::tbb::make_filter<png_file, std::unique_ptr<mipmap_image>>(
		oneapi::tbb::filter_mode::parallel, decode_png(files_data, paths, vflip, mipmaps, fix_size, errors));
}

} // namespace todds::pipeline::impl
