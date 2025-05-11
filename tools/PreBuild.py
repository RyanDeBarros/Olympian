import time

import archetype.Generator


if __name__ == "__main__":
    start_time = time.time()
    archetype.Generator.generate_manifest()
    elapsed = time.time() - start_time
    print(f"-- <PreBuild>: Finished in {elapsed:.3f} seconds")
