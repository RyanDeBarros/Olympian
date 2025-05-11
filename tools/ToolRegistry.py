from Tool import ToolNode
import TextureImports
import archetype.Generator

ROOT = ToolNode("root")

ROOT.add_child(TextureImports.TOOL)
ROOT.add_child(archetype.Generator.TOOL)
