# scraplinux-ports

Source recipes for ScrapLinux, one per package, at `ALL/<repo>/<name>/recipe`.
This is the tree the build system reads and what `scraps add -s <pkg>` fetches.

The recipe is the only place a package is described. Edit it directly.
`ALL/ports.idx` is generated from the recipes by `scraps-repo ports ALL` and is
never edited by hand.

Binaries are not here. They are at
[scraplinux-pkgs](https://github.com/apiwo/scraplinux-pkgs), on a
different host, reached through a different setting: "what can be installed"
and "what can be compiled" are deliberately not the same list.
