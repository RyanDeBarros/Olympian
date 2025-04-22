#pragma once

#include <unordered_map>

#include "core/Textures.h"
#include "Loader.h"

namespace oly
{
	class TextureRegistry
	{
		enum DimensionIndex
		{
			IMAGE,
			GIF
		};
		typedef std::variant<rendering::ImageDimensions, std::shared_ptr<rendering::GIFDimensions>> TextureDimensions;

		struct Registree
		{
			rendering::BindlessTextureRes texture;
			TextureDimensions dimensions;
		};

		std::unordered_map<std::string, Registree> reg;

		void load_registree(const std::string& root_dir, const assets::AssetNode& node);

	public:
		void load(const std::string& texture_registry_file) { load(texture_registry_file.c_str()); }
		void load(const char* texture_registry_file);

		rendering::BindlessTextureRes get_texture(const std::string& name) const;
		rendering::ImageDimensions get_image_dimensions(const std::string& name) const;
		std::shared_ptr<rendering::GIFDimensions> get_gif_dimensions(const std::string& name) const;

	private:
		decltype(reg)::const_iterator get_registree(const std::string& name) const;
	};
}
