from .Common import *


def sprite_constructor(sprite, name) -> str:
    c = write_named_transformer_2d(sprite, name, 3)

    if 'texture' in sprite:
        c += f"\t\t\t{name}.texture = \"{sprite['texture']}\";\n"
    if 'texture index' in sprite:
        c += f"\t\t\t{name}.texture_index = (unsigned int){sprite['texture index']};\n"
    if 'svg scale' in sprite:
        c += f"\t\t\t{name}.svg_scale = (float){sprite['svg scale']};\n"

    if 'modulation' in sprite:
        if isinstance(sprite['modulation'][0], list):
            c += f"\t\t\t{name}.modulation = rendering::ModulationRect{{\n"
            c += f"\t\t\t\tglm::vec4{{ (float){sprite['modulation'][0][0]}, (float){sprite['modulation'][0][1]}, (float){sprite['modulation'][0][2]}, (float){sprite['modulation'][0][3]} }},\n"
            c += f"\t\t\t\tglm::vec4{{ (float){sprite['modulation'][1][0]}, (float){sprite['modulation'][1][1]}, (float){sprite['modulation'][1][2]}, (float){sprite['modulation'][1][3]} }},\n"
            c += f"\t\t\t\tglm::vec4{{ (float){sprite['modulation'][2][0]}, (float){sprite['modulation'][2][1]}, (float){sprite['modulation'][2][2]}, (float){sprite['modulation'][2][3]} }},\n"
            c += f"\t\t\t\tglm::vec4{{ (float){sprite['modulation'][3][0]}, (float){sprite['modulation'][3][1]}, (float){sprite['modulation'][3][2]}, (float){sprite['modulation'][3][3]} }}\n"
            c += "\t\t\t};\n"
        else:
            c += f"\t\t\t{name}.modulation = {{ (float){sprite['modulation'][0]}, (float){sprite['modulation'][1]}, (float){sprite['modulation'][2]}, (float){sprite['modulation'][3]} }};\n"

    if 'tex coords' in sprite:
        c += f"\t\t\t{name}.tex_coords = rendering::SpriteBatch::UVRect{{\n"
        c += f"\t\t\t\tglm::vec4{{ (float){sprite['tex coords'][0][0]}, (float){sprite['tex coords'][0][1]} }},\n"
        c += f"\t\t\t\tglm::vec4{{ (float){sprite['tex coords'][1][0]}, (float){sprite['tex coords'][1][1]} }},\n"
        c += f"\t\t\t\tglm::vec4{{ (float){sprite['tex coords'][2][0]}, (float){sprite['tex coords'][2][1]} }},\n"
        c += f"\t\t\t\tglm::vec4{{ (float){sprite['tex coords'][3][0]}, (float){sprite['tex coords'][3][1]} }}\n"
        c += "\t\t\t};\n"

    if 'frame_format' in sprite:
        frame_format = sprite['frame_format']

        def write_single_frame_format():
            c = "\t\t\t\treg::params::Sprite::SingleFrameFormat frame_format;\n"
            if 'frame' in frame_format:
                c += f"\t\t\t\tframe_format.frame = (GLuint){frame_format['frame']};\n"
            return c

        def write_auto_frame_format():
            c = "\t\t\t\treg::params::Sprite::AutoFrameFormat frame_format;\n"
            if 'starting frame' in frame_format:
                c += f"\t\t\t\tframe_format.starting_frame = (GLuint){frame_format['starting frame']};\n"
            if 'speed' in frame_format:
                c += f"\t\t\t\tframe_format.speed = (float){frame_format['speed']};\n"
            return c

        def write_anim_frame_format():
            c = "\t\t\t\tgraphics::AnimFrameFormat frame_format;\n"
            if 'starting frame' in frame_format:
                c += f"\t\t\t\tframe_format.starting_frame = (GLuint){frame_format['starting frame']};\n"
            if 'num frames' in frame_format:
                c += f"\t\t\t\tframe_format.num_frames = (GLuint){frame_format['num frames']};\n"
            if 'starting time' in frame_format:
                c += f"\t\t\t\tframe_format.starting_time = (float){frame_format['starting time']};\n"
            if 'delay seconds' in frame_format:
                c += f"\t\t\t\tframe_format.delay_seconds = (float){frame_format['delay seconds']};\n"
            return c

        c += """\t\t\t{\n"""
        if 'mode' in frame_format:
            match frame_format['mode']:
                case "single":
                    c += write_single_frame_format()
                case "auto":
                    c += write_auto_frame_format()
        else:
            c += write_anim_frame_format()
        c += f"""\t\t\t\t{name}.frame_format = frame_format;
\t\t\t}}\n"""

    return c


def constructor(sprite) -> str:
    return sprite_constructor(sprite, sprite['name'])
