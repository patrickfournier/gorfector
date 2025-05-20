metainfo_file = 'com.patrickfournier.gorfector.metainfo.xml.in.in'
metainfo_file = i18n.merge_file(
    input: configure_file(
        input: 'com.patrickfournier.gorfector.metainfo.xml.in.in',
        output: 'com.patrickfournier.gorfector.metainfo.xml.in',
        configuration: conf,
    ),
    output: app_id + '.metainfo.xml',
    type: 'xml',
    po_dir: podir,
    install: true,
    install_dir: join_paths (get_option ('datadir'), 'metainfo')
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
