# Translating

Gorfector uses the GNU `gettext` system to translate the user interface. The source strings are
in the C++ source files as well as in the JSON files that define the scanner parameters, in the `scanners` directory. 

To update the translation files, you need to run the following commands at the repository root 
(assuming you have set up `meson` to use the `build` directory):

```bash
  meson compile scanner_strings -C build
  meson compile gorfector-pot -C build
  meson compile gorfector-update-po -C build
```

This will update the `gorfector.pot` file and all the `*.po` files in the `po` directory. You are now ready to edit
the translations. Once you are done, you can run the following command to compile the translations:

```bash
  meson compile gorfector-gmo -C build
```

This will compile the `*.po` files into `*.mo` files, which are used by the program at runtime. 
You can then run the following command to install the translations:

```bash
  meson install -C build
```
