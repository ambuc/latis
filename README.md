# 🏁 latis
## (pronounced "_lattice_")
A command-line spreadsheet-style utility. Written in `C++`. Mostly a testing
grounds for writing a parser in the parser-combinator style. (See
`/src/formula/...`.) Might eventually implement a TUI for opening and editing
`.latis` files, performing spreadsheet computations, who knows.

## Developing

### Building locally

Your local `.bazelrc` should contain:

```
build --cxxopt='-std=c++2a'
```

For ncurses stuff on Debian/Ubuntu:

```
sudo apt-get install libncurses5-dev libncursesw5-dev
```

### Submitting changes

Use `addlicense` (https://github.com/google/addlicense) to ensure all 
necessary files have the Apache license: `addlicense -c "Google LLC" -l apache .`.
