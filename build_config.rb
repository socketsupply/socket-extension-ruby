SSC_PLATFORM_TARGET = ENV['SSC_PLATFORM_TARGET']

ANDROID_HOME = ENV['ANDROID_HOME']
ANDROID_PLATFORM = "33"
ANDROID_NDK_VERSION = "25.0.8775105"
ANDROID_NDK = "#{ANDROID_HOME}/ndk/#{ANDROID_NDK_VERSION}"
ANDROID_NDK_BUILD = "#{ANDROID_NDK}/ndk-build"

HOST = %x[uname -s].strip.downcase
ARCH = %x[uname -m].strip.downcase

SOCKET_HOME = /SOCKET_HOME=(.*)/.match(%x[ssc env])[1].strip.gsub(/\/$/, '')
SOCKET_HOME_INCLUDE = "#{SOCKET_HOME}/include"
SOCKET_RUBY_GEMBOX = 'full-core'

def configure (conf, arch = ARCH, platform = 'desktop')
  conf.bins = []
  conf.disable_presym

  cflags = []
  ldflags = []

  cflags << "-I#{SOCKET_HOME_INCLUDE}"
  cflags << "-I#{SOCKET_HOME}"
  ldflags << "-L#{SOCKET_HOME}/lib/#{arch}-#{platform}"
  ldflags << "-lsocket-runtime"

  case platform
  when 'desktop'
    if RUBY_PLATFORM =~ /darwin/i
      cflags << '-Ibuild/mac/include'
    elsif RUBY_PLATFORM =~ /linux/i
      cflags << '-Ibuild/linux/include'
    elsif RUBY_PLATFORM =~ /win/i
      cflags << '-Ibuild/win/include'
    end
  when 'iPhoneOS'
    cflags << '-Ibuild/ios/include'
  when 'iPhoneSimulator'
    cflags << '-Ibuild/ios-simulator/include'
  end

  if RUBY_PLATFORM =~ /darwin/i
    if platform =~ /iphone|desktop/i
      conf.cc.flags << '-ObjC'
      conf.cxx.flags << '-std=c++2a -fvisibility=hidden -ObjC++'
      ldflags << '-framework Foundation,Security,CoreFoundation,CoreBluetooth,ObjC'
    end
  end

  puts ldflags

  conf.cc.flags << cflags
  conf.cxx.flags << cflags
  conf.linker.flags << ldflags

  if ENV['OS'] != 'Windows_NT' then
    # needed for using bundled gems
    conf.cc.flags << %w|-fPIC|
    conf.cxx.flags << %w|-fPIC| # needed for using bundled gems
  end

  ## enable full core library
  conf.gembox 'full-core'

  ## user space
  conf.gem :github => 'mattn/mruby-json'
  conf.gem 'gem'

  ## must be last
  conf.gem :github => 'mattn/mruby-require'
  conf.gem :github => 'mattn/mruby-thread'
end

if SSC_PLATFORM_TARGET == 'ios'
  MRuby::CrossBuild.new('ios') do |conf|
    sdk = %x[xcrun -sdk iphoneos -show-sdk-path].strip
    cflags = []
    ldflags = []

    cflags << "-isysroot #{sdk}"
    cflags << '-iframeworkwithsysroot /System/Library/Frameworks/'
    cflags << '-arch arm64'
    cflags << '-fembed-bitcode'
    cflags << '-D_XOPEN_SOURCE'
    cflags << "-miphoneos-version-min=#{ENV.fetch('IPHONEOS_VERSION_MIN', '14.0')}"

    ldflags << "-isysroot #{sdk}"
    ldflags << '-arch arm64'
    ldflags << "-miphoneos-version-min=#{ENV.fetch('IPHONEOS_VERSION_MIN', '14.0')}"

    ## commands
    conf.cc.command = 'xcrun -sdk iphoneos clang'
    conf.cxx.command = 'xcrun -sdk iphoneos clang++'
    conf.linker.command = 'xcrun -sdk iphoneos clang'

    ## flags
    conf.cc.flags << cflags
    conf.cxx.flags << cflags
    conf.linker.flags << ldflags

    configure(conf, 'arm64', 'iPhoneOS')
  end

