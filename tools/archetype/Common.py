def write_named_transform_2d(renderable: dict, name: str, tabs=2) -> str:
    c = ""
    if 'transform' not in renderable:
        return c
    transform = renderable['transform']
    if "position" in transform:
        c += "\t" * tabs + f"{name}.local.position = {{ (float){transform['position'][0]}, (float){transform['position'][1]} }};\n"
    if "rotation" in transform:
        c += "\t" * tabs + f"{name}.local.rotation = (float){transform['rotation']};\n"
    if "scale" in transform:
        c += "\t" * tabs + f"{name}.local.scale = {{ (float){transform['scale'][0]}, (float){transform['scale'][1]} }};\n"
    return c


def write_transform_2d(renderable: dict, tabs=2) -> str:
    return write_named_transform_2d(renderable, renderable['name'], tabs)


def write_vec2_vector(renderable: dict, variable: str, parameter: str, tabs=2) -> str:
    c = ""
    if parameter in renderable and len(renderable[parameter]) > 0:
        c += "\t" * tabs + f"{variable}.reserve({len(renderable[parameter])});\n"
        for vec2 in renderable[parameter]:
            c += "\t" * tabs + f"{variable}.push_back({{ (float){vec2[0]}, (float){vec2[1]} }});\n"
    return c


def write_vec4_vector(renderable: dict, variable: str, parameter: str, tabs=2) -> str:
    c = ""
    if parameter in renderable and len(renderable[parameter]) > 0:
        c += "\t" * tabs + f"{variable}.reserve({len(renderable[parameter])});\n"
        for vec2 in renderable[parameter]:
            c += "\t" * tabs + f"{variable}.push_back({{ (float){vec2[0]}, (float){vec2[1]}, (float){vec2[2]}, (float){vec2[3]} }});\n"
    return c


def write_vec4(renderable: dict, variable: str, parameter: str, tabs=2) -> str:
    c = ""
    if parameter in renderable:
        vec4 = renderable[parameter]
        c += "\t" * tabs + f"{variable} = {{ (float){vec4[0]}, (float){vec4[1]}, (float){vec4[2]}, (float){vec4[3]} }};\n"
    return c
