# Credits

This filter (`src/filter_floating.cpp`) is **original code**, written for
this repository. It is not derived from, or related to, any upstream MLT
pull request or any other third-party MLT filter.

It reuses the same small set of generic image-conversion helpers
(`src/common.cpp` / `src/common.h` — QImage/MLT image conversion,
Qt application bootstrap) that are part of MLT's own `qt` module and used
by several of MLT's built-in Qt-based filters (e.g. `qtblend`), under the
same LGPL-2.1 license as the rest of MLT. See `LICENSE`.
