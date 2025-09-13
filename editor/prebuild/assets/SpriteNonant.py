from . import Sprite


def params_constructor(sprite_nonant, name) -> str:
	c = Sprite.params_constructor(sprite_nonant['sprite'], f'{name}.sprite_params')

	if 'nsize' in sprite_nonant:
		c += f"\t\t\t{name}.nsize = {{ (float){sprite_nonant['nsize'][0]}, (float){sprite_nonant['nsize'][1]} }};\n"

	if 'left_offset' in sprite_nonant:
		c += f"\t\t\t{name}.offsets.x_left = (float){sprite_nonant['left_offset']};\n"
	if 'right_offset' in sprite_nonant:
		c += f"\t\t\t{name}.offsets.x_right = (float){sprite_nonant['right_offset']};\n"
	if 'bottom_offset' in sprite_nonant:
		c += f"\t\t\t{name}.offsets.y_bottom = (float){sprite_nonant['bottom_offset']};\n"
	if 'top_offset' in sprite_nonant:
		c += f"\t\t\t{name}.offsets.y_top = (float){sprite_nonant['top_offset']};\n"
	return c


def constructor(sprite_nonant) -> str:
	return f"""
		{{
			reg::params::SpriteNonant params;
{params_constructor(sprite_nonant, 'params')}
			{sprite_nonant['name']}.init(reg::load_sprite_nonant(params));
		}}"""
