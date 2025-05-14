bindir = join_paths(get_option('prefix'), get_option('bindir'))
applications_dir = join_paths(datadir, 'applications')

desktop_file_config = configuration_data()
desktop_file_config.set('bindir', bindir)
desktop_file = configure_file(
    input: 'com.patrickfournier.gorfector.desktop.in',
    output: '@0@.desktop'.format(app_id),
    configuration: desktop_file_config,
)

install_data(
    desktop_file,
    install_dir: applications_dir,
)

install_data(
    'gorfector.png',
    install_dir: join_paths(datadir, 'icons/hicolor/48x48/apps')
)
