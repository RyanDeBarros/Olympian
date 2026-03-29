from enum import Enum
from io import StringIO
from typing import override

from .. import AbstractBuffer, BufferPath, BufferChooser
from ..processing import AssetType, ExclamCommand, BoolField, RangedNumberField, EnumField, BufferSectionContext


# TODO v9 write Visual Studio Code plugin for text highlighting


class RealKeys(Enum):
	TEXTURE = "texture"
	ABSTRACT_STORAGE = "abstract_storage"
	IMAGE_STORAGE = "image_storage"
	STORAGE = "storage"
	GENERATE_MIPMAPS = "generate_mipmaps"
	MIN_FILTER = "min_filter"
	MAG_FILTER = "mag_filter"
	WRAP_S = "wrap_s"
	WRAP_T = "wrap_t"
	SVG_SCALE = "svg_scale"
	ANIM = "anim"
	ROWS = "rows"
	COLS = "cols"
	CELL_WIDTH_OVERRIDE = "cell_width_override"
	CELL_HEIGHT_OVERRIDE = "cell_height_override"
	DELAY_CS = "delay_cs"
	ROW_MAJOR = "row_major"
	ROW_UP = "row_up"


class VirtualKeys(Enum):
	ABSTRACT_STORAGE = "Abstract Storage"
	IMAGE_STORAGE = "Image Storage"
	STORAGE = "Storage"
	GENERATE_MIPMAPS = "Generate Mipmaps"
	MIN_FILTER = "Minimization Filter"
	MAG_FILTER = "Magnification Filter"
	WRAP_S = "Wrap (S)"
	WRAP_T = "Wrap (T)"
	SVG_SCALE = "Vector Scale"
	ANIM = "Animated"
	ROWS = "Rows"
	COLS = "Columns"
	CELL_WIDTH_OVERRIDE = "Cell Width Override"
	CELL_HEIGHT_OVERRIDE = "Cell Height Override"
	DELAY_CS = "Delay (cs)"
	ROW_MAJOR = "Row Major"
	ROW_UP = "Row Up"


class RealImageStorage(Enum):
	KEEP = "keep"
	DISCARD = "discard"


class VirtualImageStorage(Enum):
	KEEP = "Keep"
	DISCARD = "Discard"

	@staticmethod
	def default() -> "VirtualImageStorage":
		return VirtualImageStorage.DISCARD


class RealVectorGenerateMipmaps(Enum):
	OFF = "off"
	AUTO = "auto"
	MANUAL = "manual"


class VirtualVectorGenerateMipmaps(Enum):
	OFF = "Off"
	AUTO = "Auto"
	MANUAL = "Manual"

	@staticmethod
	def default() -> "VirtualVectorGenerateMipmaps":
		return VirtualVectorGenerateMipmaps.OFF


class RealMinFilter(Enum):
	NEAREST = 0x2600
	LINEAR = 0x2601
	NEAREST_MIPMAP_NEAREST = 0x2700
	LINEAR_MIPMAP_NEAREST = 0x2701
	NEAREST_MIPMAP_LINEAR = 0x2702
	LINEAR_MIPMAP_LINEAR = 0x2703


class VirtualMinFilter(Enum):
	NEAREST = "Nearest"
	LINEAR = "Linear"
	NEAREST_MIPMAP_NEAREST = "Nearest (nearest mipmap)"
	LINEAR_MIPMAP_NEAREST = "Linear (nearest mipmap)"
	NEAREST_MIPMAP_LINEAR = "Nearest (linear mipmap)"
	LINEAR_MIPMAP_LINEAR = "Linear (linear mipmap)"

	@staticmethod
	def default() -> "VirtualMinFilter":
		return VirtualMinFilter.NEAREST


class RealMagFilter(Enum):
	NEAREST = 0x2600
	LINEAR = 0x2601


class VirtualMagFilter(Enum):
	NEAREST = "Nearest"
	LINEAR = "Linear"

	@staticmethod
	def default() -> "VirtualMagFilter":
		return VirtualMagFilter.NEAREST


# TODO v8 Not super safe to use opengl constants (though they are very unlikely to change in a different version). Still, use custom value tables
class RealWrap(Enum):
	CLAMP_TO_EDGE = 0x812F
	CLAMP_TO_BORDER = 0x812D
	MIRRORED_REPEAT = 0x8370
	REPEAT = 0x2901
	MIRROR_CLAMP_TO_EDGE = 0x8743


class VirtualWrap(Enum):
	CLAMP_TO_EDGE = "Clamp To Edge"
	CLAMP_TO_BORDER = "Clamp To Border"
	MIRRORED_REPEAT = "Mirrored Repeat"
	REPEAT = "Repeat"
	MIRROR_CLAMP_TO_EDGE = "Mirror Clamp To Edge"

	@staticmethod
	def default() -> "VirtualWrap":
		return VirtualWrap.CLAMP_TO_EDGE


