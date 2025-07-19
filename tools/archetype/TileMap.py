from .Common import *


def params_constructor(tilemap, name) -> str:
    c = write_named_transformer_2d(tilemap, name, 3)

    if 'layer' in tilemap:
        for layer in tilemap['layer']:
            c += "\t\t\t{\n"
            c += "\t\t\t\treg::params::TileMap::Layer layer;\n"

            if 'tileset' in layer:
                c += f"\t\t\t\tlayer.tileset = \"{layer['tileset']}\";\n"

            if 'z' in layer:
                c += f"\t\t\t\tlayer.z = (size_t){layer['z']};\n"

            c += write_vec2_vector(layer, 'layer.tiles', 'tiles', 4)

            c += f"\t\t\t\t{name}.layers.push_back(std::move(layer));\n"
            c += "\t\t\t}\n"

    return c


def constructor(tilemap) -> str:
    return f"""
        {{
            reg::params::TileMap params;
{params_constructor(tilemap, 'params')}
            {tilemap['name']}.init(reg::load_tilemap(params));
        }}"""
