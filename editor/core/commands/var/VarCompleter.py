from typing import Iterable

from prompt_toolkit.completion import Completion
from prompt_toolkit.document import Document

from editor.core import KeyCompleter
from . import Storage


def get_temp_completions(document: Document) -> Iterable[Completion]:
	yield from KeyCompleter.get_keys_completions(document, Storage.temp_keys())


def get_persistent_completions(document: Document) -> Iterable[Completion]:
	yield from KeyCompleter.get_keys_completions(document, Storage.persistent_keys())
