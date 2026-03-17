import os
from typing import Iterable

from prompt_toolkit.completion import Completion
from prompt_toolkit.document import Document

from editor.core import Resolver


# DOC document that paths are wrapped with [], and document macro usage $^
def get_path_completions(document: Document) -> Iterable[Completion]:
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
				if os.path.isdir(full_path):
					completion = f'{f}/'
				else:
					completion = f'{f}{Resolver.GROUP_CLOSE}'
				yield Completion(completion, start_position=-len(prefix))
	except FileNotFoundError:
		pass
