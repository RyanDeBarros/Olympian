#include "Tilesets.h"

#include "graphics/sprites/TileSet.h"

#include "core/util/LoggerOperators.h"
#include "core/util/Loader.h"
#include "core/util/MetaSplitter.h"
#include "core/base/Definitions.h"

#include ".gen/keys/TileSet.inl"

#include ".gen/enums/StorageMode.inl"
#include ".gen/enums/rendering/tileset/Transformation.inl"

namespace oly::context
{
	namespace internal
	{
		std::unordered_map<ResourcePath, rendering::TileSetRef> tilesets;
	}

	struct TerminateTilesets
	{
		void operator()() const
		{
			internal::tilesets.clear();
		}
	};

	// TODO v7 put internal loading logic in a TileSet::load/overload
	rendering::TileSetRef load_tileset(const ResourcePath& file)
	{
		SingletonTickService<TickPhase::None, void, TerminatePhase::Graphics, TerminateTilesets>::instance();

		if (file.empty())
		{
			_OLY_ENGINE_LOG_ERROR("CONTEXT") << "Filename is empty" << LOG.nl;
			throw Error(ErrorCode::LoadAsset);
		}

		auto it = internal::tilesets.find(file);
		if (it != internal::tilesets.end())
			return it->second;

		_OLY_ENGINE_LOG_DEBUG("CONTEXT") << "Parsing tileset [" << file << "]..." << LOG.nl;

		if (!io::MetaSplitter::meta(file).has_type("tileset"))
		{
			_OLY_ENGINE_LOG_ERROR("CONTEXT") << "Meta fields do not contain tileset type" << LOG.nl;
			throw Error(ErrorCode::LoadAsset);
		}

		auto table = io::load_toml(file);
		TOMLNode toml = (TOMLNode)table;

		auto toml_assignments = io::parse_key(toml, _gen::keys::TileSet::AssignmentArray).as_array();

		std::vector<rendering::TileSet::Assignment> assignments;
		if (toml_assignments)
		{
			size_t _a_idx = 0;
			toml_assignments->for_each([&assignments, &_a_idx, &file](auto&& _node) {
				const size_t a_idx = _a_idx++;
				TOMLNode node = (TOMLNode)_node;

				auto _texture = io::parse_key(node, _gen::keys::TileSet::Texture).value<std::string>();
				if (!_texture)
				{
					_OLY_ENGINE_LOG_WARNING("CONTEXT") << "Cannot parse tileset assignment #" << a_idx
						<< " - missing " << io::key_string(_gen::keys::TileSet::Texture) << " field" << LOG.nl;
					return;
				}

				auto _config = io::parse_key(node, _gen::keys::TileSet::Configuration);
				if (!_config)
				{
					_OLY_ENGINE_LOG_WARNING("CONTEXT") << "Cannot parse tileset assignment #" << a_idx
						<< " - missing " << io::key_string(_gen::keys::TileSet::Configuration) << " field" << LOG.nl;
					return;
				}

				rendering::TileSet::Assignment assignment;

				if (auto config = io::parse<int>(_config))
				{
					if (config >= 0 && config < (int64_t)rendering::TileSet::Configuration::_c)
						assignment.config = (rendering::TileSet::Configuration)*config;
					else
					{
						_OLY_ENGINE_LOG_WARNING("CONTEXT") << "In tileset assignment #" << a_idx
							<< ", unrecognized configuration #" << *config << LOG.nl;
						return;
					}
				}
				else
				{
					_OLY_ENGINE_LOG_WARNING("CONTEXT") << "Cannot parse tileset assignment #" << a_idx
						<< " - " << io::key_string(_gen::keys::TileSet::Configuration) << " field is missing or not an int" << LOG.nl;
					return;
				}

				assignment.desc.file = ResourcePath(*_texture, file);
				if (auto uvs = io::parse<glm::vec4>(io::parse_key(node, _gen::keys::TileSet::UVvec4)))
				{
					assignment.desc.uvs.x1 = (*uvs)[0];
					assignment.desc.uvs.x2 = (*uvs)[1];
					assignment.desc.uvs.y1 = (*uvs)[2];
					assignment.desc.uvs.y2 = (*uvs)[3];
				}

				if (auto transformations = io::parse_key(node, _gen::keys::TileSet::TransformationArray).as_array())
				{
					size_t tr_idx = 0;
					for (auto& trfm : *transformations)
					{
						if (auto transformation = io::parse<unsigned int>(TOMLNode(trfm)))
						{
							// TODO v7 throughout tileset, &= is used for transformations. verify that this is correct and that it shouldn't be |=.
							try
							{
								assignment.transformation &= _gen::rendering::tileset::Transformation::val(*transformation);
							}
							catch (const std::out_of_range&)
							{
								_OLY_ENGINE_LOG_WARNING("CONTEXT") << "In tileset assignment #" << a_idx
									<< " transformation #" << tr_idx << ", unrecognized tile transformation (" << *transformation << ")" << LOG.nl;
							}
						}
						else
							_OLY_ENGINE_LOG_WARNING("CONTEXT") << "In tileset assignment #" << a_idx << ", tile transformation #" << tr_idx << " is not a string" << LOG.nl;
						++tr_idx;
					}
				}

				assignments.push_back(assignment);
				});
		}

		rendering::TileSetRef tileset(assignments);
		if (_gen::StorageMode::val(io::parse<unsigned int>(io::parse_key(toml, _gen::keys::TileSet::Storage)), StorageMode::Discard) == StorageMode::Keep)
			internal::tilesets.emplace(file, tileset);

		_OLY_ENGINE_LOG_DEBUG("CONTEXT") << "...Tileset [" << file << "] parsed" << LOG.nl;

		return tileset;
	}

	void free_tileset(const ResourcePath& file)
	{
		internal::tilesets.erase(file);
	}
}
