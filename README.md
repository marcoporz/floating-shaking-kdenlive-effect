# floating-shaking

A **Floating** effect for [Kdenlive](https://kdenlive.org): gentle,
automatic drift/hover motion (position + rotation) for a clip or PNG.
No keyframes needed — set the amplitude and speed once, and the clip
hovers for its whole duration. Good for logos, watermarks, or PNGs
placed on top of a video.

Independent from any other project/effect — this is its own small MLT
module (`libmltfloating.so`), not related to or bundled with anything
else.

> ⚠️ **Status: early / unofficial.** Works, but hasn't been tested inside
> the Kdenlive UI by more than one person yet. Not affiliated with the
> MLT or Kdenlive projects.

## How it works

The filter sums two sine waves per axis (position X, position Y,
rotation), each at a slightly different frequency, so the motion looks
organic rather than a perfectly repeating metronome. No tracking, no
AI, no external assets — just a small, cheap trigonometric function
evaluated per frame. See [CREDITS.md](CREDITS.md) for authorship.

## Requirements

- Linux, with **Kdenlive using your system's MLT** (not the Flatpak or
  AppImage build — those bundle their own MLT and won't see this module).
- MLT development headers (any reasonably recent MLT 7.x should work; this
  filter doesn't use anything exotic).
- Qt6 development headers (Gui + Widgets).
- A C/C++ toolchain and CMake.

Check your MLT version:
```bash
pkg-config --modversion mlt-framework-7
```

## Install

### openSUSE (Slowroll / Tumbleweed)
```bash
sudo zypper install libmlt-devel qt6-base-devel libX11-devel cmake gcc-c++ pkgconf-pkg-config
```

### Fedora
```bash
sudo dnf install mlt-devel qt6-qtbase-devel libX11-devel cmake gcc-c++ pkgconf-pkg-config
```

### Debian / Ubuntu
```bash
sudo apt install libmlt-dev qt6-base-dev libx11-dev cmake g++ pkg-config
```

### Arch
```bash
sudo pacman -S mlt qt6-base libx11 cmake gcc pkgconf
```

*(Package names may drift over time/distro version — if a name above is
wrong for your system, search your package manager for `mlt` and
`qt6-base` "devel"/"dev" packages and open an issue so this list can be
fixed.)*

### Build & install (all distros)
```bash
git clone https://github.com/marcoporz/floating-shaking-kdenlive-effect.git
cd floating-shaking-kdenlive-effect
./install.sh
```

`install.sh` runs `cmake` + `cmake --build`, installs the compiled module
into MLT's module directory (needs `sudo`), and copies the Kdenlive effect
XML into `~/.local/share/kdenlive/effects/`.

**Fully restart Kdenlive** afterwards (it only scans custom effects on
startup).

### Verify before opening Kdenlive (optional but recommended)
MLT's command-line tool is called `melt` on some distros (e.g. openSUSE) and
`mlt-melt` on others (e.g. Fedora, since `melt` is taken by an unrelated
package there). Try either:
```bash
melt -query filters 2>/dev/null | grep floating
# or, if the above prints nothing / "command not found":
mlt-melt -query filters 2>/dev/null | grep floating
```
Should print `- floating`. If neither works, the module isn't in MLT's
search path — open an issue with your distro and MLT version.

## Usage

1. Place a PNG/logo/clip where you want it (e.g. with Kdenlive's own
   Transform effect, or just a centered PNG).
2. Apply **"Floating"** on top. No keyframes needed — it runs for the
   whole clip automatically.
3. Adjust **Horizontal/Vertical/Rotation Amplitude** for how far it drifts,
   and **Speed** for how fast (0.1–0.3 gives a slow, dreamy hover; higher
   values look jittery/nervous).
4. If you use Floating on more than one clip at the same time (e.g. two
   overlapping logos), give each a different **Seed** so they don't drift
   in sync with each other.
5. **Rotation Pivot X/Y** (0–1, default 0.5/0.5 = center) moves the point
   the rotation spins around, relative to the whole frame.

## Uninstall
```bash
sudo rm /usr/lib64/mlt-7/libmltfloating.so   # path may differ, see install.sh output
sudo rm -rf /usr/share/mlt-7/floating
rm ~/.local/share/kdenlive/effects/floating.xml
```

## Known limitations

- 8-bit color only.
- The whole incoming frame is nudged as one image — there's no built-in
  size/position control (pair it with Kdenlive's own Transform effect, or
  a pre-positioned/centered PNG, for that).
- Not tested inside the Kdenlive UI across many different setups yet.

## License

LGPL-2.1-or-later, same as MLT. See [LICENSE](LICENSE) and
[CREDITS.md](CREDITS.md) for authorship details.
