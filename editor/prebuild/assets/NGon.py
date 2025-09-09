from .Common import *


def params_constructor(ngon, name) -> str:
    c = write_named_transformer_2d(ngon, name, 3)
    if 'bordered' in ngon:
        c += f"\t\t\t{name}.bordered = {'true' if ngon['bordered'] else 'false'};\n"

    c += write_vec2_vector(ngon, f'{name}.ngon_base.points', 'points', 3)
    c += write_vec4_vector(ngon, f'{name}.ngon_base.fill_colors', 'fill_colors', 3)
    c += write_vec4_vector(ngon, f'{name}.ngon_base.border_colors', 'border_colors', 3)
    if 'border_width' in ngon:
        c += f"\t\t\t{name}.ngon_base.border_width = (float){ngon['border_width']};\n"
    if 'border_pivot' in ngon:
        if ngon['border_pivot'] is float:
            c += f"\t\t\t{name}.ngon_base.border_pivot = (float){ngon['border_pivot']};\n"
        elif ngon['border_pivot'] == 'inner':
            c += f"\t\t\t{name}.ngon_base.border_pivot = cmath::BorderPivot::INNER;\n"
        elif ngon['border_pivot'] == 'middle':
            c += f"\t\t\t{name}.ngon_base.border_pivot = cmath::BorderPivot::MIDDLE;\n"
        elif ngon['border_pivot'] == 'outer':
            c += f"\t\t\t{name}.ngon_base.border_pivot = cmath::BorderPivot::OUTER;\n"

    return c


def constructor(ngon) -> str:
    return f"""
        {{
            reg::params::NGon params;
{params_constructor(ngon, 'params')}
            {ngon['name']}.init(reg::load_ngon(std::move(params)));
        }}"""
