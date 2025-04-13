#include "Loader.h"

namespace oly
{
	namespace assets
	{
		toml::v3::parse_result load_toml(const char* file)
		{
			try
			{
				return toml::parse_file(file);
			}
			catch (const toml::parse_error& err)
			{
				throw Error(ErrorCode::TOML_PARSE, err.description().data());
			}
		}

		toml::v3::parse_result load_toml(const std::string& file)
		{
			try
			{
				return toml::parse_file(file);
			}
			catch (const toml::parse_error& err)
			{
				throw Error(ErrorCode::TOML_PARSE, err.description().data());
			}
		}

#define get_float(node, name) (float)node[name].value<double>().value()
#define get_unsigned_int(node, name) (unsigned int)node[name].value<int64_t>().value()
#define get_string(node, name) node[name].value<std::string>().value()
#define get_float_element(arr, index) (float)arr->get_as<double>(index)->get()

		particles::EmitterParams load_emitter_params(const toml::v3::node_view<toml::v3::node>& node)
		{
			particles::EmitterParams params;
			if (auto one_shot = node["one shot"])
				params.one_shot = one_shot.value<bool>().value();
			if (auto period = node["period"])
				params.period = (float)period.value<double>().value();
			if (auto max_live_particles = node["max live particles"])
				params.max_live_particles = max_live_particles.value<toml::int64_t>().value();
			if (auto spawn_rate = node["spawn_rate"])
			{
				std::string spawn_rate_type = get_string(spawn_rate, "type");
				static const auto load_constant = [](const decltype(spawn_rate)& node) {
					particles::spawn_rate::Constant fn;
					fn.c = get_float(node, "c");
					return fn; 
					};
				static const auto load_linear = [](const decltype(spawn_rate)& node) {
					particles::spawn_rate::Linear fn;
					fn.i = get_float(node, "i");
					fn.f = get_float(node, "f");
					return fn;
					};
				static const auto load_sine = [](const decltype(spawn_rate)& node) {
					particles::spawn_rate::Sine fn;
					fn.a = get_float(node, "a");
					fn.k = get_float(node, "k");
					fn.b = get_float(node, "b");
					fn.c = get_float(node, "c");
					return fn;
					};
				static const auto load_discrete_pulse = [](const decltype(spawn_rate)& node) {
					particles::spawn_rate::DiscretePulse fn;
					auto points = node["points"].as_array();
					points->for_each([&fn](auto&& point) {
						if constexpr (toml::is_table<decltype(point)>)
						{
							particles::spawn_rate::DiscretePulse::Point pt;
							pt.t = get_float(point, "t");
							pt.w = get_unsigned_int(point, "w");
							fn.pts.push_back(pt);
						}
						});
					return fn;
					};
				static const auto load_continuous_pulse = [](const decltype(spawn_rate)& node) {
					particles::spawn_rate::ContinuousPulse fn;
					if (auto global_multiplier = node["global multiplier"].value<double>())
						fn.global_multiplier = (float)global_multiplier.value();
					auto points = node["points"].as_array();
					points->for_each([&fn](auto&& point) {
						if constexpr (toml::is_table<decltype(point)>)
						{
							particles::spawn_rate::ContinuousPulse::Point pt;
							pt.t     = get_float(point, "t");
							pt.w     = get_unsigned_int(point, "w");
							pt.a     = get_float(point, "a");
							pt.b     = get_float(point, "b");
							pt.alpha = get_float(point, "alpha");
							pt.beta  = get_float(point, "beta");
							fn.pts.push_back(pt);
						}
						});
					return fn;
					};
				static const auto load_piecewise = [](const decltype(spawn_rate)& node) {
					particles::spawn_rate::Piecewise fn;
					auto subfunctions = node["subfunctions"].as_array();
					subfunctions->for_each([&fn](auto&& subfunction) {
						if constexpr (toml::is_table<decltype(subfunction)>)
						{
							particles::spawn_rate::Piecewise::SubFunction subfunc;
							auto interval = subfunction["interval"].as_array();
							subfunc.interval.left  = get_float_element(interval, 0);
							subfunc.interval.right = get_float_element(interval, 1);
							std::string type = get_string(subfunction, "type");
							if (type == "constant")
								subfunc.fn = load_constant((decltype(spawn_rate))subfunction);
							else if (type == "linear")
								subfunc.fn = load_linear((decltype(spawn_rate))subfunction);
							else if (type == "sine")
								subfunc.fn = load_sine((decltype(spawn_rate))subfunction);
							else if (type == "discrete pulse")
								subfunc.fn = load_discrete_pulse((decltype(spawn_rate))subfunction);
							else if (type == "continuous pulse")
								subfunc.fn = load_continuous_pulse((decltype(spawn_rate))subfunction);
							fn.subfunctions.push_back(std::move(subfunc));
						}
						});
					return fn;
					};
				if (spawn_rate_type == "constant")
					params.spawn_rate = load_constant(spawn_rate);
				else if (spawn_rate_type == "linear")
					params.spawn_rate = load_linear(spawn_rate);
				else if (spawn_rate_type == "sine")
					params.spawn_rate = load_sine(spawn_rate);
				else if (spawn_rate_type == "discrete pulse")
					params.spawn_rate = load_discrete_pulse(spawn_rate);
				else if (spawn_rate_type == "continuous pulse")
					params.spawn_rate = load_continuous_pulse(spawn_rate);
				else if (spawn_rate_type == "piecewise")
					params.spawn_rate = load_piecewise(spawn_rate);
			}
			return params;
		}

#undef get_float
#undef get_unsigned_int
#undef get_string
#undef get_float_element

	}
}
