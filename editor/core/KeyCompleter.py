from typing import Iterable

from prompt_toolkit.completion import Completion
from prompt_toolkit.document import Document


def get_keys_completions(document: Document, keys: Iterable[str]) -> Iterable[Completion]:
	cword = document.get_word_before_cursor(WORD=True)

	typed_parts = cword.split(".")
	current_part = typed_parts[-1]
	complete_parts = typed_parts[:-1]

	seen: set[str] = set()

	for cmd in keys:
		parts = cmd.split(".")

		if len(typed_parts) > len(parts):
			continue

		if parts[:len(complete_parts)] != complete_parts:
			continue

		next_part = parts[len(complete_parts)]
		if not next_part.startswith(current_part):
			continue

		suggestion = next_part
		if len(parts) > len(complete_parts) + 1:
			suggestion += "."

		if suggestion not in seen:
			seen.add(suggestion)
			yield Completion(suggestion, start_position=-len(current_part))
