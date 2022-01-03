cmake_minimum_required(VERSION 3.21)

project(emacs-module-wrapper-template)

include_directories("/Applications/Emacs.app/Contents/Resources/include")

add_library(test-module SHARED test-module.cc)
set_property(TARGET test-module PROPERTY CXX_STANDARD 17)
