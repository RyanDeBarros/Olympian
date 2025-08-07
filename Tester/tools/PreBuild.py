import time

import archetype
import textures
import fonts


# TODO generate PreBuild in project folder. Remove tools/ and just use PreBuild.py
if __name__ == "__main__":
    start_time = time.time()

    tool_start = time.time()
    textures.Tool.import_textures_manifest()
    print(f"-- <PreBuild>: Imported textures ({time.time() - tool_start:.3f}s)")

    tool_start = time.time()
    fonts.Tool.import_fonts_manifest()
    print(f"-- <PreBuild>: Imported fonts ({time.time() - tool_start:.3f}s)")

    tool_start = time.time()
    archetype.Tool.generate_manifest()
    print(f"-- <PreBuild>: Generated archetypes ({time.time() - tool_start:.3f}s)")

    print(f"-- <PreBuild>: Finished pre-build ({time.time() - start_time:.3f}s)")
