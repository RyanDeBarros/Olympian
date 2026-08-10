#pragma once

#include "documents/IDocument.h"

#include "desc/impl/ProjectDesc.h"
#include "desc/DoubleDescriptor.h"

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
		void LoadImpl() override;
		void DumpImpl() override;
		void ResetAssetImpl() override;
		const IDoubleDescriptor& GetDoubleDescriptor() const override;
		IDoubleDescriptor& GetDoubleDescriptor() override;

		std::string TabName() const override;

	private:
		void Draw(ProjectDesc& desc);
		void Draw(ContextDesc& desc);
		void Draw(PlatformDesc& desc);
		void Draw(WindowDesc& desc);
		void Draw(ViewportDesc& desc);
		void Draw(WindowHintsDesc& desc);
		void Draw(CollisionDesc& desc);
		void Draw(LoggerDesc& desc);
		void Draw(LoggerEnableDesc& desc);
		void Draw(FrameRateDesc& desc);

		void Load(imtk::toml_node node, ProjectDesc& desc);
		void Load(imtk::toml_node node, ContextDesc& desc);
		void Load(imtk::toml_node node, PlatformDesc& desc);
		void Load(imtk::toml_node node, WindowDesc& desc);
		void Load(imtk::toml_node node, ViewportDesc& desc);
		void Load(imtk::toml_node node, WindowHintsDesc& desc);
		void Load(imtk::toml_node node, CollisionDesc& desc);
		void Load(imtk::toml_node node, LoggerDesc& desc);
		void Load(imtk::toml_node node, LoggerEnableDesc& desc);
		void Load(imtk::toml_node node, FrameRateDesc& desc);

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
