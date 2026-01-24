# librope

A high-performance C11 rope data structure for efficient text editing operations.

## What is a Rope?

A **rope** is a binary tree data structure for storing and manipulating strings. Unlike traditional contiguous string buffers (arrays), where insertions and deletions require shifting potentially millions of characters, a rope organizes text as a tree of smaller segments:

```
              [Branch]
             /        \
        [Branch]     "world!"
        /      \
    "Hello"   ", "
```

This structure enables O(log n) insertions and deletions regardless of position, making it ideal for text editors where users frequently edit in the middle of large documents.

## Key Features of This Implementation

### Multi-Dimensional Position Tracking

librope tracks different "dimensions" of text positions simultaneously:

| Unit | Description | Use Case |
|------|-------------|----------|
| `ROPE_BYTE` | Raw bytes | File I/O, binary operations |
| `ROPE_CHAR` | Grapheme clusters | Visual cursor movement ("one character") |
| `ROPE_CP` | Unicode codepoints | Unicode processing |
| `ROPE_LINE` | Newline count | Line-based navigation |
| `ROPE_UTF16` | UTF-16 code units | Interop with JavaScript/Windows APIs |

This means you can efficiently ask questions like "what is the byte offset of line 500?" or "how many visual characters are in bytes 0-1000?" without scanning the entire document.

### Compact Dimension Encoding

Dimensions are bit-packed into a single 64-bit integer per string segment:

- **Small strings (<=1020 bytes)**: All 5 dimensions encoded in 64 bits using 11 bits each
- **Large strings**: Only byte count stored; other dimensions computed on demand

This dual approach keeps metadata overhead minimal while supporting arbitrarily large text segments.

### Inline String Optimization

Small strings avoid heap allocation entirely:

| String Size | Storage |
|-------------|---------|
| <= 48 bytes | Stored inline in the node structure |
| 49-1020 bytes | Heap-allocated with reference counting |
| > 1020 bytes | Heap-allocated, dimensions computed lazily |

Most text editing involves small segments (typing characters, small pastes), so the inline path is the common case.

### Cursor System

Cursors are persistent position trackers that automatically update when the document changes:

```c
struct RopeCursor cursor;
rope_cursor_init(&cursor, rope);
rope_cursor_move_to(&cursor, ROPE_LINE, 100);  // Go to line 100
rope_cursor_insert(&cursor, "Hello", 5);       // Insert at cursor
// Cursor position automatically adjusts after any edit
```

Cursors support callbacks for reactive updates.

### Tagging System

Leaf nodes can be tagged with 63-bit bitmasks for categorization:

```c
// Tag a region as "comment"
rope_node_add_tags(node, TAG_COMMENT);

// Iterate only over tagged regions
rope_iterator_init(&iter, rope, range, TAG_COMMENT);
```

Useful for syntax highlighting, semantic annotations, or any metadata that should travel with the text.

## Building

```bash
# Configure with tests enabled
meson setup build -Dtest=true

# Build and run tests
meson compile -C build && meson test -C build
```
