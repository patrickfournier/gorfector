metainfo_conf = configuration_data()
metainfo_conf.set('APP_ID', app_id)

metainfo_file = custom_target(
    'metainfo_file',
    input: configure_file(
        input: 'com.patrickfournier.gorfector.metainfo.xml.in.in',
        output: '@BASENAME@',
        configuration: metainfo_conf
    ),
    output: app_id + '.metainfo.xml',
    command: [
        msgfmt,
        '--xml',
        '-d', 'po',
        '--template=@INPUT@',
        '--output-file=@OUTPUT@',
    ],
    install: true,
    install_dir: join_paths (get_option ('datadir'), 'metainfo'),
    depends: update_all_po,
)

# Validate MetaInfo file
ascli_exe = find_program('appstreamcli', required: false)
if ascli_exe.found()
    test('validate metainfo file',
         ascli_exe,
         args: ['validate',
                '--no-net',
                '--pedantic',
                metainfo_file]
    )
endif
