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
#define get_float_default(node, name, def) (float)node[name].value<double>().value_or(def)
#define get_unsigned_int(node, name) (unsigned int)node[name].value<int64_t>().value()
#define get_string(node, name) node[name].value<std::string>().value()
#define get_bool(node, name) node[name].value<bool>().value()
#define get_bool_default(node, name, def) node[name].value<bool>().value_or(def)
#define get_float_element(arr, index) (float)(arr->get_as<double>(index) ? arr->get_as<double>(index)->get() : arr->get_as<int64_t>(index)->get())
#define get_float_element_default(arr, index, def) (float)(arr->get_as<double>(index) ? arr->get_as<double>(index)->get() :\
 (arr->get_as<int64_t>(index) ? arr->get_as<int64_t>(index)->get() : def))
#define get_vec4(node, name) glm::vec4{ get_float_element(node[name].as_array(), 0), get_float_element(node[name].as_array(), 1), get_float_element(node[name].as_array(), 2), get_float_element(node[name].as_array(), 3) }
#define get_vec4_default(node, name, def) glm::vec4{ get_float_element_default(node[name].as_array(), 0, def[0]),\
 get_float_element_default(node[name].as_array(), 1, def[1]), get_float_element_default(node[name].as_array(), 2, def[2]), get_float_element_default(node[name].as_array(), 3, def[3]) }

		// TODO make some default parameters optional

		Transform2D load_transform_2d(const toml::v3::node_view<toml::v3::node>& node)
		{
			Transform2D transform;
			if (!node)
				return transform;
			if (auto position = node["position"].as_array())
			{
				transform.position.x = get_float_element(position, 0);
				transform.position.y = get_float_element(position, 1);
			}
			if (auto rotation = node["rotation"].value<double>())
				transform.rotation = (float)rotation.value();
			if (auto scale = node["scale"].as_array())
			{
				transform.scale.x = get_float_element(scale, 0);
				transform.scale.y = get_float_element(scale, 1);
			}
			return transform;
		}

		random::bound::Function load_random_bound_function(const toml::v3::node_view<toml::v3::node>& node)
		{
			if (!node)
				return random::bound::Function{};
			std::string type = get_string(node, "type");
			if (type == "constant")
			{
				random::bound::Constant fn;
				fn.c = get_float_default(node, "c", 0.0f);
				return fn;
			}
			else if (type == "uniform")
			{
				random::bound::Uniform fn;
				fn.a = get_float_default(node, "a", -1.0f);
				fn.b = get_float_default(node, "b", 1.0f);
				return fn;
			}
			else if (type == "power spike")
			{
				random::bound::PowerSpike fn;
				fn.a        = get_float_default(node, "a", -1.0f);
				fn.b        = get_float_default(node, "b", 1.0f);
				fn.power    = get_float_default(node, "power", 1.0f);
				fn.inverted = get_bool_default(node, "inverted", false);
				return fn;
			}
			else if (type == "dual power spike")
			{
				random::bound::DualPowerSpike fn;
				fn.a        = get_float_default(node, "a", -1.0f);
				fn.b        = get_float_default(node, "b", 1.0f);
				fn.alpha    = get_float_default(node, "alpha", 1.0f);
				fn.beta     = get_float_default(node, "beta", 1.0f);
				fn.inverted = get_bool_default(node, "inverted", false);
				return fn;
			}
			else if (type == "sine")
			{
				random::bound::Sine fn;
				fn.min = get_float_default(node, "min", -1.0f);
				fn.max = get_float_default(node, "max", 1.0f);
				fn.a =   get_float_default(node, "a", 1.0f);
				fn.k =   get_float_default(node, "k", 1.0f);
				fn.b =   get_float_default(node, "b", 0.0f);
				return fn;
			}
			else if (type == "power spike array")
			{
				random::bound::PowerSpikeArray fn;
				auto spikes = node["spikes"].as_array();
				spikes->for_each([&fn](auto&& spike) {
					if constexpr (toml::is_table<decltype(spike)>)
					{
						random::bound::PowerSpikeArray::WeightedSpike spk;
						spk.pos            = get_float(spike, "pos");
						spk.w              = get_float_default(spike, "w", 1.0f);
						spk.spike.a        = get_float_default(spike, "a", -1.0f);
						spk.spike.b        = get_float_default(spike, "b", 1.0f);
						spk.spike.power    = get_float_default(spike, "power", 1.0f);
						spk.spike.inverted = get_bool_default(spike, "inverted", false);
						fn.spikes.push_back(spk);
					}
					});
				return fn;
			}
			else
				throw Error(ErrorCode::LOAD_ASSET);
		}

		random::bound::Function2D load_random_bound_function_2d(const toml::v3::node_view<toml::v3::node>& node)
		{
			return { load_random_bound_function(node["x"]), load_random_bound_function(node["y"]) };
		}

		random::domain2d::Domain load_random_domain_2d(const toml::v3::node_view<toml::v3::node>& node)
		{
			random::domain2d::Domain d;
			d.transform = load_transform_2d(node);
			if (auto polygon = node["polygon"].as_array())
			{
				std::vector<glm::vec2> points;
				polygon->for_each([&points](auto&& point) {
					if (auto pt = point.as_array())
						points.push_back({ get_float_element(pt, 0), get_float_element(pt, 1) });
					});
				d.shapes = random::domain2d::create_triangulated_domain_shapes(points);
			}
			else
			{
				auto shapes = node["shapes"].as_array();
				shapes->for_each([&d](auto&& shape) {
					if constexpr (toml::is_table<decltype(shape)>)
					{
						random::domain2d::Domain::WeightedShape shp;
						shp.w = get_float(shape, "w");
						std::string type = get_string(shape, "type");
						if (type == "rect")
						{
							shp.shape = random::domain2d::Rect{
								load_random_bound_function(shape["fnx"]),
								load_random_bound_function(shape["fny"]),
								load_transform_2d(shape["transform"])
							};
						}
						else if (type == "ellipse")
						{
							shp.shape = random::domain2d::Ellipse{
								load_random_bound_function(shape["fnr"]),
								load_random_bound_function(shape["fna"]),
								load_transform_2d(shape["transform"])
							};
						}
						else if (type == "bary triangle")
						{
							auto pta = shape["pta"].as_array();
							auto ptb = shape["ptb"].as_array();
							auto ptc = shape["ptc"].as_array();
							shp.shape = random::domain2d::BaryTriangle{
								load_random_bound_function(shape["fna"]),
								load_random_bound_function(shape["fnb"]),
								load_random_bound_function(shape["fnc"]),
								{ get_float_element(pta, 0), get_float_element(pta, 1) },
								{ get_float_element(ptb, 0), get_float_element(ptb, 1) },
								{ get_float_element(ptc, 0), get_float_element(ptc, 1) }
							};
						}
						else if (type == "ear triangle")
						{
							auto root_pt = shape["root pt"].as_array();
							auto prev_pt = shape["prev pt"].as_array();
							auto next_pt = shape["next pt"].as_array();
							shp.shape = random::domain2d::EarTriangle{
								load_random_bound_function(shape["fnd"]),
								load_random_bound_function(shape["fna"]),
								{ get_float_element(root_pt, 0), get_float_element(root_pt, 1) },
								{ get_float_element(prev_pt, 0), get_float_element(prev_pt, 1) },
								{ get_float_element(next_pt, 0), get_float_element(next_pt, 1) }
							};
						}
						else if (type == "uniform triangle")
						{
							auto pta = shape["pta"].as_array();
							auto ptb = shape["ptb"].as_array();
							auto ptc = shape["ptc"].as_array();
							shp.shape = random::domain2d::UniformTriangle{
								load_random_bound_function(shape["fna"]),
								load_random_bound_function(shape["fnb"]),
								{ get_float_element(pta, 0), get_float_element(pta, 1) },
								{ get_float_element(ptb, 0), get_float_element(ptb, 1) },
								{ get_float_element(ptc, 0), get_float_element(ptc, 1) }
							};
						}
						d.shapes.push_back(shp);
					}
					});
			}
			return d;
		}

		particles::EmitterParams load_emitter_params(const toml::v3::node_view<toml::v3::node>& node)
		{
			particles::EmitterParams params;
			if (auto one_shot = node["one shot"])
				params.one_shot = one_shot.value<bool>().value();
			if (auto period = node["period"])
				params.period = (float)period.value<double>().value();
			if (auto max_live_particles = node["max live particles"])
				params.max_live_particles = (GLuint)max_live_particles.value<toml::int64_t>().value();
			if (auto spawn_rate = node["spawn_rate"])
			{
				std::string spawn_rate_type = get_string(spawn_rate, "type");
				static const auto load_constant = [](const decltype(spawn_rate)& node) {
					particles::spawn_rate::Constant fn;
					fn.c = get_float_default(node, "c", 3.0f);
					return fn; 
					};
				static const auto load_linear = [](const decltype(spawn_rate)& node) {
					particles::spawn_rate::Linear fn;
					fn.i = get_float_default(node, "i", 60.0f);
					fn.f = get_float_default(node, "f", 0.0f);
					return fn;
					};
				static const auto load_sine = [](const decltype(spawn_rate)& node) {
					particles::spawn_rate::Sine fn;
					fn.a = get_float_default(node, "a", 1.0f);
					fn.k = get_float_default(node, "k", 1.0f);
					fn.b = get_float_default(node, "b", 0.0f);
					fn.c = get_float_default(node, "c", 0.0f);
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
							pt.a     = get_float_default(point, "a", 1.0f);
							pt.b     = get_float_default(point, "b", 1.0f);
							pt.alpha = get_float_default(point, "alpha", 1.0f);
							pt.beta  = get_float_default(point, "beta", 1.0f);
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
			if (auto lifespan = node["lifespan"])
			{
				std::string lifespan_type = get_string(lifespan, "type");
				if (lifespan_type == "constant")
				{
					particles::lifespan::Constant fn;
					fn.c = get_float_default(lifespan, "c", 0.3f);
					params.lifespan = fn;
				}
				else if (lifespan_type == "linear")
				{
					particles::lifespan::Linear fn;
					fn.i = get_float_default(lifespan, "i", 0.5f);
					fn.f = get_float_default(lifespan, "f", 0.0f);
					params.lifespan = fn;
				}
				else if (lifespan_type == "sine")
				{
					particles::lifespan::Sine fn;
					fn.a = get_float_default(lifespan, "a", 1.0f);
					fn.k = get_float_default(lifespan, "k", 1.0f);
					fn.b = get_float_default(lifespan, "b", 0.0f);
					fn.c = get_float_default(lifespan, "c", 0.0f);
					params.lifespan = fn;
				}
			}
			if (auto lifespan_offset_rng = node["lifespan_offset_rng"])
				params.lifespan_offset_rng = load_random_bound_function(lifespan_offset_rng);
			if (auto transform_rng = node["transform_rng"])
			{
				if (auto position = transform_rng["position"])
					params.transform_rng.position = load_random_domain_2d(position);
				if (auto rotation = transform_rng["rotation"])
					params.transform_rng.rotation = load_random_bound_function(rotation);
				if (auto scale = transform_rng["scale"])
					params.transform_rng.scale = load_random_bound_function_2d(scale);
			}
			if (auto velocity = node["velocity"])
				params.velocity = load_random_bound_function_2d(velocity);
			if (auto mass = node["mass"])
			{
				std::string type = get_string(mass, "type");
				if (type == "constant")
				{
					particles::mass::Constant fn;
					fn.m = get_float_default(mass, "m", 1.0f);
					fn.t_factor = get_float_default(mass, "t factor", 0.0f);
					params.mass = fn;
				}
				else if (type == "proportional")
				{
					particles::mass::Proportional fn;
					fn.m = get_float_default(mass, "m", 1.0f);
					fn.t_factor = get_float_default(mass, "t factor", 0.0f);
					params.mass = fn;
				}
			}
			if (auto acceleration = node["acceleration"])
			{
				static const auto load_acceleration = [](const toml::v3::node_view<toml::v3::node>& node) -> particles::acceleration::Function {
					std::string type = get_string(node, "type");
					if (type == "constant")
					{
						particles::acceleration::Constant fn;
						fn.a = get_float_default(node, "a", 0.0f);
						return fn;
					}
					else if (type == "force")
					{
						particles::acceleration::Force fn;
						fn.f = get_float_default(node, "f", 0.0f);
						return fn;
					}
					else if (type == "sine position")
					{
						particles::acceleration::SinePosition fn;
						fn.a = get_float_default(node, "a", 1.0f);
						fn.k = get_float_default(node, "k", 1.0f);
						fn.b = get_float_default(node, "b", 0.0f);
						return fn;
					}
					else
						throw Error(ErrorCode::LOAD_ASSET);
				};
				params.acceleration = { load_acceleration(acceleration["x"]), load_acceleration(acceleration["y"]) };
			}
			if (auto color = node["color"])
			{
				std::string type = get_string(color, "type");
				static const auto load_constant = [](const toml::v3::node_view<toml::v3::node>& node) {
					particles::color::Constant fn;
					fn.c = get_vec4_default(node, "c", glm::vec4(1.0f));
					return fn;
					};
				static const auto load_interp = [](const toml::v3::node_view<toml::v3::node>& node) {
					particles::color::Interp fn;
					fn.t1    = get_float_default(node, "t1", 0.0f);
					fn.t2    = get_float_default(node, "t2", 1.0f);
					fn.power = get_float_default(node, "power", 1.0f);
					fn.c1 = get_vec4_default(node, "c1", glm::vec4(1.0f));
					fn.c2 = get_vec4_default(node, "c2", glm::vec4(1.0f));
					return fn;
					};
				if (type == "constant")
					params.color = load_constant(color);
				else if (type == "interp")
					params.color = load_interp(color);
				else if (type == "piecewise")
				{
					particles::color::Piecewise fn;
					fn.last_color = get_vec4_default(color, "last color", glm::vec4(1.0f));
					auto subfunctions = color["subfunctions"].as_array();
					subfunctions->for_each([&fn](auto&& subfunction) {
						if constexpr (toml::is_table<decltype(subfunction)>)
						{
							particles::color::Piecewise::SubFunction subfn;
							auto t = subfunction["t"].value<double>();
							if (t)
								subfn.interval_end.t = (float)t.value();
							else
							{
								subfn.interval_end.use_index = true;
								subfn.interval_end.i = get_unsigned_int(subfunction, "i");
							}
							std::string type = get_string(subfunction, "type");
							if (type == "constant")
								subfn.fn = load_constant((toml::v3::node_view<toml::v3::node>)subfunction);
							else if (type == "interp")
								subfn.fn = load_interp((toml::v3::node_view<toml::v3::node>)subfunction);
							fn.subfunctions.push_back(std::move(subfn));
						}
						});
					params.color = fn;
				}
			}
			if (auto gradient = node["gradient"])
			{
				std::string type = get_string(gradient, "type");
				if (type == "keep")
					params.gradient = particles::gradient::Keep{};
				else if (type == "interp")
				{
					particles::gradient::Interp fn;
					fn.i     = get_vec4_default(gradient, "i", glm::vec4(1.0f));
					fn.f     = get_vec4_default(gradient, "f", glm::vec4(1.0f));
					fn.power = get_float_default(gradient, "power", 1.0f);
					params.gradient = fn;
				}
				else if (type == "multi interp")
				{
					particles::gradient::MultiInterp fn;
					fn.starting = get_vec4_default(gradient, "starting", glm::vec4(1.0f));
					fn.ending   = get_vec4_default(gradient, "ending", glm::vec4(1.0f));
					fn.power    = get_float_default(gradient, "power", 1.0f);
					auto steps = gradient["steps"].as_array();
					steps->for_each([&fn](auto&& step) {
						if constexpr (toml::is_table<decltype(step)>)
						{
							particles::gradient::MultiInterp::Step stp;
							stp.t_end = get_float(step, "t end");
							stp.col   = get_vec4_default(step, "color", glm::vec4(1.0f));
							stp.power = get_float_default(step, "power", 1.0f);
							fn.steps.push_back(stp);
						}
						});
					params.gradient = fn;
				}
			}
			return params;
		}

#undef get_float
#undef get_float_default
#undef get_unsigned_int
#undef get_string
#undef get_bool
#undef get_bool_default
#undef get_float_element
#undef get_float_element_default
#undef get_vec4
#undef get_vec4_default

	}
}
