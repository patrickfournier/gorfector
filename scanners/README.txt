This folder contains files used to customize how scanners settings are displayed in ZooScan.

The files are in JSON format and are grouped by vendor. To generate a file for a scanner:

- Start ZooScan in development mode (using the `--dev` option)
- From the menu, open the `Settings...` dialog and select the `Developer` tab.
- Turn on the `Dump SANE Options` option and close the dialog.
- From the menu, open the "Select Device..." dialog and select the scanner you want to dump the options for.
  If the scanner is already selected, select another one and then select the original one again.
  This will cause the options to be dumped to the standard output. Copy the output to a file. Name the file
  according to the model of the scanner and put it in a folder named after the vendor of the scanner. Folder and
  file names should be in lowercase, with spaces replaced by underscore.

Now you can customize the options in the file:

- `title` is the title of the setting.
- `description` is the description of the setting.
- `string_list` is a list of strings that will be displayed in a combo box, for settings that have a list
  of string values. Do not change the order of the strings.
- `flags` is a list of strings used to override attributes on the setting. Scanners can mark a setting as
  advanced, display only or hidden (as indicated in the `comment` field). Use the flags to override this.
  The possible flag values are:
  - `ForceBasic`: force the setting to be displayed in the basic tab.
  - `ForceAdvanced`: force the setting to be displayed in the advanced tab.
  - `ForceShown`: force the setting to be displayed. Note that this may not make sense for some settings combinations.
  - `ForceHidden`: force the setting to be hidden.
  - `ForceReadOnly`: force the setting to be read only.

Before starting customizing the file, you should check the `option_hash` value. This is a hash of the scanner
options. If there is already a file with the same hash, you should probably modify that file instead of adding
a new one. To make an existing file be used for a new scanner, add the model and vendor to the `devices`
list in the file. Then do any other modifications you want to do.
