from .Common import *


def constructor(ngon) -> str:
    c = write_transformer_2d(ngon, 3)
    if 'bordered' in ngon:
        c += f"\t\t\t{ngon['name']}.bordered = {'true' if ngon['bordered'] else 'false'};\n"

    c += write_vec2_vector(ngon, f'{ngon['name']}.ngon_base.points', 'points', 3)
    c += write_vec4_vector(ngon, f'{ngon['name']}.ngon_base.fill_colors', 'fill colors', 3)
    c += write_vec4_vector(ngon, f'{ngon['name']}.ngon_base.border_colors', 'border colors', 3)
    if 'border width' in ngon:
        c += f"\t\t\t{ngon['name']}.ngon_base.border_width = (float){ngon['border width']};\n"
    if 'border pivot' in ngon:
        if ngon['border pivot'] is float:
            c += f"\t\t\t{ngon['name']}.ngon_base.border_pivot = (float){ngon['border pivot']};\n"
        elif ngon['border pivot'] == 'inner':
            c += f"\t\t\t{ngon['name']}.ngon_base.border_pivot = cmath::BorderPivot::INNER;\n"
        elif ngon['border pivot'] == 'middle':
            c += f"\t\t\t{ngon['name']}.ngon_base.border_pivot = cmath::BorderPivot::MIDDLE;\n"
        elif ngon['border pivot'] == 'outer':
            c += f"\t\t\t{ngon['name']}.ngon_base.border_pivot = cmath::BorderPivot::OUTER;\n"

    return c
