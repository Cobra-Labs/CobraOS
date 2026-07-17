libc_sources = files(
  'src/crt0.S',
  'src/syscall.S',
  'src/unistd.cpp'
)

libc_inc = include_directories('include')

cobra_libc = static_library('c',
  libc_sources,
  include_directories: libc_inc,
)
