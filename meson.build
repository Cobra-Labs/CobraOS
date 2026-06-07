project('CobraOS', 'cpp',
  version: '1.0',
  default_options: [
    'warning_level=3',
    'cpp_std=c++20',
    'optimization=1',
    'b_staticpic=false'
  ]
)

cxxflags = [
  '-ffreestanding',
  '-fno-stack-protector',
  '-mno-sse',
  '-mno-sse2',
  '-mno-mmx',
  '-mno-80387'
]
add_project_arguments(cxxflags, language: 'cpp')

ldflags = [
  '-nostdlib',
  '-z', 'max-page-size=0x1000',
  '-static'
]
add_project_link_arguments(ldflags, language: 'cpp')

sources = files(
  'src/kernel/kernel.cpp',
  'src/paging/pmm.cpp',
  'src/paging/vmm.cpp',
  'src/arch/gdt.cpp',
  'src/arch/idt.cpp',
  'src/arch/apic.cpp',
  'src/arch/tss.cpp',
  'src/arch/syscall.cpp',
  'src/kernel/process.cpp',
  'src/kernel/tar.cpp'
)

inc = include_directories('src/include')

kernel_elf = executable('cobraos',
  sources,
  include_directories: inc,
  link_args: ['-T', meson.current_source_dir() / 'src/linker.ld'],
  link_depends: files('src/linker.ld'),
  name_suffix: 'elf'
)

xorriso = find_program('xorriso')
limine = find_program('limine')
sh = find_program('sh')

limine_dir = '/usr/share/limine'
src_root = meson.project_source_root()

shell_script = '''
  mkdir -p build_iso/iso_root/boot/limine
  cp @INPUT@ build_iso/iso_root/boot/
  cp ''' + limine_dir + '''/limine-bios-cd.bin build_iso/iso_root/
  cp ''' + limine_dir + '''/limine-uefi-cd.bin build_iso/iso_root/
  cp ''' + src_root + '''/esp/boot/limine.conf build_iso/iso_root/boot/
  cp ''' + src_root + '''/esp/boot/initramfs.tar build_iso/iso_root/boot/
  ''' + xorriso.full_path() + ''' -as mkisofs -b limine-bios-cd.bin -no-emul-boot -boot-load-size 4 -boot-info-table --efi-boot limine-uefi-cd.bin -no-emul-boot build_iso/iso_root -o @OUTPUT@
  ''' + limine.full_path() + ''' bios-install @OUTPUT@
'''

iso_target = custom_target('cobraos_iso',
  input: kernel_elf,
  output: 'cobraos.iso',
  build_by_default: true,
  command: [sh, '-c', shell_script]
)
