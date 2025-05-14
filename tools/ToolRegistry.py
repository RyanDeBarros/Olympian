from ToolNode import ToolNode
import textures
import archetype
import fonts

ROOT = ToolNode("root")

ROOT.add_child(textures.TOOL)
ROOT.add_child(archetype.TOOL)
ROOT.add_child(fonts.TOOL)
