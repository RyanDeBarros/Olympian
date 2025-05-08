from math import inf

from Tool import varinput, print_error


def str_to_bool(s: str) -> bool:
    if s == "True" or s == "true":
        return True
    elif s == "False" or s == "false":
        return False
    else:
        raise TypeError()


def edit_optional_bool_parameter(node, parameter):
    if parameter not in node:
        inpt = varinput(f"{parameter}: ")
    else:
        inpt = varinput(f"{parameter} ({node[parameter]}): ")
    try:
        inpt = str_to_bool(inpt)
        node[parameter] = inpt
    except TypeError:
        pass


def edit_optional_bound_int_parameter(node, parameter, minimum, maximum):
    if parameter not in node:
        inpt = varinput(f"{parameter}: ")
    else:
        inpt = varinput(f"{parameter} ({node[parameter]}): ")
    if inpt.isdigit() and minimum <= int(inpt) <= maximum:
        node[parameter] = int(inpt)


def edit_bound_int_parameter(node, parameter, minimum, maximum):
    try:
        if parameter in node:
            inpt = varinput(f"{parameter} ({node[parameter]}): ")
            if not inpt:
                return
        else:
            inpt = varinput(f"{parameter}: ")
            if not inpt:
                raise TypeError()
        inpt = int(inpt)
        if inpt < minimum or inpt > maximum:
            raise TypeError()
        node[parameter] = inpt
    except TypeError:
        if maximum == inf:
            if minimum == -inf:
                print_error("Invalid input.")
            else:
                print_error(f"{parameter} must be >= {minimum}.")
        elif minimum == -inf:
            print_error(f"{parameter} must be <= {maximum}.")
        else:
            print_error(f"{parameter} must be >= {minimum} and <= {maximum}.")
        edit_bound_int_parameter(node, parameter, minimum, maximum)


