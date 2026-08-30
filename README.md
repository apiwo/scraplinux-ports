# scraplinux-ports

Source recipes for ScrapLinux, organized by repo under `ALL/`. This is the
real `ports/` tree the build system reads from, and what `scraps add -s <pkg>`
fetches — one recipe at a time, as `ALL/<repo>/<name>/recipe`.

Recipes are POSIX `/bin/sh`. Most are generated from `manifest.tsv`; a recipe
with a `recipe.local` beside it is hand-maintained and the generator leaves it
alone. That marker is not decoration — without it, a hand edit is silently
reverted the next time the tree is regenerated, and the recipe stops building
the package that is actually shipped.

Binaries are not here. They are at
[scraplinux-pkgs](https://github.com/apiwo/scraplinux-pkgs), on a
different host, reached through a different setting: "what can be installed"
and "what can be compiled" are deliberately not the same list.
