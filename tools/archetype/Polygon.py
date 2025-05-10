from Common import *


def constructor(polygon) -> str:
    c = write_transform_2d(polygon)
    c += write_vec2_vector(polygon, f'{polygon['name']}.points', 'points')
    c += write_vec4_vector(polygon, f'{polygon['name']}.colors', 'colors')
    return c
