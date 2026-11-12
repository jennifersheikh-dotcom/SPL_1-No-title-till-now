# MEOW CLI — Module 1

A small C-based command-line application scaffold. It displays the MEOW logo and welcome screen, then accepts commands in an interactive prompt.

## Requirements

- Ubuntu or another Linux distribution
- GCC
- GNU Make

No external libraries are used. The program uses standard C headers and the POSIX `isatty()` function.

## 1. Install the compiler tools

```bash
sudo apt update
sudo apt install build-essential
```

## 2. Open the project folder

If you extracted the ZIP into your Downloads folder:

```bash
cd ~/Downloads/meow_cli
```

If you extracted it somewhere else, `cd` to that folder instead.

## 3. Compile

```bash
make
```

This creates an executable named `meow` in the project folder.

You can also compile directly:

```bash
gcc -std=c11 -Wall -Wextra -Wpedantic -O2 src/meow.c -o meow
```

## 4. Run it

```bash
./meow
```

Try the startup display options:

```bash
./meow --wide
./meow --no-shadow
./meow --no-color
./meow --wide --no-shadow
```

## 5. Try the commands

At the `meow>` prompt, enter:

```text
help
version
explorer
assign
compress
clear
exit
```

`explorer`, `assign`, and `compress` are intentional placeholders in Module 1. They confirm that the command dispatcher works; the actual features will be added in later modules.

## 6. Run MEOW like a system command (optional)

To install the executable into `/usr/local/bin`:

```bash
sudo make install
```

Now you can run it from any directory:

```bash
meow
```

To uninstall it:

```bash
sudo make uninstall
```

## 7. Rebuild after changing the code

```bash
make clean
make
./meow
```

## Project structure

```text
meow_cli/
├── Makefile
├── README.md
└── src/
    └── meow.c
```

## Notes

- This first module intentionally does not perform file operations yet.
- The program is written in C, not C++.
- No third-party packages or external libraries are linked.
- ANSI escape sequences are used for terminal colors and clearing the screen.
