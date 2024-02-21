/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "todds/arguments.hpp"

#include "todds/format.hpp"
#include "todds/project.hpp"
#include "todds/string.hpp"

#include <boost/nowide/args.hpp>
#include <fmt/format.h>
#include <oneapi/tbb/info.h>

#include <algorithm>
#include <array>
#include <charconv>
#include <sstream>
#include <string_view>

namespace fs = boost::filesystem;

namespace {

struct optional_arg {
	std::string_view name{};
	std::string_view shorter{};
	std::string_view help{};
};

consteval optional_arg optional_argument(std::string_view arg_name, std::string_view help) {
	return optional_arg{arg_name, std::string_view{arg_name.data() + 1U, arg_name.data() + 3}, help};
}

constexpr bool matches(std::string_view argument, const optional_arg& optional_arg) {
	return argument == optional_arg.shorter || argument == optional_arg.name;
}

constexpr auto clean_arg =
	optional_arg{"--clean", "-cl", "Deletes all DDS files matching input PNG files instead of encoding them."};

constexpr auto format_arg = optional_argument("--format", "DDS encoding format.");

constexpr auto alpha_format_arg = optional_arg{"--alpha-format", "-af",
	"Use a different DDS encoding format for files with alpha. Defaults to using the value in --format unconditionally."};

constexpr auto default_quality = todds::format::quality::really_slow;
constexpr auto quality_arg =
	optional_argument("--quality", "Encoder quality level, must be in [{:d}, {:d}]. Defaults to {:d}.");

constexpr auto no_mipmaps_arg = optional_arg{"--no-mipmaps", "-nm", "Disable mipmap generation."};

constexpr auto fix_size_arg =
	optional_arg{"--fix-size", "-fs", "Set image width and height to the next multiple of 4."};

constexpr auto default_mipmap_filter = todds::filter::type::lanczos;
constexpr auto mipmap_filter_arg =
	optional_arg{"--mipmap-filter", "-mf", "Filter used to resize images during mipmap generation."};

constexpr double default_mipmap_blur = 0.55;
constexpr auto mipmap_blur_arg =
	optional_arg{"--mipmap-blur", "-mb", "Blur applied during mipmap generation. Defaults to {:.2f}."};

constexpr auto scale_arg = optional_arg{"--scale", "-sc", "Scale image size by a value given in %."};

constexpr auto max_size_arg = optional_arg{
	"--max-size", "-ms", "Downscale images with a width or height larger than this threshold to fit into it."};

constexpr auto default_scale_filter = todds::filter::type::lanczos;
constexpr auto scale_filter_arg =
	optional_arg{"--scale-filter", "-sf", "Filter used to scale images when using the scale or max_size parameters."};

constexpr auto threads_arg = optional_arg{
	"--threads", "-th", "Number of threads used by the parallel pipeline, must be in [1, {:d}]. Defaults to maximum."};

constexpr std::size_t max_depth = std::numeric_limits<std::size_t>::max();
constexpr auto depth_arg =
	optional_argument("--depth", "Maximum subdirectory depth to use when looking for source files. Defaults to maximum.");

constexpr auto overwrite_arg = optional_argument("--overwrite", "Convert files even if an output file already exists.");

constexpr auto overwrite_new_arg = optional_arg{
	"--overwrite-new", "-on", "Convert files if an output file exists, but it is older than the input file."};

constexpr auto vflip_arg = optional_arg{"--vflip", "-vf", "Flip source images vertically before encoding."};

constexpr auto time_arg = optional_argument("--time", "Show total execution time.");

constexpr auto regex_arg =
	optional_argument("--regex", "Process only absolute paths matching this regular expression.");

constexpr auto substring_arg =
	optional_arg{"--substring", "-ss", "Process only absolute paths containing this substring."};

constexpr auto dry_run_arg =
	optional_arg{"--dry-run", "-dr", "Retrieve all files that would be affected but do not make any changes."};

constexpr auto progress_arg = optional_argument("--progress", "Display progress messages.");

constexpr auto verbose_arg = optional_argument("--verbose", "Display all input files of the current operation.");

constexpr auto help_arg = optional_argument("--help", "Show usage information.");

// Advanced options.
constexpr auto alpha_black_arg = optional_arg{"--bc1-alpha-black", "-bc1-ab",
	"The BC1 encoder will use 3 color blocks for blocks containing black or very dark pixels. Increases texture quality "
	"substantially, but programs using these textures must ignore the alpha channel."};

constexpr auto report_arg =
	optional_arg{"--report", "-rp", "Prints information about the encoding process of each file."};

// Positional arguments.
constexpr std::string_view input_name = "input";
constexpr std::string_view input_help =
	"Encode all PNG files inside of this folder as DDS. It can also point to a single PNG file. "
	"If this parameter points to a TXT file, it will be processed as a list of PNG files and/or directories. Entries "
	"must be on separate lines. Every listed PNG file and those inside listed directories will be encoded as DDS.";

constexpr std::string_view output_name = "output";
constexpr std::string_view output_help =
	"Write DDS files to this folder instead of creating them next to input PNGs. This argument is ignored if input "
	"points to a TXT file.";

consteval std::size_t argument_name_total_space() {
	std::size_t max_space{};
	max_space = std::max(max_space, clean_arg.name.size() + clean_arg.shorter.size() + 2UL);
	max_space = std::max(max_space, format_arg.name.size() + format_arg.shorter.size() + 2UL);
	max_space = std::max(max_space, alpha_format_arg.name.size() + alpha_format_arg.shorter.size() + 2UL);
	max_space = std::max(max_space, quality_arg.name.size() + quality_arg.shorter.size() + 2UL);
	max_space = std::max(max_space, no_mipmaps_arg.name.size() + no_mipmaps_arg.shorter.size() + 2UL);
	max_space = std::max(max_space, fix_size_arg.name.size() + fix_size_arg.shorter.size() + 2UL);
	max_space = std::max(max_space, mipmap_filter_arg.name.size() + mipmap_filter_arg.shorter.size() + 2UL);
	max_space = std::max(max_space, mipmap_blur_arg.name.size() + mipmap_blur_arg.shorter.size() + 2UL);
	max_space = std::max(max_space, scale_arg.name.size() + scale_arg.shorter.size() + 2UL);
	max_space = std::max(max_space, max_size_arg.name.size() + max_size_arg.shorter.size() + 2UL);
	max_space = std::max(max_space, scale_filter_arg.name.size() + scale_filter_arg.shorter.size() + 2UL);
	max_space = std::max(max_space, threads_arg.name.size() + threads_arg.shorter.size() + 2UL);
	max_space = std::max(max_space, depth_arg.name.size() + depth_arg.shorter.size() + 2UL);
	max_space = std::max(max_space, overwrite_arg.name.size() + overwrite_arg.shorter.size() + 2UL);
	max_space = std::max(max_space, overwrite_new_arg.name.size() + overwrite_new_arg.shorter.size() + 2UL);
	max_space = std::max(max_space, vflip_arg.name.size() + vflip_arg.shorter.size() + 2UL);
	max_space = std::max(max_space, time_arg.name.size() + time_arg.shorter.size() + 2UL);
	max_space = std::max(max_space, dry_run_arg.name.size() + dry_run_arg.shorter.size() + 2UL);
	max_space = std::max(max_space, progress_arg.name.size() + progress_arg.shorter.size() + 2UL);
	max_space = std::max(max_space, verbose_arg.name.size() + verbose_arg.shorter.size() + 2UL);
	max_space = std::max(max_space, regex_arg.name.size() + regex_arg.shorter.size() + 2UL);
	max_space = std::max(max_space, substring_arg.name.size() + substring_arg.shorter.size() + 2UL);
	max_space = std::max(max_space, help_arg.name.size() + help_arg.shorter.size() + 2UL);
	max_space = std::max(max_space, alpha_black_arg.name.size() + alpha_black_arg.shorter.size() + 2UL);
	max_space = std::max(max_space, report_arg.name.size() + report_arg.shorter.size() + 2UL);
	max_space = std::max(max_space, input_name.size());
	max_space = std::max(max_space, output_name.size());

	return max_space + 2UL;
}
void print_argument_impl(
	std::ostringstream& ostream, std::string_view arg_shorter, std::string_view arg_name, std::string_view arg_help) {
	constexpr std::size_t argument_table_size = argument_name_total_space();
	ostream << "  ";
	if (!arg_shorter.empty()) { ostream << arg_shorter << ", "; }
	ostream << arg_name;
	const auto num_spaces = argument_table_size - arg_name.size() - (arg_shorter.empty() ? 0U : arg_shorter.size() + 2UL);
	std::fill_n(std::ostream_iterator<char>(ostream), num_spaces, ' ');
	ostream << arg_help << '\n';
}

void print_positional_argument(std::ostringstream& ostream, std::string_view arg_name, std::string_view arg_help) {
	print_argument_impl(ostream, "", arg_name, arg_help);
}

void print_optional_argument(std::ostringstream& ostream, const optional_arg& argument) {
	print_argument_impl(ostream, argument.shorter, argument.name, argument.help);
}

void print_string_argument(std::ostringstream& ostream, std::string_view name, std::string_view text) {
	constexpr std::size_t argument_space = argument_name_total_space() + 6UL;
	std::fill_n(std::ostream_iterator<char>(ostream), argument_space, ' ');
	ostream << fmt::format("{:s}: {:s}\n", name, text);
}

void print_filter_options(std::ostringstream& ostream, todds::filter::type default_value) {
	const todds::string default_str = fmt::format("{:s} [Default]", todds::filter::description(default_value));
	print_string_argument(ostream, todds::filter::name(default_value), default_str);

	constexpr std::array<todds::filter::type, 5U> filter_types{
		todds::filter::type::nearest,
		todds::filter::type::linear,
		todds::filter::type::cubic,
		todds::filter::type::area,
		todds::filter::type::lanczos,
	};
	for (auto filter_type : filter_types) {
		if (filter_type == default_value) { continue; }
		print_string_argument(ostream, todds::filter::name(filter_type), todds::filter::description(filter_type));
	}
}

todds::string get_help(std::size_t max_threads) {
	std::ostringstream ostream;
	ostream << todds::project::name() << ' ' << todds::project::version() << "\n\n"
					<< todds::project::description() << "\n\n";
	ostream << "USAGE:\n  " << todds::project::name() << " [OPTIONS] " << input_name << " [" << output_name << ']'
					<< "\n\n";

	ostream << "ARGS:\n";

	print_positional_argument(ostream, input_name, input_help);
	print_positional_argument(ostream, output_name, output_help);

	ostream << "\nOPTIONS:\n";

	print_optional_argument(ostream, clean_arg);

	print_optional_argument(ostream, format_arg);
	print_string_argument(
		ostream, todds::format::name(todds::format::type::bc7), "High-quality compression supporting alpha. [Default]");
	print_string_argument(ostream, todds::format::name(todds::format::type::bc1), "Highly compressed RGB data.");
	print_string_argument(
		ostream, todds::format::name(todds::format::type::bc3), "Highly compressed data supporting alpha.");
	print_string_argument(ostream, todds::format::name(todds::format::type::png),
		"PNG format, intended for downscaling or metric calculation.");
	print_optional_argument(ostream, alpha_format_arg);
	print_string_argument(
		ostream, todds::format::name(todds::format::type::bc7), "High-quality compression supporting alpha. [Default]");
	print_string_argument(
		ostream, todds::format::name(todds::format::type::bc3), "Highly compressed data supporting alpha.");

	const todds::string quality_help =
		fmt::format(quality_arg.help, static_cast<unsigned int>(todds::format::quality::minimum),
			static_cast<unsigned int>(todds::format::quality::maximum), static_cast<unsigned int>(default_quality));
	print_argument_impl(ostream, quality_arg.shorter, quality_arg.name, quality_help);

	print_optional_argument(ostream, no_mipmaps_arg);

	print_optional_argument(ostream, fix_size_arg);

	print_optional_argument(ostream, mipmap_filter_arg);
	print_filter_options(ostream, default_mipmap_filter);

	const todds::string mipmap_blur_help = fmt::format(mipmap_blur_arg.help, default_mipmap_blur);
	print_argument_impl(ostream, mipmap_blur_arg.shorter, mipmap_blur_arg.name, mipmap_blur_help);

	print_optional_argument(ostream, scale_arg);
	print_optional_argument(ostream, max_size_arg);
	print_optional_argument(ostream, scale_filter_arg);
	print_filter_options(ostream, default_scale_filter);

	const todds::string threads_help = fmt::format(threads_arg.help, max_threads);
	print_argument_impl(ostream, threads_arg.shorter, threads_arg.name, threads_help);
	print_optional_argument(ostream, depth_arg);
	print_optional_argument(ostream, overwrite_arg);
	print_optional_argument(ostream, overwrite_new_arg);
	print_optional_argument(ostream, vflip_arg);
	print_optional_argument(ostream, time_arg);
#if defined(TODDS_REGULAR_EXPRESSIONS)
	print_optional_argument(ostream, regex_arg);
#endif // defined(TODDS_REGULAR_EXPRESSIONS)
	print_optional_argument(ostream, substring_arg);
	print_optional_argument(ostream, dry_run_arg);
	print_optional_argument(ostream, progress_arg);
	print_optional_argument(ostream, verbose_arg);
	print_optional_argument(ostream, help_arg);

	ostream << "\nADVANCED OPTIONS:\n";

	print_optional_argument(ostream, alpha_black_arg);
	print_optional_argument(ostream, report_arg);

	return std::move(ostream).str();
}

/**
 * Obtains a format value from its string representation.
 * @param argument Format string in upper-case.
 * @return Format value.
 */
todds::format::type parse_format(std::string_view argument) {
	todds::format::type format{todds::format::type::invalid};
	if (argument == todds::format::name(todds::format::type::bc1)) {
		format = todds::format::type::bc1;
	} else if (argument == todds::format::name(todds::format::type::bc3)) {
		format = todds::format::type::bc3;
	} else if (argument == todds::format::name(todds::format::type::bc7)) {
		format = todds::format::type::bc7;
	} else if (argument == todds::format::name(todds::format::type::png)) {
		format = todds::format::type::png;
	}

	return format;
}

void format_from_str(std::string_view argument, todds::args::data& parsed_arguments) {
	const todds::string argument_upper = todds::to_upper_copy(std::string{argument});
	const auto format = parse_format(argument_upper);
	if (format != todds::format::type::invalid) {
		parsed_arguments.format = format;
	} else {
		// ToDo remove support for BC1_ALPHA_BC7.
		constexpr std::string_view bc1_alpha_bc7{"BC1_ALPHA_BC7"};
		if (argument_upper == bc1_alpha_bc7) {
			parsed_arguments.format = todds::format::type::bc1;
			parsed_arguments.alpha_format = todds::format::type::bc7;
			parsed_arguments.warning_message =
				fmt::format("Using {:s} {:s} is deprecated. Use {:s} {:s} {:s} {:s} instead.\n", format_arg.name, bc1_alpha_bc7,
					format_arg.name, todds::format::name(todds::format::type::bc1), alpha_format_arg.name,
					todds::format::name(todds::format::type::bc7));
		} else {
			parsed_arguments.stop_message = fmt::format("Argument error: invalid encoding format: {:s}", argument);
		}
	}
}

void alpha_format_from_str(std::string_view argument, todds::args::data& parsed_arguments) {
	const todds::string argument_upper = todds::to_upper_copy(std::string{argument});
	const auto format = parse_format(argument_upper);
	if (format == todds::format::type::invalid || format == todds::format::type::png) {
		parsed_arguments.stop_message = fmt::format("Argument error: invalid encoding alpha format: {:s}", argument);
	} else if (!todds::format::has_alpha(format)) {
		parsed_arguments.stop_message =
			fmt::format("Argument error: chosen encoding alpha format lacks transparency support: {:s}", argument);
	} else {
		parsed_arguments.alpha_format = format;
	}
}

todds::filter::type filter_from_str(std::string_view argument, todds::args::data& parsed_arguments) {
	const todds::string argument_upper = todds::to_upper_copy(std::string{argument});
	todds::filter::type value = todds::filter::type::lanczos;
	if (argument_upper == todds::filter::name(todds::filter::type::nearest)) {
		value = todds::filter::type::nearest;
	} else if (argument_upper == todds::filter::name(todds::filter::type::cubic)) {
		value = todds::filter::type::cubic;
	} else if (argument_upper == todds::filter::name(todds::filter::type::area)) {
		value = todds::filter::type::area;
	} else if (argument_upper == todds::filter::name(todds::filter::type::lanczos)) {
		value = todds::filter::type::lanczos;
	} else {
		parsed_arguments.stop_message = fmt::format("Argument error: unsupported filter: {:s}", argument);
	}
	return value;
}

template<typename Type>
void argument_from_str(
	std::string_view argument_name, std::string_view argument, Type& value, todds::args::data& parsed_arguments) {
	const auto [_, error] = std::from_chars(argument.data(), argument.data() + argument.size(), value);
	if (error == std::errc::invalid_argument || error == std::errc::result_out_of_range) {
		parsed_arguments.stop_message = fmt::format("Argument error: {:s} must be a number.", argument_name);
	}
}

// libc++ (used on MacOS by default) lacks support for the floating-point part of P0067R5.
// Until that is addressed, it is not possible to use from_chars with variables of type double.
template<>
void argument_from_str<double>(
	std::string_view argument_name, std::string_view argument, double& value, todds::args::data& parsed_arguments) {
	try {
		value = std::stod(std::string{argument});
	} catch (const std::logic_error& exception) {
		parsed_arguments.stop_message = fmt::format(
			"Argument error: {:s} must be a floating-point number. Exception: {:s}", argument_name, exception.what());
	}
}

} // anonymous namespace

