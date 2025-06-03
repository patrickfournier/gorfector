# To add a new language

- Add the name of the new language to the `meson.build` file.
- Build the `help-gorfector-update-po` target to create the new .po file.
- Edit the new .po file.

# Updating the manual pages

- If you added a new page, add it to the `meson.build` file.
- Build the `help-gorfector-pot` target to update the .pot file.
- Build the `help-gorfector-update-po` target to update the .po files.
- Update the translations in the .po files, if you can.
