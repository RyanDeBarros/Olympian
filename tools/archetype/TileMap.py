from Common import *


def constructor(tilemap) -> str:
    c = write_transform_2d(tilemap)

    if 'layer' in tilemap:
        for layer in tilemap['layer']:
            c += "\t\t{\n"
            c += "\t\t\treg::params::TileMap::Layer layer;\n"

            if 'tileset' in layer:
                c += f"\t\t\tlayer.tileset = \"{layer['tileset']}\";\n"

            if 'z' in layer:
                c += f"\t\t\tlayer.z = (size_t){layer['z']};\n"

            c += write_vec2_vector(layer, 'layer.tiles', 'tiles', 3)

            c += f"\t\t\t{tilemap['name']}.layers.push_back(std::move(layer));\n"
            c += "\t\t}\n"

    return c
