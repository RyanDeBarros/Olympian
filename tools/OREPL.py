import ToolRegistry
from ToolNode import var_cmd, varinput, print_error


DESCRIPTION_OFFSET = 64


def resolve_path(current, path, root):
    if path.startswith("/"):
        # absolute path
        node = root
        parts = path.strip("/").split("/")
    else:
        # relative path
        node = current
        parts = path.strip("/").split("/")

    for part in parts:
        if part == "..":
            if node.parent:
                node = node.parent
        elif part in node.children:
            node = node.children[part]
        else:
            return None
    return node


def repl(root):
    current = root
    while True:
        try:
            cmd = varinput(f"[{current.get_path() or '/'}] > ").strip()
        except KeyboardInterrupt:
            print("\n^C - cancelled input.")
            continue

        if not cmd:
            continue

        if cmd == "exit":
            break
        elif cmd == "list":
            for child in current.children.values():
                print(f"{child.name:<{DESCRIPTION_OFFSET}} - {child.description}")
        elif cmd == "..":
            if current.parent:
                current = current.parent
        elif cmd.startswith("!"):
            if var_cmd(cmd):
                continue
            else:
                print_error(
                    "Unknown command. Enter 'list' for a list of options, '..' to go return to the preceding command, "
                    ", '!LIST' for a list of special commands, or 'exit' to quit OREPL.")
        elif "/" in cmd:
            node = resolve_path(current, cmd, root)
            if node is None:
                print("Invalid command path.")
                continue
            if node.action:
                try:
                    node.action()
                except KeyboardInterrupt:
                    print("\n^C - cancelled input.")
            else:
                current = node
        elif cmd in current.children:
            node = current.children[cmd]
            if node.action:
                try:
                    node.action()
                except KeyboardInterrupt:
                    print("\n^C - cancelled input.")
            else:
                current = node
        else:
            print_error(
                "Unknown command. Enter 'list' for a list of options, '..' to go return to the preceding command, "
                ", '!LIST' for a list of special commands, or 'exit' to quit OREPL.")


if __name__ == "__main__":
    repl(ToolRegistry.ROOT)
