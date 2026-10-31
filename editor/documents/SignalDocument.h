#pragma once

#include "desc/SignalDesc.h"

#include "documents/IDocument.h"

#include "core/InputListener.h"

#include "assets/MetaSplitter.h"

#include <imp/counter.hpp>

namespace oly::editor
{
    class SignalDocument
        : public IDocument
    {
        imtk::desc::doubler<SignalFullDesc> desc_;
        detail::MetaMap meta_;
        imtk::w::owned_list_indexer signal_slots_;
        imtk::w::owned_list_indexer route_slots_;

        ListenMode listen_mode_ = ListenMode::None;
        bool stop_listening_ = true;

        imp::counter<std::string> signal_id_counter_;
        imp::counter<std::string> route_id_counter_;
        imp::counter<std::string> id_counter_;
        imp::counter<std::string> route_local_signal_id_counter_;

    public:
        SignalDocument(detail::ResourcePath oly_path);

    public:
        void InitImpl() override;
        void Draw() override;
        void LoadImpl() override;
        void DumpImpl() override;
        void ResetAssetImpl() override;
        const imtk::desc::idoubler& GetDoubleDescriptor() const override;
        imtk::desc::idoubler& GetDoubleDescriptor() override;

    public:
        static const char* GetVersion();

    private:
        void Draw(imtk::desc::vector<SignalDesc>& desc);
        void Draw(imtk::desc::vector<RouteDesc>& desc);

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
