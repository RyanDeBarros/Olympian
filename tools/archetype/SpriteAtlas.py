from . import Sprite


def params_constructor(sprite_atlas, name) -> str:
    c = Sprite.params_constructor(sprite_atlas['sprite'], f'{name}.sprite_params')

    if 'starting frame' in sprite_atlas:
        c += f"\t\t\t{name}.starting_frame = (GLuint){sprite_atlas['starting frame']}"
    if 'starting time' in sprite_atlas:
        c += f"\t\t\t{name}.starting_time = (float){sprite_atlas['starting time']}"

    if 'rows' in sprite_atlas and 'cols' in sprite_atlas and 'delay seconds' in sprite_atlas:
        c += "\t\t\t{\n"
        c += f"\t\t\t\treg::params::SpriteAtlas::Frame frame{{ .rows = (GLuint){sprite_atlas['rows']}, .cols = (GLuint){sprite_atlas['cols']}, .delay_seconds = (float){sprite_atlas['delay seconds']} }};\n"
        if 'row major' in sprite_atlas:
            c += f"\t\t\t\tframe.row_major = {'true' if sprite_atlas['row major'] else 'false'};\n"
        if 'row up' in sprite_atlas:
            c += f"\t\t\t\tframe.row_up = {'true' if sprite_atlas['row up'] else 'false'};\n"
        c += f"\t\t\t\t{name}.frame = frame;\n"
        c += "\t\t\t}\n"
    elif 'static frame' in sprite_atlas:
        c += f"\t\t\t{name}.frame = reg::params::SpriteAtlas::StaticFrame{{ (GLuint){sprite_atlas['static frame']} }};\n"

    return c
    

def constructor(sprite_atlas) -> str:
    return f"""
        {{
            reg::params::SpriteAtlas params;
{params_constructor(sprite_atlas, 'params')}
            {sprite_atlas['name']}.init(reg::load_sprite_atlas(params));
        }}"""
