from Common import *


def constructor(sprite) -> str:
    c = write_transform_2d(sprite)

    if 'texture' in sprite:
        c += f"\t\t{sprite['name']}.texture = \"{sprite['texture']}\";\n"
    if 'texture index' in sprite:
        c += f"\t\t{sprite['name']}.texture_index = (unsigned int){sprite['texture index']};\n"
    if 'svg scale' in sprite:
        c += f"\t\t{sprite['name']}.svg_scale = (float){sprite['svg scale']};\n"

    if 'modulation' in sprite:
        if sprite['modulation'][0] is list:
            c += f"\t\t{sprite['name']}.modulation = {{\n"
            c += f"\t\t\t{{ (float){sprite['modulation'][0][0]}, (float){sprite['modulation'][0][1]}, (float){sprite['modulation'][0][2]}, (float){sprite['modulation'][0][3]} }}, "
            c += f"\t\t\t{{ (float){sprite['modulation'][1][0]}, (float){sprite['modulation'][1][1]}, (float){sprite['modulation'][1][2]}, (float){sprite['modulation'][1][3]} }}, "
            c += f"\t\t\t{{ (float){sprite['modulation'][2][0]}, (float){sprite['modulation'][2][1]}, (float){sprite['modulation'][2][2]}, (float){sprite['modulation'][2][3]} }}, "
            c += f"\t\t\t{{ (float){sprite['modulation'][3][0]}, (float){sprite['modulation'][3][1]}, (float){sprite['modulation'][3][2]}, (float){sprite['modulation'][3][3]} }}"
            c += "\t\t};\n"
        else:
            c += f"\t\t{sprite['name']}.modulation = {{ (float){sprite['modulation'][0]}, (float){sprite['modulation'][1]}, (float){sprite['modulation'][2]}, (float){sprite['modulation'][3]} }};\n"

    if 'tex coords' in sprite:
        c += f"\t\t{sprite['name']}.tex_coords = {{\n"
        c += f"\t\t\t{{ (float){sprite['tex coords'][0][0]}, (float){sprite['tex coords'][0][1]} }}, "
        c += f"\t\t\t{{ (float){sprite['tex coords'][1][0]}, (float){sprite['tex coords'][1][1]} }}, "
        c += f"\t\t\t{{ (float){sprite['tex coords'][2][0]}, (float){sprite['tex coords'][2][1]} }}, "
        c += f"\t\t\t{{ (float){sprite['tex coords'][3][0]}, (float){sprite['tex coords'][3][1]} }}"
        c += "\t\t};\n"

    if 'frame_format' in sprite:
        frame_format = sprite['frame_format']

        def write_single_frame_format():
            c = "\t\t\treg::params::Sprite::SingleFrameFormat frame_format;\n"
            if 'frame' in frame_format:
                c += f"\t\t\tframe_format.frame = (GLuint){frame_format['frame']};\n"
            return c

        def write_auto_frame_format():
            c = "\t\t\treg::params::Sprite::AutoFrameFormat frame_format;\n"
            if 'starting frame' in frame_format:
                c += f"\t\t\tframe_format.starting_frame = (GLuint){frame_format['starting frame']};\n"
            if 'speed' in frame_format:
                c += f"\t\t\tframe_format.speed = (float){frame_format['speed']};\n"
            return c

        def write_anim_frame_format():
            c = "\t\t\tgraphics::AnimFrameFormat frame_format;\n"
            if 'starting frame' in frame_format:
                c += f"\t\t\tframe_format.starting_frame = (GLuint){frame_format['starting frame']};\n"
            if 'num frames' in frame_format:
                c += f"\t\t\tframe_format.num_frames = (GLuint){frame_format['num frames']};\n"
            if 'starting time' in frame_format:
                c += f"\t\t\tframe_format.starting_time = (float){frame_format['starting time']};\n"
            if 'delay seconds' in frame_format:
                c += f"\t\t\tframe_format.delay_seconds = (float){frame_format['delay seconds']};\n"
            return c

        c += """\t\t{\n"""
        if 'mode' in frame_format:
            match frame_format['mode']:
                case "single":
                    c += write_single_frame_format()
                case "auto":
                    c += write_auto_frame_format()
        else:
            c += write_anim_frame_format()
        c += f"""\t\t\t{sprite['name']}.frame_format = frame_format;
\t\t}}\n"""

    if 'transform_modifier' in sprite and sprite['transform_modifier']['type'] in ["shear", "pivot", "pivot-shear"]:
        transform_modifier = sprite['transform_modifier']

        def write_shear_modifier():
            c = "\t\t\tShearTransformModifier2D modifier;\n"
            if 'shearing' in transform_modifier:
                c += f"\t\t\tmodifier.shearing = {{ (float){transform_modifier['shearing'][0]}, (float){transform_modifier['shearing'][1]} }};\n"
            return c

        def write_pivot_modifier():
            c = "\t\t\tPivotTransformModifier2D modifier;\n"
            if 'pivot' in transform_modifier:
                c += f"\t\t\tmodifier.pivot = {{ (float){transform_modifier['pivot'][0]}, (float){transform_modifier['pivot'][1]} }};\n"
            if 'size' in transform_modifier:
                c += f"\t\t\tmodifier.size = {{ (float){transform_modifier['size'][0]}, (float){transform_modifier['size'][1]} }};\n"
            return c

        def write_pivot_shear_modifier():
            c = "\t\t\tPivotShearTransformModifier2D modifier;\n"
            if 'shearing' in transform_modifier:
                c += f"\t\t\tmodifier.shearing = {{ (float){transform_modifier['shearing'][0]}, (float){transform_modifier['shearing'][1]} }};\n"
            if 'pivot' in transform_modifier:
                c += f"\t\t\tmodifier.pivot = {{ (float){transform_modifier['pivot'][0]}, (float){transform_modifier['pivot'][1]} }};\n"
            if 'size' in transform_modifier:
                c += f"\t\t\tmodifier.size = {{ (float){transform_modifier['size'][0]}, (float){transform_modifier['size'][1]} }};\n"
            return c

        c += """\t\t{\n"""
        match transform_modifier['type']:
            case "shear":
                c += write_shear_modifier()
            case "pivot":
                c += write_pivot_modifier()
            case "pivot-shear":
                c += write_pivot_shear_modifier()
        c += f"""\t\t\t{sprite['name']}.modifier = modifier;
\t\t}}\n"""
    return c
