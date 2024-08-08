project('steppy', 'c',
  version : '0.1',
  default_options : ['warning_level=3'])

gnome = import('gnome')

dep_gtk = dependency('gtk4')
dep_adw = dependency('libadwaita-1')
dep_sqlite = dependency('sqlite3')
dep_gio = dependency('gio-2.0')

cc = meson.get_compiler('c')
dep_m = cc.find_library('m', required : false)

conf = configuration_data()
conf.set_quoted('APP_ID', 'io.furios.Steppy')
conf.set_quoted('VERSION', meson.project_version())
configure_file(output : 'config.h',
               configuration : conf)

steppy_sources = [
  'steppy/main.c',
  'steppy/db.c',
  'steppy/sensors.c',
  'steppy/utils.c',
  'steppy/ui/widgets/progress.c',
]

steppy_lib = static_library('steppy',
                            steppy_sources,
                            dependencies : [dep_gtk, dep_adw, dep_sqlite, dep_gio, dep_m],
                            install : false)

executable('io.furios.Steppy',
           'steppy/main.c',
           link_with : steppy_lib,
           dependencies : [dep_gtk, dep_adw, dep_sqlite, dep_gio, dep_m],
           install : true)

install_data('data/io.furios.Steppy.desktop',
             install_dir : get_option('datadir') / 'applications')

install_data('data/io.furios.Steppy.gschema.xml',
             install_dir : get_option('datadir') / 'glib-2.0' / 'schemas')

meson.add_install_script('meson_post_install.sh')
