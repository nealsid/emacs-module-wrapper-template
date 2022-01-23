(module-load "/Users/nealsid/src/github/emwt/build/libtest-module.dylib")
(message "\nlooping call")
(dotimes (i 20000)
  (emwt-lisp-callable "Hello" 5 10))
