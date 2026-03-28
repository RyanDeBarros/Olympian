from enum import Enum
from pathlib import Path

from .. import AbstractBuffer, BufferPath, BufferChooser
from ..processing import AssetType


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


# TODO v7.2 add format version to meta fields
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


class TextureImportBuffer(AbstractBuffer):
	@classmethod
	def matches_asset(cls, buf: BufferPath) -> bool:
		return cls.simple_matches_asset(buf, AssetType.TEXTURE, [".png", ".jpg", ".jpeg", ".bmp", ".gif", ".svg"])  # TODO v7.2 use editor preferences

	def __init__(self, buf: BufferPath):
		super().__init__(buf)

	def on_open(self) -> None:
		with self.buf.buffer_path.open('w') as f:
			# Header
			self.write(f, f"-- Texture @/{self.buf.resource_path_string()}\n")

			if self.is_svg():
				self.write(f, "# SVG")
				self.write_enum(f, self.d, RealKeys.ABSTRACT_STORAGE, VirtualKeys.ABSTRACT_STORAGE, RealImageStorage, VirtualImageStorage)
				self.write(f)

			self.write(f, "# Slots\n")

			slots = self.d[RealKeys.TEXTURE.value]
			for i in range(len(slots)):
				self.write(f, f"## Slot {i}")
				self.indent += 1
				slot = slots[i]

				if self.is_svg():
					self.write_enum(f, slot, RealKeys.IMAGE_STORAGE, VirtualKeys.IMAGE_STORAGE, RealImageStorage, VirtualImageStorage)
					self.write_enum(f, slot, RealKeys.GENERATE_MIPMAPS, VirtualKeys.GENERATE_MIPMAPS, RealVectorGenerateMipmaps, VirtualVectorGenerateMipmaps)
					self.write_float(f, slot, RealKeys.SVG_SCALE, VirtualKeys.SVG_SCALE, 1.0, "(0.0 - 1048576.0)")
				else:
					self.write_enum(f, slot, RealKeys.STORAGE, VirtualKeys.STORAGE, RealImageStorage, VirtualImageStorage)
					self.write_bool(f, slot, RealKeys.GENERATE_MIPMAPS, VirtualKeys.GENERATE_MIPMAPS, False)

				self.write_enum(f, slot, RealKeys.MIN_FILTER, VirtualKeys.MIN_FILTER, RealMinFilter, VirtualMinFilter)
				self.write_enum(f, slot, RealKeys.MAG_FILTER, VirtualKeys.MAG_FILTER, RealMagFilter, VirtualMagFilter)
				self.write_enum(f, slot, RealKeys.WRAP_S, VirtualKeys.WRAP_S, RealWrap, VirtualWrap)
				self.write_enum(f, slot, RealKeys.WRAP_T, VirtualKeys.WRAP_T, RealWrap, VirtualWrap)

				# TODO v7.1 anim + spritesheet

				self.indent -= 1
				self.write(f)

			self.write(f)

	def on_modified(self, path: Path, was_dir: bool) -> None:
		pass  # TODO v7.1 validate and transfer formatted changes

	def is_svg(self) -> bool:
		return RealKeys.ABSTRACT_STORAGE.value in self.d or self.buf.asset_path.suffix == ".svg"


def register():
	BufferChooser.instance().classes.append(TextureImportBuffer)
