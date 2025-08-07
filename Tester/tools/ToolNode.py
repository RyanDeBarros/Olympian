import os.path

import toml
from colorama import Fore, Style
import re


RES_PATH = "../res/"


def res_path(path: str) -> str:
    return os.path.join(RES_PATH, path)


class ToolNode:
    def __init__(self, name, description="", action=None):
        self.name = name
        self.description = description
        self.action = action
        self.children = {}
        self.parent = None

    def add_child(self, subnode):
        self.children[subnode.name] = subnode
        subnode.parent = self

    def get_path(self):
        path = []
        node = self
        while node:
            path.append(node.name)
            node = node.parent
        return "/".join(reversed(path)).replace("root", "")


def print_info(text):
    print(Fore.CYAN + text + Style.RESET_ALL)


def print_warning(text):
    print(Fore.YELLOW + text + Style.RESET_ALL)


def print_error(text):
    print(Fore.RED + text + Style.RESET_ALL)


GLOBAL_VARS = {}


def expand_global_vars(inpt):
    # global variable has the form ${name}
    return re.sub(r"\$\{(\w+)}", lambda m: GLOBAL_VARS.get(m.group(1), ""), inpt)


def varinput(message):
    return expand_global_vars(input(message))


def var_cmd(cmd: str) -> bool:
    if cmd.startswith("!SET "):
        parts = cmd.split(" ", 2)
        if len(parts) == 3:
            GLOBAL_VARS[parts[1]] = parts[2]
            print_info(f"Set {parts[1]} = {parts[2]}")
        else:
            print_error("Incorrect format. Use '!SET <key> <value>'")
        return True

    elif cmd.startswith("!GET "):
        parts = cmd.split(" ")
        if len(parts) == 2:
            if parts[1] in GLOBAL_VARS:
                print_info(f"Get {parts[1]} = {GLOBAL_VARS[parts[1]]}")
            else:
                print_info(f"No key named {parts[1]}")
        else:
            print_error("Incorrect format. Use '!GET <key>'")
        return True

    elif cmd.startswith("!UNSET "):
        parts = cmd.split(" ")
        if len(parts) == 2:
            if parts[1] in GLOBAL_VARS:
                print_info(f"Unset {parts[1]} = {GLOBAL_VARS[parts[1]]}")
                del GLOBAL_VARS[parts[1]]
            else:
                print_info(f"No key named {parts[1]}")
        else:
            print_error("Incorrect format. Use '!UNSET <key>'")
        return True

    elif cmd.startswith("!CLEAR"):
        GLOBAL_VARS.clear()
        print_info("Global variables cleared")
        return True

    elif cmd.startswith("!VARS"):
        if len(GLOBAL_VARS) == 0:
            print_info("No global variables present")
        else:
            for k, v in GLOBAL_VARS.items():
                print_info(f"{k} = {v}")
        return True

    elif cmd.startswith("!OVERWRITE"):
        with open("vars.toml", 'w') as f:
            toml.dump(GLOBAL_VARS, f)
        print_info("Saved global variables overwritten")
        return True

    elif cmd.startswith("!APPEND"):
        saved = {}
        if os.path.exists("vars.toml"):
            with open("vars.toml", 'r') as f:
                saved = toml.load(f)
        for k, v in GLOBAL_VARS.items():
            saved[k] = v
        with open("vars.toml", 'w') as f:
            toml.dump(saved, f)
        print_info("Saved global variables appended to")
        return True

    elif cmd.startswith("!LOAD"):
        if os.path.exists("vars.toml"):
            with open("vars.toml", 'r') as f:
                loaded = toml.load(f)
            for k, v in loaded.items():
                GLOBAL_VARS[k] = v
        print_info("Global variables loaded")
        return True

    elif cmd.startswith("!LIST"):
        print("!SET <key> <value>      - Set global variable.")
        print("!GET <key>              - Get global variable.")
        print("!UNSET <key>            - Unset global variable.")
        print("!CLEAR                  - Unset all global variables.")
        print("!VARS                   - View all global variables.")
        print("!OVERWRITE              - Overwrite saved global variables with current state.")
        print("!APPEND                 - Append to saved global variables with current state.")
        print("!LOAD                   - Load all saved global variables.")
        return True

    else:
        return False
