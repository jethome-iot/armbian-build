Patches for the khadas-vims-v2019.01-mainline u-boot branch (BRANCH=edge).

Intentionally empty: that branch already boots mainline kernels as-is
(distro bootcmd finds /boot/boot.scr, bootm skips the vendor rsvmem
check for FDTs without amlogic-dt-id). Keep the legacy CoreELEC patches
in ../u-boot-meson-s4t7 out of this tree - they target a different fork.
