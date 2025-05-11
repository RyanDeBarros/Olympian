from .Common import *


def constructor(poly_composite) -> str:
    c = write_transformer_2d(poly_composite, 3)

    def write_ngon_method() -> str:
        c = "\t\t\t\treg::params::PolyComposite::NGonMethod method;\n"
        c += write_vec2_vector(poly_composite, 'method.points', 'points', 4)
        c += write_vec4_vector(poly_composite, 'method.colors', 'colors', 4)
        return c

    def write_bordered_ngon_method() -> str:
        c = "\t\t\t\treg::params::PolyComposite::BorderedNGonMethod method;\n"
        c += write_vec2_vector(poly_composite, 'method.ngon_base.points', 'points', 4)
        c += write_vec4_vector(poly_composite, 'method.ngon_base.fill_colors', 'fill colors', 4)
        c += write_vec4_vector(poly_composite, 'method.ngon_base.border_colors', 'border colors', 4)
        if 'border width' in poly_composite:
            c += f"\t\t\t\tmethod.ngon_base.border_width = (float){poly_composite['border width']};\n"
        if 'border pivot' in poly_composite:
            if poly_composite['border pivot'] is float:
                c += f"\t\t\t\tmethod.ngon_base.border_pivot = (float){poly_composite['border pivot']};\n"
            elif poly_composite['border pivot'] == 'inner':
                c += f"\t\t\t\tmethod.ngon_base.border_pivot = cmath::BorderPivot::INNER;\n"
            elif poly_composite['border pivot'] == 'middle':
                c += f"\t\t\t\tmethod.ngon_base.border_pivot = cmath::BorderPivot::MIDDLE;\n"
            elif poly_composite['border pivot'] == 'outer':
                c += f"\t\t\t\tmethod.ngon_base.border_pivot = cmath::BorderPivot::OUTER;\n"
        return c

    def write_convex_decomposition_method() -> str:
        c = "\t\t\t\treg::params::PolyComposite::ConvexDecompositionMethod method;\n"
        c += write_vec2_vector(poly_composite, 'method.points', 'points', 4)
        return c

    c += """\t\t\t{\n"""
    match poly_composite['method']:
        case "ngon":
            c += write_ngon_method()
        case "bordered ngon":
            c += write_bordered_ngon_method()
        case "convex decomposition":
            c += write_convex_decomposition_method()
    c += f"""\t\t\t\t{poly_composite['name']}.method = method;
\t\t\t}}\n"""
    return c
