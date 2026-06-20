#pragma once

#include "documents/IDocument.h"

#include "desc/SignalDesc.h"

#include "core/InputListener.h"

#include "assets/MetaSplitter.h"
#include "util/Counter.h"

namespace oly::editor
{
	class SignalDocument : public IDocument
	{
		SignalFullDesc _scratch;
		SignalFullDesc _disk;
		detail::MetaMap _meta;
		gui::ListModel _signal_slots;
		gui::ListModel _route_slots;

		ListenMode _listen_mode = ListenMode::None;
		bool _stop_listening = true;

	public:
		using IDocument::IDocument;

		static const char* GetVersion();

		void Init() override;
		void Draw() override;
		void Load() override;
		void Dump() override;

	private:
		void DrawSignals();
		void DrawRoutes();
		Counter<std::string> GetSignalIDCounter() const;
		Counter<std::string> GetRouteIDCounter() const;
		Counter<std::string> GetIDCounter() const;

		void Draw(SignalDesc& desc);
		void Draw(RouteDesc& desc);
		void Draw(KeyDesc& desc);
		void Draw(MouseButtonDesc& desc);
		void Draw(GamepadButtonDesc& desc);
		void Draw(GamepadAxis1DDesc& desc);
		void Draw(GamepadAxis2DDesc& desc);
		void Draw(CursorPosDesc& desc);
		void Draw(ScrollDesc& desc);
		void Draw(Modifier0dDesc& desc);
		void Draw(Modifier1dDesc& desc);
		void Draw(Modifier2dDesc& desc);
		void Draw(ModifierBaseDesc& desc);

		void Load(TOMLNode node, SignalFullDesc& desc);
		void Load(TOMLNode node, SignalDesc& desc);
		void Load(TOMLNode node, RouteDesc& desc);
		void Load(TOMLNode node, KeyDesc& desc);
		void Load(TOMLNode node, MouseButtonDesc& desc);
		void Load(TOMLNode node, GamepadButtonDesc& desc);
		void Load(TOMLNode node, GamepadAxis1DDesc& desc);
		void Load(TOMLNode node, GamepadAxis2DDesc& desc);
		void Load(TOMLNode node, CursorPosDesc& desc);
		void Load(TOMLNode node, ScrollDesc& desc);
		void Load(TOMLNode node, Modifier0dDesc& desc);
		void Load(TOMLNode node, Modifier1dDesc& desc);
		void Load(TOMLNode node, Modifier2dDesc& desc);
		void Load(TOMLNode node, ModifierBaseDesc& desc);

		void Dump(toml::table& table, SignalFullDesc& desc);
		void Dump(toml::table& table, SignalDesc& desc);
		void Dump(toml::table& table, RouteDesc& desc);
		void Dump(toml::table& table, KeyDesc& desc);
		void Dump(toml::table& table, MouseButtonDesc& desc);
		void Dump(toml::table& table, GamepadButtonDesc& desc);
		void Dump(toml::table& table, GamepadAxis1DDesc& desc);
		void Dump(toml::table& table, GamepadAxis2DDesc& desc);
		void Dump(toml::table& table, CursorPosDesc& desc);
		void Dump(toml::table& table, ScrollDesc& desc);
		void Dump(toml::table& table, Modifier0dDesc& desc);
		void Dump(toml::table& table, Modifier1dDesc& desc);
		void Dump(toml::table& table, Modifier2dDesc& desc);
		void Dump(toml::table& table, ModifierBaseDesc& desc);
	};
}
