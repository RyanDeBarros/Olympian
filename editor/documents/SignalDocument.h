#pragma once

#include "documents/IDocument.h"

#include "desc/SignalDesc.h"

#include "assets/MetaSplitter.h"

namespace oly::editor
{
	class SignalDocument : public IDocument
	{
		SignalFullDesc _scratch;
		SignalFullDesc _disk;
		detail::MetaMap _meta;

	public:
		using IDocument::IDocument;

		static const char* GetVersion();

		void Init() override;
		void Draw() override;
		void DrawMenuBar() override;
		void Load() override;
		void Dump() override;

	private:
		void DrawSignals();
		void DrawRoutes();

		void Draw(SignalDesc& desc);
		void Draw(RouteDesc& desc);

		void Load(TOMLNode node, SignalFullDesc& desc);
		void Load(TOMLNode node, SignalDesc& desc);
		void Load(TOMLNode node, RouteDesc& desc);

		void Dump(toml::table& table, SignalFullDesc& desc);
		void Dump(toml::table& table, SignalDesc& desc);
		void Dump(toml::table& table, RouteDesc& desc);
	};
}
