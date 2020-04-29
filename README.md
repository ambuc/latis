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

# TODOS

*  Need to implement `latis::ui::Window` and `latis::ui::Pad` such that they
   share a similar API, but Pad can be subaddressed with subrectangle
   coordinates
*  Need to design `latis::ui::Gridbox` to handle printing multiple forms at
   locations without the border of a `latis::ui::Textbox`.
   *  Should `Gridbox` hold unique_ptrs to other textboxes? 
   *  Should it hold the set of callbacks?
   *  Can we decompose the grid into broad sections to avoid the number of
      widgets which need to handle bubble-style calls?
*  Broadly: need to figure out how to test methods related to ncurses. Even
   printing a total screen dump and asserting text contents would be helpful.
   *  Can ncurses and gtest be compiled together?
*  Should I be pulling out _all_ ncurses-specific wrappers into a library
   further for future generalziation?
   *  To that end: I don't think anything under `latis::ui::*` is really
      `Ssheet-` or `Latis-` aware. Maybe it should be a totally separate src
      tree.