elsif SSC_PLATFORM_TARGET == 'ios-simulator'
  MRuby::CrossBuild.new('ios-simulator') do |conf|
    sdk = %x[xcrun -sdk iphonesimulator -show-sdk-path].strip

    cflags = []
    ldflags = []

    cflags << "-isysroot #{sdk}"
    cflags << '-iframeworkwithsysroot /System/Library/Frameworks/'
    cflags << "-arch #{ARCH} -target #{ARCH}-apple-ios-simulator"
    cflags << '-fembed-bitcode'
    cflags << '-D_XOPEN_SOURCE'
    cflags << "-mios-simulator-version-min=#{ENV.fetch('IPHONEOS_VERSION_MIN', '14.0')}"

    ldflags << "-isysroot #{sdk}"
    ldflags << "-arch #{ARCH} -target #{ARCH}-apple-ios-simulator"
    ldflags << "-mios-simulator-version-min=#{ENV.fetch('IPHONEOS_VERSION_MIN', '14.0')}"

    ## commands
    conf.cc.command = 'xcrun -sdk iphonesimulator clang'
    conf.cxx.command = 'xcrun -sdk iphonesimulator clang++'
    conf.linker.command = 'xcrun -sdk iphonesimulator clang'

    ## flags
    conf.cc.flags << cflags
    conf.cxx.flags << cflags
    conf.linker.flags << ldflags

    configure(conf, ARCH, 'iPhoneSimulator')
  end
elsif SSC_PLATFORM_TARGET == 'android' or SSC_PLATFORM_TARGET == 'android-emulator'
  MRuby::CrossBuild.new('android-arm64-v8a') do |conf|
    case RUBY_PLATFORM
    when /darwin/i
      RUBY_PLATFORM = 'x86_64-darwin'
      sdk = "#{ANDROID_HOME}/ndk/#{ANDROID_NDK_VERSION}/toolchains/llvm/prebuilt/#{HOST}-x86_64"
    else
      sdk = "#{ANDROID_HOME}/ndk/#{ANDROID_NDK_VERSION}/toolchains/llvm/prebuilt/#{HOST}-#{ARCH}"
    end

    params = { :arch => 'arm64-v8a', :sdk_version => 33, :toolchain => :clang }
    cflags = []
    ldflags = []

    toolchain :android, params

    cflags << "--sysroot=#{sdk}/sysroot"
    ldflags << "--sysroot=#{sdk}/sysroot"

    ## commands
    conf.cc.command = "#{sdk}/bin/clang"
    conf.cxx.command = "#{sdk}/bin/clang++"
    conf.linker.command = "#{sdk}/bin/clang"

    ## flags
    conf.cc.flags << cflags
    conf.cxx.flags << cflags
    conf.linker.flags << ldflags

    configure(conf, 'arm64-v8a', 'android')
  end

  MRuby::CrossBuild.new('android-x86_64') do |conf|
    case RUBY_PLATFORM
    when /darwin/i
      RUBY_PLATFORM = 'x86_64-darwin'
      sdk = "#{ANDROID_HOME}/ndk/#{ANDROID_NDK_VERSION}/toolchains/llvm/prebuilt/#{HOST}-x86_64"
    else
      sdk = "#{ANDROID_HOME}/ndk/#{ANDROID_NDK_VERSION}/toolchains/llvm/prebuilt/#{HOST}-#{ARCH}"
    end

    params = { :arch => 'x86_64', :sdk_version => 33, :toolchain => :clang }
    cflags = []
    ldflags = []

    toolchain :android, params

    ccflags << "--sysroot=#{sdk}/sysroot"
    ldflags << "--sysroot=#{sdk}/sysroot"

    conf.cc.command = "#{sdk}/bin/clang"
    conf.cxx.command = "#{sdk}/bin/clang++"
    conf.linker.command = "#{sdk}/bin/clang"

    conf.cc.flags << cflags
    conf.cxx.flags << cflags
    conf.linker.flags << cflags

    configure(conf, 'x86_64', 'android')
  end
else
  MRuby::Build.new do |conf|
    #toolchain ENV.fetch('TOOLCHAIN', :clang)

      conf.cc.command = "clang"
      conf.cxx.command = "clang++"
      conf.linker.command = "clang"

    conf.toolchain
    conf.enable_bintest
    conf.enable_test

    configure(conf, ARCH, 'desktop')
  end
end
