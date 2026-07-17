
init_elf = executable('init',
  files('init.cpp'),
  link_with: cobra_libc,
  include_directories: libc_inc,
  link_args: ['-no-pie'],
  pie: false,
  name_suffix: 'elf'
)