namespace todds::args {
data get(int argc, char** argv) {
	const boost::nowide::args nowide_args(argc, argv);
	todds::vector<std::string_view> arguments;
	arguments.reserve(static_cast<std::size_t>(argc));
	for (std::size_t index = 0UL; index < static_cast<std::size_t>(argc); ++index) {
		arguments.emplace_back(argv[index]);
	}

	return get(arguments);
}

data get(const todds::vector<std::string_view>& arguments) {
	data parsed_arguments{};
	// Set default values.
	parsed_arguments.format = format::type::bc7;
	parsed_arguments.alpha_format = format::type::invalid;
	parsed_arguments.mipmaps = true;
	parsed_arguments.mipmap_filter = filter::type::lanczos;
	parsed_arguments.mipmap_blur = default_mipmap_blur;
	parsed_arguments.scale = 100U;
	parsed_arguments.scale_filter = filter::type::lanczos;
	const auto max_threads = static_cast<std::size_t>(oneapi::tbb::info::default_concurrency());
	parsed_arguments.threads = max_threads;
	parsed_arguments.depth = max_depth;
	parsed_arguments.quality = default_quality;

	std::size_t index = 1UL;

	// Parse all positional arguments.
	while (parsed_arguments.stop_message.empty() && index < arguments.size()) {
		const std::string_view argument = arguments[index];
		constexpr std::string_view optional_prefix{"-"};
		if (!argument.starts_with(optional_prefix)) { break; }
		constexpr std::string_view positional_start{"--"};
		if (argument == positional_start) {
			++index;
			break;
		}

		if (matches(argument, help_arg)) {
			parsed_arguments.stop_message = get_help(max_threads);
			parsed_arguments.help = true;
			break;
		}

		const std::string_view next_argument = (index < arguments.size() - 1UL) ? arguments[index + 1UL] : "";
		if (matches(argument, clean_arg)) {
			parsed_arguments.clean = true;
		} else if (matches(argument, format_arg)) {
			++index;
			format_from_str(next_argument, parsed_arguments);
		} else if (matches(argument, alpha_format_arg)) {
			++index;
			alpha_format_from_str(next_argument, parsed_arguments);
		} else if (matches(argument, mipmap_filter_arg)) {
			++index;
			parsed_arguments.mipmap_filter = filter_from_str(next_argument, parsed_arguments);
		} else if (matches(argument, scale_filter_arg)) {
			++index;
			parsed_arguments.scale_filter = filter_from_str(next_argument, parsed_arguments);
		} else if (matches(argument, quality_arg)) {
			++index;
			unsigned int value{};
			argument_from_str(quality_arg.name, next_argument, value, parsed_arguments);
			parsed_arguments.quality = static_cast<format::quality>(value);
			if (parsed_arguments.quality > todds::format::quality::maximum) {
				parsed_arguments.stop_message = fmt::format("Argument error: Unsupported encoding quality level {:d}.",
					static_cast<unsigned int>(parsed_arguments.quality));
			}
		} else if (matches(argument, no_mipmaps_arg)) {
			parsed_arguments.mipmaps = false;
		} else if (matches(argument, fix_size_arg)) {
			parsed_arguments.fix_size = true;
		} else if (matches(argument, mipmap_blur_arg)) {
			++index;
			argument_from_str(mipmap_blur_arg.name, next_argument, parsed_arguments.mipmap_blur, parsed_arguments);
			if (parsed_arguments.mipmap_blur <= 0.0) {
				parsed_arguments.stop_message =
					fmt::format("Argument error: {:s} must be larger than zero.", mipmap_blur_arg.name);
			}
		} else if (matches(argument, scale_arg)) {
			++index;
			argument_from_str(scale_arg.name, next_argument, parsed_arguments.scale, parsed_arguments);
			parsed_arguments.scale = std::clamp<std::uint16_t>(parsed_arguments.scale, 1U, 1000U);
		} else if (matches(argument, max_size_arg)) {
			++index;
			argument_from_str(max_size_arg.name, next_argument, parsed_arguments.max_size, parsed_arguments);
		} else if (matches(argument, threads_arg)) {
			++index;
			argument_from_str(threads_arg.name, next_argument, parsed_arguments.threads, parsed_arguments);
			parsed_arguments.threads = std::clamp<std::size_t>(parsed_arguments.threads, 1ULL, max_threads);
		} else if (matches(argument, depth_arg)) {
			++index;
			argument_from_str(depth_arg.name, next_argument, parsed_arguments.depth, parsed_arguments);
			if (parsed_arguments.depth == 0UL) { parsed_arguments.depth = max_depth; }
#if defined(TODDS_REGULAR_EXPRESSIONS)
		} else if (matches(argument, regex_arg)) {
			++index;
			parsed_arguments.regex = todds::regex{next_argument};
			const auto regex_err = parsed_arguments.regex.error();
			if (!regex_err.empty()) {
				parsed_arguments.stop_message =
					fmt::format("Could not compile regular expression {:s}: {:s}", next_argument, regex_err);
			}
#endif // defined(TODDS_REGULAR_EXPRESSIONS)
		} else if (matches(argument, substring_arg)) {
			++index;
			parsed_arguments.substring = next_argument;
		} else if (matches(argument, overwrite_arg)) {
			parsed_arguments.overwrite = true;
		} else if (matches(argument, overwrite_new_arg)) {
			parsed_arguments.overwrite_new = true;
		} else if (matches(argument, vflip_arg)) {
			parsed_arguments.vflip = true;
		} else if (matches(argument, time_arg)) {
			parsed_arguments.time = true;
		} else if (matches(argument, dry_run_arg)) {
			parsed_arguments.dry_run = true;
		} else if (matches(argument, progress_arg)) {
			parsed_arguments.progress = true;
		} else if (matches(argument, verbose_arg)) {
			parsed_arguments.verbose = true;
		} else if (matches(argument, alpha_black_arg)) {
			parsed_arguments.alpha_black = true;
		} else if (matches(argument, report_arg)) {
			parsed_arguments.report = true;
		} else {
			parsed_arguments.stop_message = fmt::format("Invalid positional argument {:s}", argument);
		}

		if (parsed_arguments.overwrite && parsed_arguments.overwrite_new) {
			parsed_arguments.stop_message =
				fmt::format("{:s} and {:s} cannot be used together", overwrite_arg.name, overwrite_new_arg.name);
		}

		++index;
	}

	if (parsed_arguments.stop_message.empty()) {
		if (index < arguments.size()) {
			boost::system::error_code error_code;
			parsed_arguments.input.emplace_back(fs::canonical(arguments[index].data(), error_code));
			if (error_code) {
				parsed_arguments.stop_message =
					fmt::format("Invalid {:s} {:s}: {:s}.", input_name, arguments[index], error_code.message());
			} else if (index < arguments.size() - 1UL) {
				parsed_arguments.output = arguments[index + 1UL].data();
			}
		} else if (index == 1UL) {
			// No arguments provided.
			parsed_arguments.stop_message = get_help(max_threads);
			parsed_arguments.help = true;
		} else {
			parsed_arguments.stop_message = fmt::format("Argument error: {:s} has not been provided.", input_name);
		}
	}

	if (parsed_arguments.stop_message.empty() && parsed_arguments.format == format::type::png) {
		parsed_arguments.mipmaps = false;
		const std::string_view format_name = format::name(format::type::png);
		if (parsed_arguments.quality != default_quality) {
			parsed_arguments.warning_message =
				fmt::format("Argument error: Parameter {:s} ignored because encoding to the {:s} format is lossless.",
					quality_arg.name, format_name);
		}

		if (!parsed_arguments.output.has_value()) {
			parsed_arguments.stop_message =
				fmt::format("Argument error: {:s} parameter must be provided for format {:s} to avoid overwriting the input.",
					output_name, format_name);
		} else if (parsed_arguments.mipmap_blur != default_mipmap_blur) {
			parsed_arguments.stop_message = fmt::format(
				"Argument error: {:s} provided but format {:s} lacks mipmap support.", mipmap_blur_arg.name, format_name);
		} else if (parsed_arguments.mipmap_filter != default_mipmap_filter) {
			parsed_arguments.stop_message = fmt::format(
				"Argument error: {:s} provided but format {:s} lacks mipmap support.", mipmap_filter_arg.name, format_name);
		}
	}

	return parsed_arguments;
}

} // namespace todds::args
