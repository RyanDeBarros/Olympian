import time

import archetype.Generator
import textures.TextureImports


if __name__ == "__main__":
    start_time = time.time()

    tool_start = time.time()
    textures.TextureImports.import_manifest()
    print(f"-- <PreBuild>: Imported textures ({time.time() - tool_start:.3f}s)")

    tool_start = time.time()
    archetype.Generator.generate_manifest()
    print(f"-- <PreBuild>: Generated archetypes ({time.time() - tool_start:.3f}s)")

    print(f"-- <PreBuild>: Finished pre-build ({time.time() - start_time:.3f}s)")
