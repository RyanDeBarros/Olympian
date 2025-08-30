import time

import archetype


# TODO v4 PreBuild should be in a prebuild top-level folder
if __name__ == "__main__":
    start_time = time.time()

    # tool_start = time.time()
    # archetype.Tool.generate_manifest()
    # print(f"-- <PreBuild>: Generated archetypes ({time.time() - tool_start:.3f}s)")

    print(f"-- <PreBuild>: Finished pre-build ({time.time() - start_time:.3f}s)")
