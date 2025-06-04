project('gorfector',
        'cpp',
        version : '0.2.0',
        default_options : [
                'warning_level=2',
                'cpp_std=c++23'
        ],
        meson_version: '>=1.1.0',
        license: 'GPL-3.0-or-later',
        license_files: ['LICENSE'],
)

gnome = import('gnome')

cx = meson.get_compiler('cpp')

if get_option('development')
        app_id = 'com.patrickfournier.gorfector.devel'
else
        app_id = 'com.patrickfournier.gorfector'
endif

gettext_package = meson.project_name()

datadir = join_paths(get_option('prefix'), get_option('datadir'))

conf = configuration_data()
conf.set_quoted('APP_ID', app_id)
conf.set_quoted('VERSION', meson.project_version())
conf.set_quoted('BIN_DIR', join_paths(get_option('prefix'), get_option('bindir')))
conf.set_quoted('GETTEXT_PACKAGE', gettext_package)
conf.set_quoted('DATADIR', datadir)
conf.set_quoted('GNOMELOCALEDIR',
                join_paths(get_option('prefix'), get_option('localedir'))
)

gtk4_dep = dependency('gtk4', version : '>=4.14.0', required : true)
adw_dep = dependency('libadwaita-1', version : '>=1.5.0', required : true)
libtiff_dep = dependency('libtiff-4', version : '>=4.5.0', required : true)
libjpeg_dep = dependency('libjpeg', version : '>=2.1.0', required : true)
libpng_dep = dependency('libpng', version : '>=1.6.0', required : true)
nlohmann_json_dep = dependency('nlohmann_json', required: true)
libsane_dep = cx.find_library('sane', required : true)

cxxflags = [
        '-Wall',
        '-Wextra',
        '-Werror',
        '-Wno-unused-variable',
        '-Wno-unused-but-set-variable',
        '-Wno-unused-function',
        '-Wno-unused-result',
        '-Wno-unused-parameter',
]

subdir('scanners')
subdir('help')
subdir('po')
subdir('data')
subdir('src')

gnome.post_install(
        gtk_update_icon_cache: true,
        update_desktop_database: true,
)
