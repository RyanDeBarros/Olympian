from colorama import Fore, Style


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
