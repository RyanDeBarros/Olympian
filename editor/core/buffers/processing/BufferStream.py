from contextlib import contextmanager
from io import StringIO
from typing import Iterator


class BufferStream:
	def __init__(self):
		self.fio = StringIO()
		self.indent = -1
		self.subsection_newline = True

	def reset(self):
		self.fio.seek(0)
		self.fio.truncate()
		self.indent = -1
		self.subsection_newline = True

	def string(self) -> str:
		return self.fio.getvalue()

	def write(self, line: str = ""):
		self.subsection_newline = True
		if len(line) > 0:
			self.fio.write(f"{self.indent * '\t'}{line}\n")
		else:
			self.fio.write('\n')

	def raw_write(self, text: str):
		self.fio.write(text)

	@contextmanager
	def subindent(self) -> Iterator[None]:
		self.indent += 1
		yield None
		self.indent -= 1

	def write_subsection(self, title: str) -> None:
		if self.subsection_newline:
			self.write()
		self.write()
		self.write(title)
		self.subsection_newline = False
