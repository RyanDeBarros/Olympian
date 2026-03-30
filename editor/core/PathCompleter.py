import os
from pathlib import Path
from typing import Iterable

from prompt_toolkit.completion import Completion
from prompt_toolkit.document import Document

from . import Resolver
from ..tools import TOMLAdapter


# DOC document that paths are wrapped with [], and document macro usage $^
def get_path_completions(document: Document, *, directories_only: bool = False) -> Iterable[Completion]:
	text_before_cursor = document.text_before_cursor

	opening_index = text_before_cursor.rfind(Resolver.GROUP_OPEN)
	if opening_index == -1:
		return

	cword = text_before_cursor[opening_index + 1:]

	dir_part = os.path.dirname(cword) if os.path.dirname(cword) else '.'
	prefix = os.path.basename(cword)

	try:
		for special in ['.', '..']:
			if special.startswith(prefix):
				yield Completion(f'{special}/', start_position=-len(prefix))

		for f in os.listdir(dir_part):
			if f.startswith(prefix):
				full_path = os.path.join(dir_part, f)
				p = Path(full_path)

				if directories_only and not p.is_dir():
					continue

				from .buffers.processing import Metadata
				if p.suffix == Metadata.IMPORT_EXTENSION and Metadata.is_import(TOMLAdapter.meta(p)):
					continue

				if os.path.isdir(full_path):
					completion = f'{f}/'
				else:
					completion = f'{f}{Resolver.GROUP_CLOSE}'
				yield Completion(completion, start_position=-len(prefix))
	except FileNotFoundError:
		pass
