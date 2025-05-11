# Gorfector

Gnome Observable/Organic Record Freight Extractor

Image scanning software

# Development

## Setup

1. Clone the repository
2. Configure git to use the local hooks directory:

```
git config --local core.hooksPath .githooks/
```

meson wrap install nlohmann_json
meson wrap install gtest

# Build
```
meson build scanner_settings
meson build gorfector-pot
meson build gorfector-update-po
meson build gorfector-gmo
meson build gorfector
meson install
```
