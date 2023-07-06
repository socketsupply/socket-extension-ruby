MRuby::Build.new do |conf|
  toolchain ENV.fetch('TOOLCHAIN', :clang)

  conf.toolchain


  if ENV['OS'] != 'Windows_NT' then
    conf.cc.flags << %w|-fPIC| # needed for using bundled gems
  end

  conf.gembox 'full-core'
  conf.disable_presym

  # conf.file_separator = '/'
  # conf.enable_debug
  conf.enable_bintest
  conf.enable_test
end
