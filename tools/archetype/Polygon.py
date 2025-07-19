from .Common import *


def params_constructor(polygon, name) -> str:
    c = write_named_transformer_2d(polygon, name, 3)
    c += write_vec2_vector(polygon, f'{name}.points', 'points', 3)
    c += write_vec4_vector(polygon, f'{name}.colors', 'colors', 3)
    return c


def constructor(polygon) -> str:
    return f"""
        {{
            reg::params::Polygon params;
{params_constructor(polygon, 'params')}
            {polygon['name']}.init(reg::load_polygon(std::move(params)));
        }}"""
