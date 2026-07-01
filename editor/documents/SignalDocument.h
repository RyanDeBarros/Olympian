#pragma once

#include "documents/IDocument.h"

#include "desc/impl/SignalDesc.h"
#include "desc/DoubleDescriptor.h"

#include "core/InputListener.h"

#include "assets/MetaSplitter.h"
#include "util/Counter.h"

namespace oly::editor
{
	class SignalDocument : public IDocument
	{
		DoubleDescriptor<SignalFullDesc> _desc;
		detail::MetaMap _meta;
		gui::ListModel _signal_slots;
		gui::ListModel _route_slots;

		ListenMode _listen_mode = ListenMode::None;
		bool _stop_listening = true;

	public:
		using IDocument::IDocument;

		static const char* GetVersion();

		void InitImpl() override;
		void Draw() override;
		void LoadImpl() override;
		void DumpImpl() override;
		const IDoubleDescriptor& GetDoubleDescriptor() const override;
		IDoubleDescriptor& GetDoubleDescriptor() override;

	private:
		void Draw(DataPath path, VectorDesc<SignalDesc>& desc);
		void Draw(DataPath path, VectorDesc<RouteDesc>& desc);
		Counter<std::string> GetSignalIDCounter() const;
		Counter<std::string> GetRouteIDCounter() const;
		Counter<std::string> GetIDCounter() const;

		void Draw(DataPath path, SignalDesc& desc);
		void Draw(DataPath path, RouteDesc& desc);
		void Draw(DataPath path, KeyDesc& desc);
		void Draw(DataPath path, MouseButtonDesc& desc);
		void Draw(DataPath path, GamepadButtonDesc& desc);
		void Draw(DataPath path, GamepadAxis1DDesc& desc);
		void Draw(DataPath path, GamepadAxis2DDesc& desc);
		void Draw(DataPath path, CursorPosDesc& desc);
		void Draw(DataPath path, ScrollDesc& desc);
		void Draw(DataPath path, Modifier0dDesc& desc);
		void Draw(DataPath path, Modifier1dDesc& desc);
		void Draw(DataPath path, Modifier2dDesc& desc);
		void Draw(DataPath path, ModifierBaseDesc& desc);

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
