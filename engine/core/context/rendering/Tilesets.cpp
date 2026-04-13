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
				try
				{
					const size_t a_idx = _a_idx++;
					TOMLNode node = (TOMLNode)_node;

					const auto texture = io::parse_required<std::string>(node, _gen::keys::TileSet::Texture, { "in tileset assignment #", a_idx });
					const auto config = io::parse_required<int>(node, _gen::keys::TileSet::Configuration, {"in tileset assignment #", a_idx });

					rendering::TileSet::Assignment assignment;

					if (config >= 0 && config < (int64_t)rendering::TileSet::Configuration::_c)
						assignment.config = (rendering::TileSet::Configuration)config;
					else
					{
						_OLY_ENGINE_LOG_ERROR("CONTEXT") << "unrecognized configuration (" << config << ") in tileset assignment #" << a_idx << LOG.nl;
						throw Error(ErrorCode::LoadAsset);
					}

					assignment.desc.file = ResourcePath(texture, file);
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
							if (auto transformation = io::parse_or_warn<unsigned int>(TOMLNode(trfm), { "cannot parse transformation #", tr_idx, " in tileset assignment #", a_idx }))
							{
								// TODO v7 throughout tileset, &= is used for transformations. verify that this is correct and that it shouldn't be |=.
								try
								{
									assignment.transformation &= _gen::rendering::tileset::Transformation::val(*transformation);
								}
								catch (const std::out_of_range&)
								{
									_OLY_ENGINE_LOG_WARNING("CONTEXT") << "unrecognized tile transformation(" << *transformation << ") in transformation #"
										<< tr_idx << " from tileset assignment #" << a_idx << LOG.nl;
								}
							}
							++tr_idx;
						}
					}

					assignments.push_back(assignment);
				}
				catch (const Error& e)
				{
					if (e.code != ErrorCode::LoadAsset)
						throw;
				}
				});
		}

		rendering::TileSetRef tileset(assignments);
		if (io::parse_optional_enum<_gen::StorageMode>(toml, _gen::keys::TileSet::Storage, StorageMode::Discard) == StorageMode::Keep)
			internal::tilesets.emplace(file, tileset);

		_OLY_ENGINE_LOG_DEBUG("CONTEXT") << "...Tileset [" << file << "] parsed" << LOG.nl;

		return tileset;
	}

	void free_tileset(const ResourcePath& file)
	{
		internal::tilesets.erase(file);
	}
}
