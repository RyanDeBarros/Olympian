from . import Sprite


def constructor(sprite_atlas) -> str:
    c = Sprite.sprite_constructor(sprite_atlas['sprite'], f'{sprite_atlas['name']}.sprite_params')

    if 'starting frame' in sprite_atlas:
        c += f"\t\t\t{sprite_atlas['name']}.starting_frame = (GLuint){sprite_atlas['starting frame']}"
    if 'starting time' in sprite_atlas:
        c += f"\t\t\t{sprite_atlas['name']}.starting_time = (float){sprite_atlas['starting time']}"

    if 'rows' in sprite_atlas and 'cols' in sprite_atlas and 'delay seconds' in sprite_atlas:
        c += "\t\t\t{\n"
        c += f"\t\t\t\treg::params::SpriteAtlas::Frame frame{{ .rows = (GLuint){sprite_atlas['rows']}, .cols = (GLuint){sprite_atlas['cols']}, .delay_seconds = (float){sprite_atlas['delay seconds']} }};\n"
        if 'row major' in sprite_atlas:
            c += f"\t\t\t\tframe.row_major = {'true' if sprite_atlas['row major'] else 'false'};\n"
        if 'row up' in sprite_atlas:
            c += f"\t\t\t\tframe.row_up = {'true' if sprite_atlas['row up'] else 'false'};\n"
        c += f"\t\t\t\t{sprite_atlas['name']}.frame = frame;\n"
        c += "\t\t\t}\n"
    elif 'static frame' in sprite_atlas:
        c += f"\t\t\t{sprite_atlas['name']}.frame = reg::params::SpriteAtlas::StaticFrame{{ (GLuint){sprite_atlas['static frame']} }};\n"

    return c
