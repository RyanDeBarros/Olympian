from .Common import *


def params_constructor(poly_composite, name) -> str:
    c = write_named_transformer_2d(poly_composite, name, 3)

    def write_ngon_method() -> str:
        c = "\t\t\t\treg::params::PolyComposite::NGonMethod _method;\n"
        c += write_vec2_vector(poly_composite, '_method.points', 'points', 4)
        c += write_vec4_vector(poly_composite, '_method.colors', 'colors', 4)
        return c

    def write_bordered_ngon_method() -> str:
        c = "\t\t\t\treg::params::PolyComposite::BorderedNGonMethod _method;\n"
        c += write_vec2_vector(poly_composite, '_method.ngon_base.points', 'points', 4)
        c += write_vec4_vector(poly_composite, '_method.ngon_base.fill_colors', 'fill_colors', 4)
        c += write_vec4_vector(poly_composite, '_method.ngon_base.border_colors', 'border_colors', 4)
        if 'border_width' in poly_composite:
            c += f"\t\t\t\t_method.ngon_base.border_width = (float){poly_composite['border_width']};\n"
        if 'border_pivot' in poly_composite:
            if poly_composite['border_pivot'] is float:
                c += f"\t\t\t\t_method.ngon_base.border_pivot = (float){poly_composite['border_pivot']};\n"
            elif poly_composite['border_pivot'] == 'inner':
                c += f"\t\t\t\t_method.ngon_base.border_pivot = cmath::BorderPivot::INNER;\n"
            elif poly_composite['border_pivot'] == 'middle':
                c += f"\t\t\t\t_method.ngon_base.border_pivot = cmath::BorderPivot::MIDDLE;\n"
            elif poly_composite['border_pivot'] == 'outer':
                c += f"\t\t\t\t_method.ngon_base.border_pivot = cmath::BorderPivot::OUTER;\n"
        return c

    def write_convex_decomposition_method() -> str:
        c = "\t\t\t\treg::params::PolyComposite::ConvexDecompositionMethod _method;\n"
        c += write_vec2_vector(poly_composite, '_method.points', 'points', 4)
        return c

    c += """\t\t\t{\n"""
    match poly_composite['method']:
        case "ngon":
            c += write_ngon_method()
        case "bordered_ngon":
            c += write_bordered_ngon_method()
        case "convex_decomposition":
            c += write_convex_decomposition_method()
    c += f"""\t\t\t\t{name}.method = _method;
\t\t\t}}\n"""
    return c


def constructor(poly_composite) -> str:
    return f"""
        {{
            reg::params::PolyComposite params;
{params_constructor(poly_composite, 'params')}
            {poly_composite['name']}.init(reg::load_poly_composite(std::move(params)));
        }}"""
