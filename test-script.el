(module-load "build/libtest-module.dylib")
(describe-function 'emwt-lisp-callable)
(message "\nlooping call")
(dotimes (i 200000)
  (emwt-lisp-callable "Hello"))
