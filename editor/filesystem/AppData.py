from pathlib import Path

import platformdirs

APP_NAME = "Olympian Editor"
APP_AUTHOR = "Ryan de Barros"
APP_VERSION = "1.0"

PERSISTENT_PATH = Path(platformdirs.user_data_dir(appname=APP_NAME, appauthor=APP_AUTHOR, version=APP_VERSION, ensure_exists=True)).resolve()
SETTINGS_PATH = Path(platformdirs.user_config_dir(appname=APP_NAME, appauthor=APP_AUTHOR, version=APP_VERSION, roaming=True, ensure_exists=True)).resolve()
EDITOR_DATA_PATH = Path('data').resolve()
EDITOR_DATA_PATH.mkdir(exist_ok=True)
