import sys
import gdb

# Constants
ROPE_NODE_TYPE_MASK = 0x3FFFFFFFFFFFFFFF
TYPE_INLINE_LEAF = 0
TYPE_BRANCH = 1

NODE_TYPES = {
    TYPE_INLINE_LEAF: "INLINE",
    TYPE_BRANCH: "BRANCH"
}

def get_rope_ptr_type():
    return gdb.lookup_type("struct RopeNode").pointer()

def safe_get_string(addr, length):
    """Safely reads memory and returns a formatted string."""
    if not addr or int(addr) == 0:
        return "<NULL>"
    try:
        inferior = gdb.selected_inferior()
        mem = inferior.read_memory(addr, length)
        return mem.tobytes().decode('latin-1').encode('unicode_escape').decode('ascii')
    except Exception as e:
        return f"<Error: {e}>"

def resolve_input_to_ptr(arg):
    """Resolves a string expression or hex address to a RopeNode pointer."""
    rope_ptr_type = get_rope_ptr_type()
    try:
        val = gdb.parse_and_eval(arg)
        # If it's the struct itself, take the address
        if val.type.code == gdb.TYPE_CODE_STRUCT:
            return val.address
        return val.cast(rope_ptr_type)
    except gdb.error:
        try:
            return gdb.Value(int(arg, 0)).cast(rope_ptr_type)
        except (ValueError, gdb.error):
            return None

# Dimension bitfield layout:
# | bits    |                  |
# |---------|------------------|
# | 00 - 11 | bytes            |
# | 12 - 23 | chars            |
# | 24 - 35 | codepoints       |
# | 36 - 47 | newlines         |
# | 48 - 59 | utf16 codepoints |
# | 60 - 64 | last char size   |
def dim_to_dict(dim):
	dim = int(dim)
	return {
		'bytes': (dim & 0xFFF),
		'chars': (dim >> 12) & 0xFFF,
		'codepoints': (dim >> 24) & 0xFFF,
		'newlines': (dim >> 36) & 0xFFF,
		'utf16_cps': (dim >> 48) & 0xFFF,
		'last_char_size': (dim >> 60) & 0x1F,
	}

class DumpRopeTree(gdb.Command):
    """Print the entire RopeNode tree structure."""

    def __init__(self):
        super(DumpRopeTree, self).__init__("rope-dump", gdb.COMMAND_USER)

    def invoke(self, arg, from_tty):
        args = gdb.string_to_argv(arg)
        if not args:
            print("Usage: rope-dump <expression_or_address>")
            return

        ptr = resolve_input_to_ptr(args[0])
        if not ptr:
            print(f"Error: Could not resolve '{args[0]}'")
            return

        print(f"--- Dumping Rope Tree at {ptr} ---")
        self.walk(ptr, 0, "")

    def walk(self, node_ptr, indent, label):
        idx_prefix = "  " * indent + label

        if not node_ptr or int(node_ptr) == 0:
            print(f"{idx_prefix}NULL")
            return

        try:
            node = node_ptr.dereference()
            node_type = int(node['type'])

            # Shared metadata
            header = f"(parent: {node['parent']}) [{NODE_TYPES.get(node_type, 'UNKNOWN')}]"

            if node_type == TYPE_BRANCH:
                branch = node['data']['branch']
                depth = int(branch['depth'])
                print(f"{idx_prefix}{header} (depth: {depth})")
                self.walk(branch['children'][0], indent + 1, "L: ")
                self.walk(branch['children'][1], indent + 1, "R: ")

            else:
                str = node['data']['leaf']['value']
                str_state = str['state']
                # Leaf handling (Inline vs Standard)
                dim = dim_to_dict(str_state['dim'])
                size = int(dim['bytes'])
                is_inline = (size <= 16)
                leaf_data = node['data']['leaf']

                # Inline data address is the address of the member; Leaf data is a pointer
                data_addr = str['u']['i']['data'].address if is_inline else str['u']['h']['data']

                content = safe_get_string(data_addr, size)
                tag_val = int(leaf_data['tags'])
                print(f"{idx_prefix}{header} ({size} bytes, tag {tag_val}) \"{content}\"")

        except gdb.error as e:
            exc_type, exc_obj, exc_tb = sys.exc_info()
            print(f"{idx_prefix}<{exc_tb.tb_lineno}: Error dereferencing {node_ptr}: {e}>")

DumpRopeTree()

class PrintDim(gdb.Command):
    """Print the entire RopeNode tree structure."""

    def __init__(self):
        super(PrintDim, self).__init__("rope-dim", gdb.COMMAND_USER)

    def invoke(self, arg, from_tty):
        args = gdb.string_to_argv(arg)
        if not args:
            print("Usage: rope-dim <dimension_value>")
            return
        try:
            dim_val = int(gdb.parse_and_eval(args[0]))
            dim_dict = dim_to_dict(dim_val)
            for k, v in dim_dict.items():
              k = f"{k}:"
              print(f"  {k:<12}{v}")
        except gdb.error as e:
            print(f"Error: Could not parse dimension value '{args[0]}': {e}")

PrintDim()
