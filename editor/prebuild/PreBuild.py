import os
import sys
import time

sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..")))

import ArchetypeGenerator

if __name__ == "__main__":
	start_time = time.time()

	tool_start = time.time()
	ArchetypeGenerator.generate_manifest()
	print(f"-- <PreBuild>: Generated archetypes ({time.time() - tool_start:.3f}s)")

	print(f"-- <PreBuild>: Finished pre-build ({time.time() - start_time:.3f}s)")
