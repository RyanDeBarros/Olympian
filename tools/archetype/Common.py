def write_transform_2d(renderable: dict, tabs=2) -> str:
    c = ""
    transform = renderable['transform']
    variable = renderable['name']
    if "position" in transform:
        c += "\t" * tabs + f"{variable}.local.position = {{ (float){transform['position'][0]}, (float){transform['position'][1]} }};\n"
    if "rotation" in transform:
        c += "\t" * tabs + f"{variable}.local.rotation = (float){transform['rotation']};\n"
    if "scale" in transform:
        c += "\t" * tabs + f"{variable}.local.scale = {{ (float){transform['scale'][0]}, (float){transform['scale'][1]} }};\n"
    return c
