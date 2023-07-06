ecore_gix_deps = [eina, ecore, ecore_input]
ecore_gix_pub_deps = []
ecore_gix_ext_deps = [m]

ecore_gix_src = files([
  'ecore_gix.c',  'ecore_gix_dnd.c',  'ecore_gix_selection.c',  'ecore_illume.c',   'ecore_utils.c'
])

ecore_gix_header_src = [
  'Ecore_Gix.h',
  'ecore_Gix_Keys.h'
]

ecore_gix_ext_deps += dependency('gix')

ecore_gix_lib = library('ecore_gix',
    ecore_gix_src,
    dependencies: [ecore_gix_deps, ecore_gix_pub_deps, ecore_gix_ext_deps],
    include_directories : config_dir + [include_directories(join_paths('..','..'))],
    install: true,
    c_args : package_c_args,
)

ecore_gix = declare_dependency(
  include_directories: [include_directories('.')],
  link_with: ecore_gix_lib,
  dependencies: ecore_gix_pub_deps,
)

install_headers(ecore_gix_header_src,
  install_dir : dir_package_include,
)
