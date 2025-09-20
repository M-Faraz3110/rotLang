# rotLang

**rotLang** is a programming language and virtual machine I’m building in C. It’s still a work in progress, and I'm not sure when I'll be able to get back to this, but I do plan on. The main goal was to mess around and learn language design since the syntax was gonna be based on common internet slang.

## What’s Here So Far?

- **REPL:** You can play around with rotLang interactively in your terminal.
- **Run Scripts:** Pass a file to run your rotLang code from the command line.
- **Bytecode VM:** Under the hood, rotLang compiles to bytecode and runs it on a custom virtual machine.
- **Math & Comparisons:** Supports basic arithmetic (integers and doubles) and comparison operators.
- **Strings:** You can use and manipulate strings.
- **Hash Tables:** Used for storing variables and objects.
- **Error Reporting:** Handles compile-time and runtime errors with helpful messages.
- **Modular Compiler:** Easy to add new features as the language grows.

## Getting Started

### Build It

```sh
make
```

### Try the REPL

```sh
./rotLang
```

### Run a Script

```sh
./rotLang path/to/yourfile.rl
```