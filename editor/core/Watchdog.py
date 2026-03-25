from weakref import WeakSet
from pathlib import Path
from typing import Optional

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
