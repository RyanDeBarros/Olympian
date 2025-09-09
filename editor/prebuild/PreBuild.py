import sys
import time
from pathlib import Path

sys.path.append(Path(__file__).parent.parent.parent.as_posix())

import ArchetypeGenerator

if __name__ == "__main__":
	start_time = time.time()

	tool_start = time.time()
	ArchetypeGenerator.generate_manifest()
	print(f"-- <PreBuild>: Generated archetypes ({time.time() - tool_start:.3f}s)")

	print(f"-- <PreBuild>: Finished pre-build ({time.time() - start_time:.3f}s)")
