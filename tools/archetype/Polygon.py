from .Common import *


def constructor(polygon) -> str:
    c = write_transformer_2d(polygon, 3)
    c += write_vec2_vector(polygon, f'{polygon['name']}.points', 'points', 3)
    c += write_vec4_vector(polygon, f'{polygon['name']}.colors', 'colors', 3)
    return c
