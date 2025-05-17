# Gorfector

G-Observable Record Freight Extractor (**Gorfector**) is an image scanning software.

Being relatively new, it has not been tested with a lot of scanners yet. Currently, it is known to work with:

- Epson Perfection V600 Photo

However, it should work with any scanner that supports the SANE protocol. Please report any issues you encounter with
other scanners so we can improve compatibility.

## Features

- User-friendly scan parameter presentation
- Zoomable, high-resolution previews
- Batch scanning of a single document (useful for scanning negatives)
- User defined presets
- Output to file (TIFF, PNG, JPEG), to email or to printer
- Adwaita GTK3 user interface
- Saves pixels straight from the scanner: no image processing is done

# Improving Gorfector

You can help improve **Gorfector**!

## Add support for your scanner

**Gorfector** uses a JSON file to define how to present the parameters of your scanner. This file is also used to
translate the parameters into the current user language. If there is no JSON file for your scanner, **Gorfector** will use 
the parameters as provided by the scanner. This means they will probably be presented in English, and some
parameters may not make sense or could be too advanced (this was the case for the Epson Perfection V600 Photo scanner).

To improve the parameter presentation for your scanner, you need to create a JSON file that defines their presentation. See
the file [SCANNER_SUPPORT.md](SCANNER_SUPPORT.md) for more information on how to create the JSON file. If you do so, please consider 
submitting a pull request with the JSON file so that others can benefit from your work.

## Add support for your language

**Gorfector** uses a PO file to define the translations for the user interface. See the file
[TRANSLATE.md](TRANSLATE.md) for more information on how to create or update the PO file. 
If you add or improve language support, please consider submitting
a pull request with the PO file so that others can use **Gorfector** in your language.

## Submit a bug report

If you encounter a bug, please submit a bug report. You can do so by creating an issue on the 
[GitHub repository](https://github.com/patrickfournier/gorfector/issues).

## Contributing code

If you want to contribute code, be it a new feature or a bug fix, please fork the repository 
and create a pull request. Discuss proposed changes in an issue before starting work. 
This will help avoid duplicate work and ensure that the changes are
in line with the goals of the project. See the file [CONTRIBUTING.md](CONTRIBUTING.md) 
for more information on how to contribute code.

# Compiling

## Setup

To compile **Gorfector**, you need to have the following tools installed:

- Meson
- Gettext
- `libxml2-utils`
- `desktop-file-utils`
- `itstool`
- a compiler that supports C++23 (e.g., GCC 13, the default compiler on Ubuntu 24.04)

On Ubuntu, you can install these tools with the following command:

```bash
  sudo apt install build-essential meson gettext libxml2-utils desktop-file-utils itstool
```

Then you will need to install the following dependencies:

- GLib, GTK4 and Adwaita
- `libsane`
- `libtiff`
- `libpng`
- `libjpeg`

On Ubuntu, you can install these dependencies with the following command:
(`libjpeg` will be pulled by `libtiff`):

```bash
  sudo apt install libglib2.0-dev-bin libgtk-4-dev libadwaita-1-dev libsane-dev libtiff-dev libpng-dev
```

Finally, you will need to clone the repository:

```bash
  git clone git@github.com:patrickfournier/gorfector.git
  cd gorfector
```

Configure `git` to use the local hooks directory, to check the code formatting before each commit:

```bash
  git config --local core.hooksPath .githooks/
```

## Build

**Gorfector** uses `meson` to build the project. To build **Gorfector**, run the 
following commands (replace `<install_dir>` with the directory where you want to 
install **Gorfector**):

```bash
  meson setup build --prefix=<install_dir>
  cd build
  meson build scanner_settings
  meson build gorfector-pot
  meson build gorfector-update-po
  meson build gorfector-gmo
  meson build gorfector
  meson build gorfector-tests
  meson install
```

To run the tests, run the following command:

```bash
  meson test -C build
```

## Running

To use **Gorfector**, power up your scanner and run the following command:

```bash
  cd <install_dir>/bin
  ./gorfector
```
