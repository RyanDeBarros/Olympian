#pragma once

#include "documents/IDocument.h"

#include "desc/SignalDesc.h"

#include "core/InputListener.h"
#include "gui/Form.h"

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
		Counter<std::string> GetIDCounter() const;

		void Draw(Form& form, SignalDesc& desc);
		void Draw(Form& form, RouteDesc& desc);
		void Draw(Form& form, KeyDesc& desc);
		void Draw(Form& form, MouseButtonDesc& desc);
		void Draw(Form& form, GamepadButtonDesc& desc);
		void Draw(Form& form, GamepadAxis1DDesc& desc);
		void Draw(Form& form, GamepadAxis2DDesc& desc);
		void Draw(Form& form, CursorPosDesc& desc);
		void Draw(Form& form, ScrollDesc& desc);
		void Draw(Form& form, Modifier0dDesc& desc);
		void Draw(Form& form, Modifier1dDesc& desc);
		void Draw(Form& form, Modifier2dDesc& desc);
		void Draw(Form& form, ModifierBaseDesc& desc);

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