# TODO v7.1 actual descriptions
class Fields:
	ABSTRACT_STORAGE = EnumField(RealKeys.ABSTRACT_STORAGE, VirtualKeys.ABSTRACT_STORAGE, RealImageStorage, VirtualImageStorage, "")
	IMAGE_STORAGE = EnumField(RealKeys.IMAGE_STORAGE, VirtualKeys.IMAGE_STORAGE, RealImageStorage, VirtualImageStorage, "")
	STORAGE = EnumField(RealKeys.STORAGE, VirtualKeys.STORAGE, RealImageStorage, VirtualImageStorage, "")
	RASTER_GENERATE_MIPMAPS = BoolField(RealKeys.GENERATE_MIPMAPS, VirtualKeys.GENERATE_MIPMAPS, False, "")
	VECTOR_GENERATE_MIPMAPS = EnumField(RealKeys.GENERATE_MIPMAPS, VirtualKeys.GENERATE_MIPMAPS, RealVectorGenerateMipmaps, VirtualVectorGenerateMipmaps, "")
	SVG_SCALE = RangedNumberField(RealKeys.SVG_SCALE, VirtualKeys.SVG_SCALE, 0.0, 1048576.0, False, True, 1.0, "")
	MIN_FILTER = EnumField(RealKeys.MIN_FILTER, VirtualKeys.MIN_FILTER, RealMinFilter, VirtualMinFilter, "")
	MAG_FILTER = EnumField(RealKeys.MAG_FILTER, VirtualKeys.MAG_FILTER, RealMagFilter, VirtualMagFilter, "")
	WRAP_S = EnumField(RealKeys.WRAP_S, VirtualKeys.WRAP_S, RealWrap, VirtualWrap, "")
	WRAP_T = EnumField(RealKeys.WRAP_T, VirtualKeys.WRAP_T, RealWrap, VirtualWrap, "")


class TextureImportBuffer(AbstractBuffer):
	@override
	@classmethod
	def matches_asset(cls, buf: BufferPath) -> bool:
		return cls.simple_matches_asset(buf, AssetType.TEXTURE, [".png", ".jpg", ".jpeg", ".bmp", ".gif", ".svg"])  # TODO v7.2 use editor preferences

	def __init__(self, buf: BufferPath):
		super().__init__(buf)
		self.commands: list[ExclamCommand] = [
			ExclamCommand(cmd="new-slot", fn=self.fn_new_slot, info="Create new slot"),  # TODO v7.1 actual documentation about how to use the commands
			ExclamCommand(cmd="delete-slot", fn=self.fn_delete_slot, info="Delete slot"),
			ExclamCommand(cmd="default", fn=self.fn_default, info="Replace with default value")
		]

	@override
	def on_open(self) -> None:
		f = StringIO()
		# Header
		# TODO v7.1 extract to AbstractBuffer
		self.write(f, self.META_BLOCK_DELIMITER)
		self.write(f, f"Texture: @/{self.buf.resource_path_string()}")
		self.write(f, f"Format version: {self.format_version()}")
		self.write(f, f"\n!commands:")
		self.indent += 1
		for command in self.commands:
			self.write(f, f"{command.exclam}: {command.info}")
		self.indent -= 1
		self.write(f, self.META_BLOCK_DELIMITER)

		if self.is_svg():
			with self.write_subsection(f, "SVG"):
				self.write_enum(f, self.d, Fields.ABSTRACT_STORAGE)

		with self.write_subsection(f, "Slots"):
			slots = self.d[RealKeys.TEXTURE.value]
			for i in range(len(slots)):
				with self.write_subsection(f, f"Slot {i}"):
					self.indent += 1
					slot = slots[i]

					if self.is_svg():
						self.write_enum(f, slot, Fields.IMAGE_STORAGE)
						self.write_enum(f, slot, Fields.VECTOR_GENERATE_MIPMAPS)
						self.write_ranged_number(f, slot, Fields.SVG_SCALE)
					else:
						self.write_enum(f, slot, Fields.STORAGE)
						self.write_bool(f, slot, Fields.RASTER_GENERATE_MIPMAPS)

					self.write_enum(f, slot, Fields.MIN_FILTER)
					self.write_enum(f, slot, Fields.MAG_FILTER)
					self.write_enum(f, slot, Fields.WRAP_S)
					self.write_enum(f, slot, Fields.WRAP_T)

					# TODO v7.1 anim + spritesheet

					self.indent -= 1

		with self.buf.buffer_path.open('w') as w:
			self.internally_modified = True
			w.write(f.getvalue())

	@override
	def on_buffer_modified(self) -> None:
		pass  # TODO v7.1 validate and transfer formatted changes. Allow fuzzy matching, or entering !default (or other !commands), as well as # commands

	def is_svg(self) -> bool:
		return RealKeys.ABSTRACT_STORAGE.value in self.d or self.buf.asset_path.suffix == ".svg"

	def fn_new_slot(self, ctx: BufferSectionContext) -> None:
		pass  # TODO v7.1

	def fn_delete_slot(self, ctx: BufferSectionContext) -> None:
		pass  # TODO v7.1

	def fn_default(self, ctx: BufferSectionContext) -> None:
		pass  # TODO v7.1


def register():
	BufferChooser.instance().classes.append(TextureImportBuffer)
