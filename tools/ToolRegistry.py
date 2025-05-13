from Tool import ToolNode
import textures.TextureImports
import archetype.Generator

ROOT = ToolNode("root")

ROOT.add_child(textures.TextureImports.TOOL)
ROOT.add_child(archetype.Generator.TOOL)
