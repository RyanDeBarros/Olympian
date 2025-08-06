import toml


class ManifestTOML:
    def __init__(self):
        self.filepath = 'data/manifest.toml'
        with open(self.filepath, 'r') as f:
            self.toml = toml.load(f)

    def dump(self):
        with open(self.filepath, 'w') as f:
            toml.dump(self.toml, f)

    def filepath_is_contained_in_existing_project(self, filepath):
        pass  # TODO

    def create_project(self, project_filepath, src_folder, res_folder, gen_folder):
        pass  # TODO

    def is_valid_project_file(self, project_filepath):
        # TODO
        return True

    def get_recent_project_filepaths(self):
        # TODO
        return []
