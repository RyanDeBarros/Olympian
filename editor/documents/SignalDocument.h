#pragma once

#include "documents/IDocument.h"

#include "desc/impl/SignalDesc.h"
#include "desc/DoubleDescriptor.h"

#include "gui/ListModel.h"

#include "core/InputListener.h"

#include "assets/MetaSplitter.h"

#include <imp/counter.hpp>

namespace oly::editor
{
	class SignalDocument : public IDocument
	{
		DoubleDescriptor<SignalFullDesc> _desc;
		detail::MetaMap _meta;
		gui::ListIndexer _signal_slots;
		gui::ListIndexer _route_slots;

		ListenMode _listen_mode = ListenMode::None;
		bool _stop_listening = true;

	public:
		SignalDocument(detail::ResourcePath oly_path);

		static const char* GetVersion();

		void InitImpl() override;
		void Draw() override;
		void LoadImpl() override;
		void DumpImpl() override;
		void ResetAssetImpl() override;
		const IDoubleDescriptor& GetDoubleDescriptor() const override;
		IDoubleDescriptor& GetDoubleDescriptor() override;

	private:
		void Draw(imtk::desc::vector<SignalDesc>& desc);
		void Draw(imtk::desc::vector<RouteDesc>& desc);
		imp::counter<std::string> GetSignalIDCounter() const;
		imp::counter<std::string> GetRouteIDCounter() const;
		imp::counter<std::string> GetIDCounter() const;

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

		void Load(imtk::toml_node node, SignalFullDesc& desc);
		void Load(imtk::toml_node node, SignalDesc& desc);
		void Load(imtk::toml_node node, RouteDesc& desc);
		void Load(imtk::toml_node node, KeyDesc& desc);
		void Load(imtk::toml_node node, MouseButtonDesc& desc);
		void Load(imtk::toml_node node, GamepadButtonDesc& desc);
		void Load(imtk::toml_node node, GamepadAxis1DDesc& desc);
		void Load(imtk::toml_node node, GamepadAxis2DDesc& desc);
		void Load(imtk::toml_node node, CursorPosDesc& desc);
		void Load(imtk::toml_node node, ScrollDesc& desc);
		void Load(imtk::toml_node node, Modifier0dDesc& desc);
		void Load(imtk::toml_node node, Modifier1dDesc& desc);
		void Load(imtk::toml_node node, Modifier2dDesc& desc);
		void Load(imtk::toml_node node, ModifierBaseDesc& desc);

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
