cmake_minimum_required(VERSION 3.21)

project(emacs-module-wrapper-template)

include_directories("/Applications/Emacs.app/Contents/Resources/include")

add_library(test-module SHARED test-module.cc)
set_property(TARGET test-module PROPERTY CXX_STANDARD 23)

add_custom_command(OUTPUT libtest-module.dylib.dSYM
  COMMAND /usr/bin/dsymutil $<TARGET_FILE_NAME:test-module>
  WORKING_DIRECTORY ${CMAKE_CURRENT_BUILD_DIR})

add_custom_target(dsym ALL DEPENDS libtest-module.dylib.dSYM test-module)
