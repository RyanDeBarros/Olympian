from .Common import *


def constructor(tilemap) -> str:
    c = write_transformer_2d(tilemap, 3)

    if 'layer' in tilemap:
        for layer in tilemap['layer']:
            c += "\t\t\t{\n"
            c += "\t\t\t\treg::params::TileMap::Layer layer;\n"

            if 'tileset' in layer:
                c += f"\t\t\t\tlayer.tileset = \"{layer['tileset']}\";\n"

            if 'z' in layer:
                c += f"\t\t\t\tlayer.z = (size_t){layer['z']};\n"

            c += write_vec2_vector(layer, 'layer.tiles', 'tiles', 4)

            c += f"\t\t\t\t{tilemap['name']}->layers.push_back(std::move(layer));\n"
            c += "\t\t\t}\n"

    return c
