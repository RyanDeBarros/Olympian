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
    c += f"\t\t\t{name}.font_atlas = \"{paragraph['font_atlas']}\";\n"

    if 'atlas_index' in paragraph:
        c += f"\t\t\t{name}.atlas_index = (unsigned int){paragraph['atlas_index']};\n"
    if 'text' in paragraph:
        c += f"\t\t\t{name}.text = \"{escape_text(paragraph['text'])}\";\n"
    if 'draw_bkg' in paragraph:
        c += f"\t\t\t{name}.draw_bkg = {'true' if paragraph['draw_bkg'] else 'false'};\n"

    c += write_vec4(paragraph, f'{name}.bkg_color', 'bkg_color', 3)
    c += write_vec4(paragraph, f'{name}.text_color', 'text_color', 3)

    if 'glyph_colors' in paragraph:
        c += f"\t\t\t{name}.glyph_colors.reserve({len(paragraph['glyph_colors'])});\n"
        for index, color in paragraph['glyph_colors'].items():
            c += f"\t\t\t{name}.glyph_colors.push_back({{ {int(index)}, {{ (float){color[0]}, (float){color[1]}, (float){color[2]}, (float){color[3]} }} }});\n"

    if 'format' in paragraph:
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

    return c


def constructor(paragraph) -> str:
    return f"""
        {{
            reg::params::Paragraph params;
{params_constructor(paragraph, 'params')}
            {paragraph['name']}.init(reg::load_paragraph(std::move(params)));
        }}"""
