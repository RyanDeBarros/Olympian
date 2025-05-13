from . import Sprite


def constructor(sprite_nonant) -> str:
    c = Sprite.sprite_constructor(sprite_nonant['sprite'], f'{sprite_nonant['name']}.sprite_params')

    if 'nsize' in sprite_nonant:
        c += f"\t\t\t{sprite_nonant['name']}.nsize = {{ (float){sprite_nonant['nsize'][0]}, (float){sprite_nonant['nsize'][1]} }};\n"

    if 'left offset' in sprite_nonant:
        c += f"\t\t\t{sprite_nonant['name']}.offsets.x_left = (float){sprite_nonant['left offset']};\n"
    if 'right offset' in sprite_nonant:
        c += f"\t\t\t{sprite_nonant['name']}.offsets.x_right = (float){sprite_nonant['right offset']};\n"
    if 'bottom offset' in sprite_nonant:
        c += f"\t\t\t{sprite_nonant['name']}.offsets.y_bottom = (float){sprite_nonant['bottom offset']};\n"
    if 'top offset' in sprite_nonant:
        c += f"\t\t\t{sprite_nonant['name']}.offsets.y_top = (float){sprite_nonant['top offset']};\n"

    return c
