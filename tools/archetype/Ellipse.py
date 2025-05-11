from .Common import *


def constructor(ellipse) -> str:
    c = write_transformer_2d(ellipse, 3)

    def write_dimension_parameter(parameter: str, variable: str) -> str:
        if parameter in ellipse:
            return f"\t\t\t{ellipse['name']}.dimension.{variable} = (float){ellipse[parameter]};\n"
        else:
            return ""

    def write_color_parameter(parameter: str, variable: str) -> str:
        if parameter in ellipse:
            return f"\t\t\t{ellipse['name']}.color.{variable} = {{ (float){ellipse[parameter][0]}, (float){ellipse[parameter][1]}, (float){ellipse[parameter][2]}, (float){ellipse[parameter][3]} }};\n"
        else:
            return ""

    c += write_dimension_parameter('width', 'width')
    c += write_dimension_parameter('height', 'height')
    c += write_dimension_parameter('border', 'border')
    c += write_dimension_parameter('border exp', 'border_exp')
    c += write_dimension_parameter('fill exp', 'fill_exp')
    c += write_color_parameter('fill inner color', 'fill_inner')
    c += write_color_parameter('fill outer color', 'fill_outer')
    c += write_color_parameter('border inner color', 'border_inner')
    c += write_color_parameter('border outer color', 'border_outer')

    return c
