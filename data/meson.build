iconsdir = join_paths('icons', 'hicolor', '64x64', 'apps')
install_data(
    join_paths(iconsdir, '@0@.png'.format(app_id)),
    install_dir: join_paths(datadir, iconsdir)
)

gorfector_resources = gnome.compile_resources(
    'gorfector-resources',
    'gorfector.resources.xml',
    c_name: 'gorfector_resources',
)

msgfmt = find_program('msgfmt')

desktop_conf = configuration_data()
desktop_conf.set('icon', app_id)
desktop_conf.set('version', meson.project_version())
desktop_file = custom_target(
    'desktop_file',
    input: configure_file(
        input: 'com.patrickfournier.gorfector.desktop.in.in',
        output: '@BASENAME@',
        configuration: desktop_conf
    ),
    output: '@0@.desktop'.format(app_id),
    command: [
        msgfmt,
        '--desktop',
        '-d', 'po',
        '--template=@INPUT@',
        '--output-file=@OUTPUT@',
    ],
    install: true,
    install_dir: join_paths(datadir, 'applications'),
    depends: update_all_po,
)

subdir('metainfo')
