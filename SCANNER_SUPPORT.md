# Scanner support

**Gorfector** uses SANE to communicate with scanners, so it should work with any scanner that has a SANE driver.
SANE is a standard for scanner drivers on Linux and other Unix-like operating systems. It provides a common API for
scanner manufacturers to implement their drivers, allowing applications like **Gorfector** to interact with a wide range of scanners.

Among other things, scanners use SANE to provide a list of user-adjustable parameters to front ends like **Gorfector**.
However, some parameters may not be user-friendly or may not make sense to the average user. This is
why **Gorfector** uses a JSON file to define how to present the scanner parameters. This file is also used as 
a source for providing translations of the parameter names and descriptions.

If there is no scanner support file for your scanner, **Gorfector** will use the parameters as provided by the scanner. 
This means they will probably be presented in English, and some parameters may not make sense or could be too advanced.
The good news is that you can create a JSON file to define how the parameters are presented.

## Creating a scanner support file

To create a support file for your scanner, open a terminal and run `gorfector --dev` to activate the developer mode. After the program starts,
click to the menu button, open the "Settings" dialog and select the "Developer" tab. There, enable the 
"Dump SANE Options" switch. Now, whenever you select a scanner, its options will be dumped to the standard output.
To see this, select the "Select Device" item under the menu button. In the dialog that opens, select your scanner (if 
it is already selected, deselect it by choosing 'None,' then re-select your scanner). Copy the JSON output to a file and save it as 
`scanners/<scanner_manufacturer>/<scanner_model>.json`. You can now edit the file to define the parameter presentation.

## Editing the scanner support file

Open the file in any text editor.

- To rename a parameter, edit its `title` field.
- To change the description of a parameter, edit its `description` field.
- Some parameters take their value from a list of strings. You can edit the value strings, but you must keep their order intact.
- The scanner may mark a parameter as "advanced". In this case, the parameter will be placed in the "Advanced" tab. You can change this
  behavior by adding the `ForceBasic` flag to the `flags` list.
- Conversely, you can force a parameter to be in the "Advanced" tab by adding the `ForceAdvanced` flag to the `flags` list.
- The scanner may mark a parameter as hidden. You can make it visible by adding the `ForceShown` flag to the `flags` list.
- Conversely, you can force a parameter to be hidden by adding the `ForceHidden` flag to the `flags` list.
- You can force a parameter to be read-only by adding the `ForceReadOnly` flag to the `flags` list.

Some things to keep in mind:

- You cannot change the parameter grouping.
- You cannot change the parameter ordering.
- **Do not change the parameter `id` field.**
- The `x-resolution`, `y-resolution`, `resolution`, `tl-x`, `tl-y`, `br-x` and `br-y` parameters are automatically 
  put in the "Common Options" groups of the "Basic" tab. You cannot change this behavior.
- To ensure consistency and reduce the amount of translation work, use the same titles and descriptions as the 
  other scanners for parameters that serve the same purposes. Look at the other JSON files in the `scanners` directory for examples.

## Add the file to the build system

To add the file to the build system, edit the `meson.build` file in the `scanners` directory. Add a line to the `scanner_files` list.

## Translating the scanner support file

To make the parameter strings available to the translation system, run these commands at the repository root
(assuming you have set up `meson` to use the `build` directory):

```bash
  meson compile scanner_strings -C build
  meson compile gorfector-pot -C build
  meson compile gorfector-update-po -C build
```

This will update the `gorfector.pot` file and all the `*.po` files in the `po` directory. You (or a translator) will 
now be able to translate the strings in the `*.po` files.

## Testing your changes

To test your changes locally, copy the JSON file somewhere under `$HOME/.config/com.patrickfournier.gorfector/scanners/`.
It will be detected by **Gorfector** at startup. If it is not the case, try deleting your 
`$HOME/.config/com.patrickfournier.gorfector/scanners/index.json` file.

To test your changes as part of an install, run the following command at the repository root 
(assuming you have set up `meson` to use the `build` directory):

```bash
  meson install -C build
```

Your file should be copied to 
`<install_dir>/share/com.patrickfournier.gorfector/scanners/<scanner_manufacturer>/<scanner_model>.json`,
where it will be discovered by **Gorfector** at startup. If it is not the case, try deleting your
`$HOME/.config/com.patrickfournier.gorfector/scanners/index.json` file.

## Submitting your changes

To make others benefit from your work, please consider submitting a pull request with the JSON file, the updated 
`gorfector.pot` file, the updated `*.po` files and the `meson.build` file.

If you are unfamiliar with the pull request process and unwilling to familiarize yourself with it, submit an issue on 
[GitHub](https://github.com/patrickfournier/gorfector/issues) with the JSON file attached. Someone should be able
to complete the process for you.
