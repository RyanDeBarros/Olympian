from . import Sprite


def params_constructor(sprite_nonant, name) -> str:
	c = Sprite.params_constructor(sprite_nonant['sprite'], f'{name}.sprite_params')

	if 'nsize' in sprite_nonant:
		c += f"\t\t\t{name}.nsize = {{ (float){sprite_nonant['nsize'][0]}, (float){sprite_nonant['nsize'][1]} }};\n"

	if 'offsets' in sprite_nonant:
		offsets = sprite_nonant['offsets']
		if 'uniform' in offsets:
			c += f"\t\t\t{name}.offsets = oly::math::Padding::uniform((float){offsets['uniform']});\n"
		if 'left' in offsets:
			c += f"\t\t\t{name}.offsets.left = (float){offsets['left']};\n"
		if 'right' in offsets:
			c += f"\t\t\t{name}.offsets.right = (float){offsets['right']};\n"
		if 'bottom' in offsets:
			c += f"\t\t\t{name}.offsets.bottom = (float){offsets['bottom']};\n"
		if 'top' in offsets:
			c += f"\t\t\t{name}.offsets.top = (float){offsets['top']};\n"
	return c


def constructor(sprite_nonant) -> str:
	return f"""
		{{
			reg::params::SpriteNonant params;
{params_constructor(sprite_nonant, 'params')}
			{sprite_nonant['name']}.init(reg::load_sprite_nonant(params));
		}}"""
