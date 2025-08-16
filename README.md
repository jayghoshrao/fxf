# fxf

A vim/fzf/lazygit inspired text picker and browser.

# Build instructions:
~~~bash
cmake -S . -B build && cmake --build build
~~~

# Usage

```bash
## List all installed AUR packages
fxf <(paru -Qmq)
```
