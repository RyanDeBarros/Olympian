#pragma once

#include "documents/IDocument.h"

#include "desc/impl/ProjectDesc.h"

#include "assets/MetaSplitter.h"

namespace oly::editor
{
	class ProjectDocument : public IDocument
	{
		DoubleDescriptor<ProjectDesc> _desc;
		detail::MetaMap _meta;

	public:
		using IDocument::IDocument;
		ProjectDocument();

		static const char* GetVersion();

		void InitImpl() override;
		void Draw() override;
		void DrawMenuBar() override;
		void Load() override;
		void Dump() override;
		const IDoubleDescriptor& GetDoubleDescriptor() const override;
		IDoubleDescriptor& GetDoubleDescriptor() override;

		std::string TabName() const override;

	private:
		void Draw(DataPath path, ProjectDesc& desc);
		void Draw(DataPath path, ContextDesc& desc);
		void Draw(DataPath path, PlatformDesc& desc);
		void Draw(DataPath path, WindowDesc& desc);
		void Draw(DataPath path, ViewportDesc& desc);
		void Draw(DataPath path, WindowHintsDesc& desc);
		void Draw(DataPath path, CollisionDesc& desc);
		void Draw(DataPath path, LoggerDesc& desc);
		void Draw(DataPath path, LoggerEnableDesc& desc);
		void Draw(DataPath path, FrameRateDesc& desc);

		void Load(TOMLNode node, ProjectDesc& desc);
		void Load(TOMLNode node, ContextDesc& desc);
		void Load(TOMLNode node, PlatformDesc& desc);
		void Load(TOMLNode node, WindowDesc& desc);
		void Load(TOMLNode node, ViewportDesc& desc);
		void Load(TOMLNode node, WindowHintsDesc& desc);
		void Load(TOMLNode node, CollisionDesc& desc);
		void Load(TOMLNode node, LoggerDesc& desc);
		void Load(TOMLNode node, LoggerEnableDesc& desc);
		void Load(TOMLNode node, FrameRateDesc& desc);

		void Dump(toml::table& table, ProjectDesc& desc);
		void Dump(toml::table& table, ContextDesc& desc);
		void Dump(toml::table& table, PlatformDesc& desc);
		void Dump(toml::table& table, WindowDesc& desc);
		void Dump(toml::table& table, ViewportDesc& desc);
		void Dump(toml::table& table, WindowHintsDesc& desc);
		void Dump(toml::table& table, CollisionDesc& desc);
		void Dump(toml::table& table, LoggerDesc& desc);
		void Dump(toml::table& table, LoggerEnableDesc& desc);
		void Dump(toml::table& table, FrameRateDesc& desc);
	};
}
