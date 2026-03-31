import hashlib
from weakref import WeakSet
from pathlib import Path
from typing import Optional, Callable

from watchdog.events import FileSystemEventHandler, FileCreatedEvent, DirCreatedEvent, DirMovedEvent, FileMovedEvent, DirModifiedEvent, FileModifiedEvent, \
	FileDeletedEvent, DirDeletedEvent
from watchdog.observers import Observer


class FileSystemWatcherBase:
	def on_moved(self, src: Path, dest: Path, was_dir: bool) -> None:
		pass

	def on_created(self, path: Path, was_dir: bool) -> None:
		pass

	def on_deleted(self, path: Path, was_dir: bool) -> None:
		pass

	def on_modified(self, path: Path, was_dir: bool) -> None:
		pass


class CentralHandler(FileSystemEventHandler):
	def __init__(self):
		self.watchers: WeakSet[FileSystemWatcherBase] = WeakSet()

	def on_moved(self, event: DirMovedEvent | FileMovedEvent) -> None:
		if not event.is_synthetic:
			for watcher in list(self.watchers):
				watcher.on_moved(Path(event.src_path), Path(event.dest_path), event.is_directory)

	def on_created(self, event: DirCreatedEvent | FileCreatedEvent) -> None:
		if not event.is_synthetic:
			for watcher in list(self.watchers):
				watcher.on_created(Path(event.src_path), event.is_directory)

	def on_deleted(self, event: DirDeletedEvent | FileDeletedEvent) -> None:
		if not event.is_synthetic:
			for watcher in list(self.watchers):
				watcher.on_deleted(Path(event.src_path), event.is_directory)

	def on_modified(self, event: DirModifiedEvent | FileModifiedEvent) -> None:
		if not event.is_synthetic:
			for watcher in list(self.watchers):
				watcher.on_modified(Path(event.src_path), event.is_directory)


class Watchdog:
	_instance: Optional["Watchdog"] = None

	def __init__(self):
		from editor.core import ProgramState

		self.started = False
		self.handler = CentralHandler()
		self.observer = Observer()
		self.observer.schedule(self.handler, ProgramState.instance().project_dir, recursive=True)

	@classmethod
	def instance(cls):
		if cls._instance is None:
			cls._instance = cls()
		return cls._instance

	def start(self):
		if not self.started:
			self.observer.start()
			self.started = True

	def stop(self):
		if self.started:
			self.observer.stop()
			self.observer.join()
			self.started = False

	def update_project_root(self):
		from editor.core import ProgramState

		was_running = self.started
		if was_running:
			self.stop()

		self.observer = Observer()
		self.observer.schedule(self.handler, ProgramState.instance().project_dir, recursive=True)

		if was_running:
			self.start()


class FileSystemWatcher(FileSystemWatcherBase):
	def __init__(self):
		Watchdog.instance().handler.watchers.add(self)


class EditableFileWatcher(FileSystemWatcher):
	def __init__(self, filepath: Path):
		super().__init__()
		self.filepath = filepath
		self.internally_modified = False
		self.hash = ""
		self.created: Callable[[], None] | None = None
		self.deleted: Callable[[], None] | None = None
		self.modified: Callable[[], None] | None = None
		self.moved: Callable[[Path], None] | None = None

	def on_created(self, path: Path, was_dir: bool) -> None:
		if self.created is not None and path == self.filepath:
			#  TODO v7 use internally_modified/hash
			self.created()

	def on_deleted(self, path: Path, was_dir: bool) -> None:
		if self.deleted is not None and path == self.filepath:
			#  TODO v7 use internally_modified/hash
			self.deleted()

	def on_modified(self, path: Path, was_dir: bool) -> None:
		if self.modified is not None and path == self.filepath:
			h = hashlib.md5(path.read_bytes()).hexdigest()
			if self.internally_modified:
				self.hash = h
				self.internally_modified = False
			elif self.hash != h:
				self.hash = h
				self.modified()

	def on_moved(self, src: Path, dest: Path, was_dir: bool) -> None:
		if self.moved is not None and src == self.filepath:
			self.filepath = dest
			self.moved(src)

	def write(self, text: str) -> None:
		with self.filepath.open('w') as w:  # TODO v7 permission denied occasionally while buffer is opened
			self.internally_modified = True
			w.write(text)
