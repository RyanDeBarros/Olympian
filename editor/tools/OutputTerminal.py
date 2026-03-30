import sys
import time
from pathlib import Path

log_file = Path(sys.argv[1])
last_pos = 0

while True:
	if log_file.exists():
		with log_file.open("r", encoding="utf-8") as f:
			f.seek(last_pos)
			new_text = f.read()
			if new_text:
				print(new_text, end="", flush=True)  # flush forces immediate output
				last_pos += len(new_text)
	time.sleep(0.05)
