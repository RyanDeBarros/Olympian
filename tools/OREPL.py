import ToolRegistry


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
            cmd = input(f"[{current.get_path() or '/'}] > ").strip()
        except KeyboardInterrupt:
            print("\n^C - cancelled input.")
            continue

        if not cmd:
            continue

        if cmd == "exit":
            break
        elif cmd == "list":
            for child in current.children.values():
                print(f"{child.name:<24} - {child.description}")
        elif cmd == "..":
            if current.parent:
                current = current.parent
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
            print("Unknown command. Enter 'list' for a list of options, '..' to go return to the preceding command, "
                  "and 'exit' to quit OREPL.")


if __name__ == "__main__":
    repl(ToolRegistry.ROOT)
