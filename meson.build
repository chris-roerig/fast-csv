project('fastcsv', 'cpp',
  version : '0.1.0',
  default_options : ['warning_level=3', 'cpp_std=c++17'])

# Find dependencies
gtk_dep = dependency('gtk+-3.0')

# Source files
sources = ['src/main.cpp']

# Create executable
executable('fastcsv',
  sources,
  dependencies : [gtk_dep],
  install : true)
