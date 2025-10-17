from .Common import *


def escape_text(text: str) -> str:
	escaped = ""
	for c in text:
		if c == '\t':
			escaped += '\\t'
		elif c == '\n':
			escaped += '\\n'
		elif c == '\r':
			escaped += '\\r'
		else:
			escaped += c
	return escaped


def params_constructor(paragraph, name) -> str:
	c = write_named_transformer_2d(paragraph, name, 3)

	if 'draw_bkg' in paragraph:
		c += f"\t\t\t{name}.draw_bkg = {'true' if paragraph['draw_bkg'] else 'false'};\n"

	c += write_vec4(paragraph, f'{name}.bkg_color', 'bkg_color', 3)

	if 'format' in paragraph:
		# noinspection PyShadowingBuiltins
		format = paragraph['format']

		if 'pivot' in format:
			vec2 = format['pivot']
			c += f"\t\t\t{name}.format.pivot = {{ (float){vec2[0]}, (float){vec2[1]} }};\n"
		if 'line_spacing' in format:
			c += f"\t\t\t{name}.format.line_spacing = (float){format['line_spacing']};\n"
		if 'linebreak_spacing' in format:
			c += f"\t\t\t{name}.format.linebreak_spacing = (float){format['linebreak_spacing']};\n"
		if 'min_size' in format:
			vec2 = format['min_size']
			c += f"\t\t\t{name}.format.min_size = {{ (float){vec2[0]}, (float){vec2[1]} }};\n"
		if 'padding' in format:
			vec2 = format['padding']
			c += f"\t\t\t{name}.format.padding = {{ (float){vec2[0]}, (float){vec2[1]} }};\n"
		if 'text_wrap' in format:
			c += f"\t\t\t{name}.format.text_wrap = (float){format['text_wrap']};\n"
		if 'max_height' in format:
			c += f"\t\t\t{name}.format.max_height = (float){format['max_height']};\n"
		if 'tab_spaces' in format:
			c += f"\t\t\t{name}.format.tab_spaces = (float){format['tab_spaces']};\n"

		if 'horizontal_align' in format:
			match format['horizontal_align']:
				case 'left':
					c += f"\t\t\t{name}.format.horizontal_alignment = rendering::ParagraphFormat::HorizontalAlignment::LEFT;\n"
				case 'center':
					c += f"\t\t\t{name}.format.horizontal_alignment = rendering::ParagraphFormat::HorizontalAlignment::CENTER;\n"
				case 'right':
					c += f"\t\t\t{name}.format.horizontal_alignment = rendering::ParagraphFormat::HorizontalAlignment::RIGHT;\n"
				case 'justify':
					c += f"\t\t\t{name}.format.horizontal_alignment = rendering::ParagraphFormat::HorizontalAlignment::JUSTIFY;\n"
				case 'full_justify':
					c += f"\t\t\t{name}.format.horizontal_alignment = rendering::ParagraphFormat::HorizontalAlignment::FULL_JUSTIFY;\n"

		if 'vertical_align' in format:
			match format['vertical_align']:
				case 'top':
					c += f"\t\t\t{name}.format.vertical_alignment = rendering::ParagraphFormat::VerticalAlignment::TOP;\n"
				case 'middle':
					c += f"\t\t\t{name}.format.vertical_alignment = rendering::ParagraphFormat::VerticalAlignment::MIDDLE;\n"
				case 'bottom':
					c += f"\t\t\t{name}.format.vertical_alignment = rendering::ParagraphFormat::VerticalAlignment::BOTTOM;\n"
				case 'justify':
					c += f"\t\t\t{name}.format.vertical_alignment = rendering::ParagraphFormat::VerticalAlignment::JUSTIFY;\n"
				case 'full_justify':
					c += f"\t\t\t{name}.format.vertical_alignment = rendering::ParagraphFormat::VerticalAlignment::FULL_JUSTIFY;\n"

	def write_text_element(element):
		c = "\t\t\t{\n"
		c += "\t\t\t\treg::params::Paragraph::TextElement element;\n"
		if 'font_atlas' in element:
			c += f"\t\t\t\telement.font_atlas = \"{element['font_atlas']}\";\n"
		if 'atlas_index' in element:
			c += f"\t\t\t\telement.atlas_index = (unsigned int){element['atlas_index']};\n"
		if 'text' in element:
			c += f"\t\t\t\telement.text = \"{escape_text(element['text'])}\";\n"
		c += write_vec4(element, 'element.text_color', 'text_color', 4)
		if 'adj_offset' in element:
			c += f"\t\t\t\telement.adj_offset = (float){element['adj_offset']};\n"
		c += write_vec2(element, 'element.scale', 'scale', 4)
		if 'line_y_pivot' in element:
			c += f"\t\t\t\telement.line_y_pivot = (float){element['line_y_pivot']};\n"
		c += write_vec2(element, 'element.jitter_offset', 'jitter_offset', 4)
		if 'expand' in element:
			c += f"\t\t\t\telement.expand = {bool_str(element['expand'])};\n"
		c += "\t\t\t\tparams.elements.emplace_back(std::move(element));\n"
		c += "\t\t\t}\n"
		return c

	if 'element' in paragraph:
		element = paragraph['element']
		if isinstance(element, list):
			for e in element:
				c += write_text_element(e)
		else:
			c += write_text_element(element)

	c += write_vec4(paragraph, f'{name}.text_color', 'text_color', 3)

	return c


def constructor(paragraph) -> str:
	return f"""
		{{
			reg::params::Paragraph params;
{params_constructor(paragraph, 'params')}
			{paragraph['name']}.init(reg::load_paragraph(std::move(params)));
		}}"""
