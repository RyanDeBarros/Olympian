#include "Tilesets.h"

#include "graphics/sprites/TileSet.h"

#include "core/util/LoggerOperators.h"
#include "core/util/Loader.h"
#include "core/util/Parser.h"
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

		auto toml = io::load_toml(file);
		assets::Parser parser(toml);

		std::vector<rendering::TileSet::Assignment> assignments;
		if (auto toml_assignments = parser.optional<TOMLArray>(_gen::keys::TileSet::AssignmentArray)())
		{
			size_t _a_idx = 0;
			toml_assignments->for_each([&assignments, &_a_idx, &file](auto&& node) {
				try
				{
					const size_t a_idx = _a_idx++;
					assets::Parser parser((TOMLNode)node, { "in tileset assignment #", a_idx });

					const auto texture = parser.required<std::string>(_gen::keys::TileSet::Texture)();
					const auto config = parser.required<int>(_gen::keys::TileSet::Configuration)();

					rendering::TileSet::Assignment assignment;

					if (config >= 0 && config < (int64_t)rendering::TileSet::Configuration::_c)
						assignment.config = (rendering::TileSet::Configuration)config;
					else
					{
						_OLY_ENGINE_LOG_ERROR("CONTEXT") << "unrecognized configuration (" << config << ") in tileset assignment #" << a_idx << LOG.nl;
						throw Error(ErrorCode::LoadAsset);
					}

					assignment.desc.file = ResourcePath(texture, file);
					if (auto uvs = parser.optional<glm::vec4>(_gen::keys::TileSet::UVvec4)())
					{
						assignment.desc.uvs.x1 = (*uvs)[0];
						assignment.desc.uvs.x2 = (*uvs)[1];
						assignment.desc.uvs.y1 = (*uvs)[2];
						assignment.desc.uvs.y2 = (*uvs)[3];
					}

					if (auto transformations = parser.optional<TOMLArray>(_gen::keys::TileSet::TransformationArray)())
					{
						size_t tr_idx = 0;
						for (auto& trfm : *transformations)
						{
							assets::Parser tr_parser((TOMLNode)trfm, { "in transformation #", tr_idx, " from tileset assignment #", a_idx });

							// TODO v7 throughout tileset, &= is used for transformations. verify that this is correct and that it shouldn't be |=.
							if (auto transformation = tr_parser.translate<_gen::rendering::tileset::Transformation>().optional(assets::NO_KEY)())
								assignment.transformation &= _gen::rendering::tileset::Transformation::val(*transformation);

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
		if (parser.translate<_gen::StorageMode>().defaulted(_gen::keys::TileSet::Storage)(StorageMode::Discard) == StorageMode::Keep)
			internal::tilesets.emplace(file, tileset);

		_OLY_ENGINE_LOG_DEBUG("CONTEXT") << "...Tileset [" << file << "] parsed" << LOG.nl;

		return tileset;
	}

	void free_tileset(const ResourcePath& file)
	{
		internal::tilesets.erase(file);
	}
}
