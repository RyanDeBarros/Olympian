from io import StringIO


class ExclamEdit:
	def __init__(self, start_idx: int, end_idx: int, new_text: str):
		assert start_idx <= end_idx
		self.start_idx = start_idx
		self.end_idx = end_idx
		self.new_text = new_text

	def _transform_index(self, idx: int) -> int:
		if idx <= self.start_idx:
			return idx

		delta = len(self.new_text) - (self.end_idx - self.start_idx)
		if idx >= self.end_idx:
			return idx + delta

		return min(idx, self.end_idx + delta)

	def invoke(self, fio: StringIO, queue: list["ExclamEdit"]) -> None:
		text = fio.getvalue()
		fio.seek(0)
		fio.truncate()
		fio.write(text[:self.start_idx] + self.new_text + text[self.end_idx + 1:])
		for edit in queue:
			edit.start_idx = self._transform_index(edit.start_idx)
			edit.end_idx = self._transform_index(edit.end_idx)
