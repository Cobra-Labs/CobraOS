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
  'src/kernel/tar.cpp',
  'src/kernel/elf.cpp'
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

subdir('libc')       # definiert cobra_libc + libc_inc
subdir('userspace')  # definiert init_elf, nutzt cobra_libc + libc_inc

# init.elf -> initramfs.tar mit Eintrag "./init", passend zu tar_find() im Kernel
initramfs_tar = custom_target('initramfs_tar',
  input: init_elf,
  output: 'initramfs.tar',
  command: [sh, '-c',
    'mkdir -p @OUTDIR@/initramfs_root && ' +
    'cp @INPUT@ @OUTDIR@/initramfs_root/init && ' +
    'tar -cf @OUTPUT@ -C @OUTDIR@/initramfs_root ./init'
  ],
  build_by_default: true,
)

shell_script = '''
  mkdir -p build_iso/iso_root/boot/limine
  cp @INPUT0@ build_iso/iso_root/boot/
  cp ''' + limine_dir + '''/limine-bios-cd.bin build_iso/iso_root/
  cp ''' + limine_dir + '''/limine-uefi-cd.bin build_iso/iso_root/
  cp ''' + limine_dir + '''/limine-bios.sys build_iso/iso_root/boot/limine/
  cp ''' + src_root + '''/esp/boot/limine.conf build_iso/iso_root/boot/
  cp @INPUT1@ build_iso/iso_root/boot/initramfs.tar
  ''' + xorriso.full_path() + ''' -as mkisofs -b limine-bios-cd.bin -no-emul-boot -boot-load-size 4 -boot-info-table --efi-boot limine-uefi-cd.bin -no-emul-boot build_iso/iso_root -o @OUTPUT@
  ''' + limine.full_path() + ''' bios-install @OUTPUT@
'''

iso_target = custom_target('cobraos_iso',
  input: [kernel_elf, initramfs_tar],
  output: 'cobraos.iso',
  build_by_default: true,
  command: [sh, '-c', shell_script]
)
