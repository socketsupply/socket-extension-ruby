MRuby::Gem::Specification.new('socket-runtime') do |spec|
  spec.license = 'MIT'
  spec.summary = 'Standard library for Ruby running on the Socket Runtime'
  spec.author = 'Socket Supply, Co <https://socketsupply.co>'
  spec.homepage = 'https://github.com/socketsupply/socket-ruby'
  spec.cc.include_paths << '../include/'
  spec.add_dependency('mruby-require', :github => 'mattn/mruby-require')
end
