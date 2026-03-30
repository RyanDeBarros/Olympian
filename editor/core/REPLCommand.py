from abc import ABC, abstractmethod
from pathlib import Path
from typing import Iterable

from prompt_toolkit.completion import CompleteEvent, Completion
from prompt_toolkit.document import Document

from editor.tools import eprint
from . import PathCompleter
from .context import EditorContext, PathUtils


class REPLCommand(ABC):
	def __init__(self, name: str):
		self.name = name

	def do_execute(self):
		if self.requires_initialized_editor():
			EditorContext.assert_initialized()
		self.execute()

	def requires_initialized_editor(self):
		return True

	@abstractmethod
	def execute(self):
		raise NotImplementedError()

	@abstractmethod
	def help(self):
		raise NotImplementedError()

	def expand_macros(self):
		return True

	def get_completions(self, document: Document, complete_event: CompleteEvent) -> Iterable[Completion]:
		yield from PathCompleter.get_path_completions(document)

	def print_arg_error(self, msg: str):
		eprint(f"{msg}. Run `help {self.name}` for more details")

	def resolve_asset_path(self, arg: str) -> Path | None:
		asset = Path(arg).resolve()
		if asset.exists():
			return asset
		else:
			self.print_arg_error(f"{PathUtils.printed_path(asset)} does not exist")
			return None
