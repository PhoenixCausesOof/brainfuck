project('brainfuck', 'cpp',
  version : '0.1',
  default_options : ['warning_level=3',
                     'cpp_std=c++20'])

subdir('include')
boost = dependency('boost', modules: 'program_options')

incdir = include_directories('include')
                      
exe = executable(
  'brainfuck', 
  'brainfuck.cpp', 
  dependencies: [boost], 
  link_with: asmjit,
  include_directories : incdir,
   cpp_args: ['-DASMJIT_STATIC'],
  install : false)

test('basic', exe)
