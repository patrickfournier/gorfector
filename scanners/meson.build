jq = find_program('jq')
sed = find_program('sed')

scanner_files = [
    'epson/perfection_v600_photo.json',
]

scanner_strings = custom_target('scanner_strings',
    depend_files: scanner_files,
    output: [
        'scanner_strings.h',
    ],
    command: ['strextractor.sh', '@CURRENT_SOURCE_DIR@', '@BUILD_ROOT@/scanners/scanner_strings.h'],
)

install_subdir(
    'epson',
    install_dir: join_paths(datadir, app_id, 'scanners'),
    install_mode: 'rw-r--r--',
)
