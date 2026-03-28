from .. import AbstractBuffer, BufferPath, BufferChooser
from ..processing import AssetType


class TextureImportBuffer(AbstractBuffer):
	@classmethod
	def matches_asset(cls, buf: BufferPath) -> bool:
		return cls.simple_matches_asset(buf, AssetType.TEXTURE, [".png", ".jpg", ".jpeg", ".bmp", ".gif", ".svg"])  # TODO v7.2 use editor preferences

	def __init__(self, buf: BufferPath):
		super().__init__(buf)


def register():
	BufferChooser.instance().classes.append(TextureImportBuffer)
