iconsdir = join_paths('icons', 'hicolor', '48x48', 'apps')
install_data(
    join_paths(iconsdir, '@0@.png'.format(app_id)),
    install_dir: join_paths(datadir, iconsdir)
)

gorfector_resources = gnome.compile_resources(
    'gorfector-resources',
    'gorfector.resources.xml',
    c_name: 'gorfector_resources',
)

desktop_conf = configuration_data()
desktop_conf.set('icon', app_id)
desktop_file = i18n.merge_file(
    type: 'desktop',
    input: configure_file(
        input: 'com.patrickfournier.gorfector.desktop.in.in',
        output: '@BASENAME@',
        configuration: desktop_conf
    ),
    output: '@0@.desktop'.format(app_id),
    po_dir: podir,
    install: true,
    install_dir: join_paths(datadir, 'applications')
)

subdir('metainfo')
