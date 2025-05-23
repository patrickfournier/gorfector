To add a new translation:

- Add the name of the new .po file to the `po_files` list in `meson.build` (no need to create the file).
- Build the `update_all_po` target to create the new .po file.
- Edit the new .po file.
