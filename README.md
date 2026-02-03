<img align=right src="./logo.svg">

e - a text editor
=================

*e* will be a text editor written in C. It'll follow a transparent client-server
to allow concurrent editing of the same file(s). tree-sitter will be used for
syntax highlighting and basic code analysis.

*e* isn't ready for use (or even testing) yet.

## ToDo

- [x] rope data structure
- [x] range/cursor handling
- [ ] command framework (do/undo)
- [ ] daemon lifecycle
- [ ] tree-sitter integration
- [ ] command line frontend
- [ ] vt100 frontend

### Concepts

- e-editor
  - **0-n** documents representing opened files
    - **0-n** documents editors
      - **0-n** cursors
        - cursor position
        - edit actions

### protocol

The *e* editor accepts a very simple line based clear text message protocol. A
message consists of a sequence of arguments. Each argument is seperated by a
space. If an argument contains spaces it must be escaped using backslashes, be
enclosed in quotes or be sent as a trailing payload.

These three messages are equivalent:

```
ping Hello\ World!
ping "Hello World!"
ping @12
Hello World!
```

The `@` symbol indicates that the following number is the length of a trailing
payload. The payload is sent in the next line. If multiple trailing payloads
are needed they can be chained:

```
ping @10 @10
Argument 1
Argument 2
```

Note that each trailing payload must be followed by a newline.

> [!WARNING] The following is unimplemented

It is also possible to use termination markers to indicate the end of an
argument.

```
ping @EOT
Hello World!
@EOT
```

The Marker must start with an `@` symbol and be followed by an identifier. The
identifier must start with a letter and can contain letters, numbers and
underscores. Note that the marker must be unique within the transmission and
must be placed at the start of a line. The newline before the marker isn't
considered part the argument value.

There's the reserved marker `@EOF` which indicates that the rest
of the transmission is part of the argument.

The client always sends a message starting with a verb.
