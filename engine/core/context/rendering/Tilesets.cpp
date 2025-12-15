#include "Tilesets.h"

#include "graphics/sprites/TileSet.h"
#include "core/util/LoggerOperators.h"
#include "assets/Loader.h"
#include "assets/MetaSplitter.h"

namespace oly::context
{
	namespace internal
	{
		std::unordered_map<ResourcePath, rendering::TileSetRef> tilesets;
	}

	void internal::terminate_tilesets()
	{
		internal::tilesets.clear();
	}

	rendering::TileSetRef load_tileset(const ResourcePath& file)
	{
		if (file.empty())
		{
			_OLY_ENGINE_LOG_ERROR("CONTEXT") << "Filename is empty." << LOG.nl;
			throw Error(ErrorCode::LOAD_ASSET);
		}

		auto it = internal::tilesets.find(file);
		if (it != internal::tilesets.end())
			return it->second;

		_OLY_ENGINE_LOG_DEBUG("CONTEXT") << "Parsing tileset [" << file << "]..." << LOG.nl;

		if (!assets::MetaSplitter::meta(file).has_type("tileset"))
		{
			_OLY_ENGINE_LOG_ERROR("CONTEXT") << "Meta fields do not contain tileset type." << LOG.nl;
			throw Error(ErrorCode::LOAD_ASSET);
		}

		auto table = assets::load_toml(file);
		TOMLNode toml = (TOMLNode)table;

		auto toml_assignments = toml["assignment"].as_array();

		std::vector<rendering::TileSet::Assignment> assignments;
		if (toml_assignments)
		{
			size_t _a_idx = 0;
			toml_assignments->for_each([&assignments, &_a_idx, &file](auto&& _node) {
				const size_t a_idx = _a_idx++;
				TOMLNode node = (TOMLNode)_node;

				auto _texture = node["texture"].value<std::string>();
				if (!_texture)
				{
					_OLY_ENGINE_LOG_WARNING("CONTEXT") << "Cannot parse tileset assignment #" << a_idx
						<< " - missing \"texture\" field." << LOG.nl;
					return;
				}

				auto _config = node["config"];
				if (!_config)
				{
					_OLY_ENGINE_LOG_WARNING("CONTEXT") << "Cannot parse tileset assignment #" << a_idx
						<< " - missing \"config\" field." << LOG.nl;
					return;
				}

				rendering::TileSet::Assignment assignment;

				int config = 0;
				if (assets::parse_int(_config, config))
				{
					if (config >= 0 && config < (int64_t)rendering::TileSet::Configuration::_COUNT)
						assignment.config = (rendering::TileSet::Configuration)config;
					else
					{
						_OLY_ENGINE_LOG_WARNING("CONTEXT") << "In tileset assignment #" << a_idx
							<< ", unrecognized configuration #" << config << "." << LOG.nl;
						return;
					}
				}
				else
				{
					_OLY_ENGINE_LOG_WARNING("CONTEXT") << "Cannot parse tileset assignment #" << a_idx
						<< " - \"config\" field is missing or not an int." << LOG.nl;
					return;
				}

				assignment.desc.file = ResourcePath(*_texture, file);
				glm::vec4 uvs{};
				if (assets::parse_vec(node["uvs"], uvs))
				{
					assignment.desc.uvs.x1 = uvs[0];
					assignment.desc.uvs.x2 = uvs[1];
					assignment.desc.uvs.y1 = uvs[2];
					assignment.desc.uvs.y2 = uvs[3];
				}

				if (auto transformations = node["trfm"].as_array())
				{
					size_t tr_idx = 0;
					for (const auto& trfm : *transformations)
					{
						if (auto transformation = trfm.value<std::string>())
						{
							const std::string tr = transformation.value();
							if (tr == "RX")
								assignment.transformation &= rendering::TileSet::Transformation::REFLECT_X;
							else if (tr == "RY")
								assignment.transformation &= rendering::TileSet::Transformation::REFLECT_Y;
							else if (tr == "R90")
								assignment.transformation &= rendering::TileSet::Transformation::ROTATE_90;
							else if (tr == "R180")
								assignment.transformation &= rendering::TileSet::Transformation::ROTATE_180;
							else if (tr == "R270")
								assignment.transformation &= rendering::TileSet::Transformation::ROTATE_270;
							else
								_OLY_ENGINE_LOG_WARNING("CONTEXT") << "In tileset assignment #" << a_idx
								<< " transformation #" << tr_idx << ", unrecognized tile transformation \"" << tr << "\"." << LOG.nl;
						}
						else
							_OLY_ENGINE_LOG_WARNING("CONTEXT") << "In tileset assignment #" << a_idx
							<< ", tile transformation #" << tr_idx << " is not a string." << LOG.nl;
						++tr_idx;
					}
				}

				assignments.push_back(assignment);
				});
		}

		rendering::TileSetRef tileset(assignments);
		if (toml["storage"].value<std::string>().value_or("discard") == "keep")
			internal::tilesets.emplace(file, tileset);

		_OLY_ENGINE_LOG_DEBUG("CONTEXT") << "...Tileset [" << file << "] parsed." << LOG.nl;

		return tileset;
	}

	void free_tileset(const ResourcePath& file)
	{
		internal::tilesets.erase(file);
	}
}
