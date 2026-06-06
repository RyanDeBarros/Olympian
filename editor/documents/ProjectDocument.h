#pragma once

#include "documents/IDocument.h"
#include "gui/Form.h"

#include "desc/ProjectDesc.h"

#include "assets/MetaSplitter.h"

namespace oly::editor
{
	class ProjectDocument : public IDocument
	{
		ProjectDesc _scratch;
		ProjectDesc _disk;
		detail::MetaMap _meta;

	public:
		using IDocument::IDocument;

		static const char* GetVersion();

		void Init() override;
		void Draw() override;
		void DrawMenuBar() override;
		void Load() override;
		void Dump() override;

		std::string TabName() const override;

	private:
		void Draw(ProjectDesc& desc);
		void Draw(Form& form, ContextDesc& desc);
		void Draw(Form& form, LoggerDesc& desc);
		void Draw(Form& form, LoggerEnableDesc& desc);

		void Load(TOMLNode node, ProjectDesc& desc);
		void Load(TOMLNode node, ContextDesc& desc);
		void Load(TOMLNode node, LoggerDesc& desc);
		void Load(TOMLNode node, LoggerEnableDesc& desc);

		void Dump(toml::table& table, ProjectDesc& desc);
		void Dump(toml::table& table, ContextDesc& desc);
		void Dump(toml::table& table, LoggerDesc& desc);
		void Dump(toml::table& table, LoggerEnableDesc& desc);
	};
}
