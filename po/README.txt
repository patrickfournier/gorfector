To add a new translation:

- Add the language code to the `LINGUAS` file
- Add the .po file to the `po_files` list in `meson.build`
- Build the `update_all_po` target to create the new .po file
