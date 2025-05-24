# Translating

## Translating the User Interface

Gorfector uses the GNU `gettext` system to translate the user interface. The source strings are
in the C++ source files as well as in the JSON files that define the scanner parameters, in the `scanners` directory. 

To add a new language, you need to edit the `po/meson.build` file and add the new language to the list of languages.
Then, run the following command at the repository root  to generate the new `*.po` file:

```bash
  meson compile update_all_po -C build
```

This will update the `gorfector.pot` file (if needed) and all the `*.po` files in the `po` directory. 
You are now ready to edit the translations. Once you are done, you can run the following command to compile 
the translations:

```bash
  meson compile compile_all_mo -C build
```

This will compile the `*.po` files into `*.mo` files, which are used by the program at runtime. 
You can then run the following command to install the translations:

```bash
  meson install -C build
```

Your translations are now installed and ready to be used.

## Updating the Help Files

The help files are in the `help` directory. To add a new language, you need to edit the `help/LINGUAS` file
and add the new language to the list of languages. Then, run the following command at the repository root to generate
the new `*.po` file:

```bash
  meson compile help-gorfector-update-po -C build
```

Edit the `*.po` file to add your translations. Once you are done, you can run the following command to compile
the translations:

```bash
  meson compile help-gorfector-<lang>-gmo -C build
```

To install the translations, run the following command:

```bash
  meson install -C build
```
