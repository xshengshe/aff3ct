#ifndef FACTORY_CODEC_HPP_
#define FACTORY_CODEC_HPP_

#include <string>

#include "../../Factory.hpp"

namespace aff3ct
{
namespace factory
{
struct Codec : Factory
{
	static const std::string name;
	static const std::string prefix;

	struct parameters
	{
		virtual ~parameters() {}

		int K           = 0;
		int N           = 0;
		int N_cw        = 0;
		int tail_length = 0;
	};

	static void build_args(arg_map &req_args, arg_map &opt_args, const std::string p = prefix);
	static void store_args(const arg_val_map &vals, parameters &params, const std::string p = prefix);
	static void make_header(params_list& head_cdc, const parameters& params, const bool full = true);
};
}
}

#endif /* FACTORY_CODEC_HPP_ */