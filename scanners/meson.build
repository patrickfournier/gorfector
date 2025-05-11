jq = find_program('jq')
sed = find_program('sed')

scanner_strings = custom_target('scanner_strings',
    depend_files: [
        'epson/perfection_v600_photo.json',
    ],
    output: [
        'scanner_strings.h',
    ],
    command: ['strextractor.sh', '@CURRENT_SOURCE_DIR@', '@BUILD_ROOT@/scanners/scanner_strings.h'],
)
