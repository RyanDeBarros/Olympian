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


def constructor(paragraph) -> str:
    c = write_transform_2d(paragraph)
    c += f"\t\t{paragraph['name']}.font_atlas = \"{paragraph['font atlas']}\";\n"

    if 'text' in paragraph:
        c += f"\t\t{paragraph['name']}.text = \"{escape_text(paragraph['text'])}\";\n"
    if 'draw bkg' in paragraph:
        c += f"\t\t{paragraph['name']}.draw_bkg = {'true' if paragraph['draw bkg'] else 'false'};\n"

    c += write_vec4(paragraph, f'{paragraph['name']}.bkg_color', 'bkg color')
    c += write_vec4(paragraph, f'{paragraph['name']}.text_color', 'text color')

    if 'glyph colors' in paragraph:
        c += f"\t\t{paragraph['name']}.glyph_colors.reserve({len(paragraph['glyph colors'])});\n"
        for index, color in paragraph['glyph colors'].items():
            c += f"\t\t{paragraph['name']}.glyph_colors.push_back({{ {int(index)}, {{ (float){color[0]}, (float){color[1]}, (float){color[2]}, (float){color[3]} }} }});\n"

    if 'format' in paragraph:
        format = paragraph['format']

        if 'pivot' in format:
            vec2 = format['pivot']
            c += f"\t\t{paragraph['name']}.format.pivot = {{ (float){vec2[0]}, (float){vec2[1]} }};\n"
        if 'line spacing' in format:
            c += f"\t\t{paragraph['name']}.format.line_spacing = (float){format['line spacing']};\n"
        if 'linebreak spacing' in format:
            c += f"\t\t{paragraph['name']}.format.linebreak_spacing = (float){format['linebreak spacing']};\n"
        if 'min size' in format:
            vec2 = format['min size']
            c += f"\t\t{paragraph['name']}.format.min_size = {{ (float){vec2[0]}, (float){vec2[1]} }};\n"
        if 'padding' in format:
            vec2 = format['padding']
            c += f"\t\t{paragraph['name']}.format.padding = {{ (float){vec2[0]}, (float){vec2[1]} }};\n"
        if 'text wrap' in format:
            c += f"\t\t{paragraph['name']}.format.text_wrap = (float){format['text wrap']};\n"
        if 'max height' in format:
            c += f"\t\t{paragraph['name']}.format.max_height = (float){format['max height']};\n"
        if 'tab spaces' in format:
            c += f"\t\t{paragraph['name']}.format.tab_spaces = (float){format['tab spaces']};\n"

        if 'horizontal align' in format:
            match format['horizontal align']:
                case 'left':
                    c += f"\t\t{paragraph['name']}.format.horizontal_alignment = rendering::ParagraphFormat::HorizontalAlignment::LEFT;\n"
                case 'center':
                    c += f"\t\t{paragraph['name']}.format.horizontal_alignment = rendering::ParagraphFormat::HorizontalAlignment::CENTER;\n"
                case 'right':
                    c += f"\t\t{paragraph['name']}.format.horizontal_alignment = rendering::ParagraphFormat::HorizontalAlignment::RIGHT;\n"
                case 'justify':
                    c += f"\t\t{paragraph['name']}.format.horizontal_alignment = rendering::ParagraphFormat::HorizontalAlignment::JUSTIFY;\n"
                case 'full justify':
                    c += f"\t\t{paragraph['name']}.format.horizontal_alignment = rendering::ParagraphFormat::HorizontalAlignment::FULL_JUSTIFY;\n"

        if 'vertical align' in format:
            match format['vertical align']:
                case 'top':
                    c += f"\t\t{paragraph['name']}.format.vertical_alignment = rendering::ParagraphFormat::VerticalAlignment::TOP;\n"
                case 'middle':
                    c += f"\t\t{paragraph['name']}.format.vertical_alignment = rendering::ParagraphFormat::VerticalAlignment::MIDDLE;\n"
                case 'bottom':
                    c += f"\t\t{paragraph['name']}.format.vertical_alignment = rendering::ParagraphFormat::VerticalAlignment::BOTTOM;\n"
                case 'justify':
                    c += f"\t\t{paragraph['name']}.format.vertical_alignment = rendering::ParagraphFormat::VerticalAlignment::JUSTIFY;\n"
                case 'full justify':
                    c += f"\t\t{paragraph['name']}.format.vertical_alignment = rendering::ParagraphFormat::VerticalAlignment::FULL_JUSTIFY;\n"

    return c
