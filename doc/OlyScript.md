# General format

A .oly file may be composed of several asset segments. Each asset consists of a header, content, and footer. The footer simply closes the asset segment, while the content's syntax may be dependent on the asset type as defined by the header. Each line within the content section is referred to as a *statement*.

### Header

The header has the format:

```^HDR[:lid][:tmp]```

* HDR is a character code corresponding to the asset type. A comprehensive list is found in the "Asset Types" subsection below.
* lid is an optional local identifier used to identify assets within the asset file, which isn't necessary for files containing single assets. It consists strictly of letters, numbers, hyphens, underscores, and spaces. In general, an asset can be referenced by other assets or in code by `#FILE[:lid]#`.
* If tmp is T, then the file is temporary, meaning it won't be kept in an asset registry.

### Footer

The footer has the format:

```$HDR```

Where the HDR character code must match the header character code.

### Common syntax

Comments are denoted with a backtick (\`), or alternatively a triple backtick (\`\`\`). Inline comments can be used via a pair of backticks (like \`comment\` or \`\`\`comment\`\`\`), otherwise the entire line is commented from the first unclosed backtick.

Leading and trailing whitespace in lines are trimmed, so empty lines and tabs will be ignored. To join multiple lines, end a line with a single backslash (\\). This will not work for multi-line comments (start each line with a backtick (\`) instead).

Strings must always be delimited by double quotes ("), not single quotes (') or backticks (\`). Within a string, normal escaping rules apply, using the backslash (\\) as an escape character.

## Asset Types

The following is a list of header codes for each asset type:

| HDR | Asset Type |
| --- | --- |
| oly | olympian configuration |
| is | input signal |
| ism | input signal mapping |
| tex | texture import |
| spr | sprite |
| tst | tileset |
| tm | tilemap |

# Olympian Engine Configuration

Header code is "oly". Each statement begins with a (!). Note that "Olympian.oly" in the resource root is the only file that accepts engine configuration assets, and there can only be one at the beginning of the file.

| Statement | Description |
| --- | --- |
| `!w:width:height:"title"` | Width, height, and title of window. |
| `!wh:name=setting|...` | List of window hint name/setting pairs, separated by pipes. |
| `!g:gamepads` | Supported number of gamepads. |

## Window hints

| Name | Setting |
| --- | --- |
| | |
