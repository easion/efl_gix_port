mods = []

if get_option('x11')
  mods +=['ibus', 'xim', 'scim']
endif

if get_option('gix')
#  mods +=['ibus', 'xim', 'scim']
endif
if get_option('wl')
  mods += ['wayland']
endif

loader_disabler = get_option('ecore-imf-loaders-disabler')

foreach mod_name : mods
  mod_src = []
  mod_deps = []

  if loader_disabler.contains(mod_name) == false
    if mod_name == 'ibus'
      if get_option('glib') == false
        error('Need glib enabled if ibus is enabled')
      endif
    endif
    mod_install_dir =  join_paths(dir_package_modules, mod_name, version_name)
    subdir(mod_name)
    module_files += join_paths(mod_install_dir, 'lib'+mod_name+'.'+sys_mod_extension)
    config_h.set('BUILD_ECORE_IMF_'+mod_name.to_upper(), '1')
  endif
endforeach
