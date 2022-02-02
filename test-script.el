(module-load "/Users/nealsid/src/github/emwt/build/libtest-module.dylib")
;;(describe-function 'emwt-lisp-callable)

(defmacro measure-time (&rest body)
  "Measure the time it takes to evaluate BODY."
  `(let ((time (current-time)))
     ,@body
     (message "%.06f" (float-time (time-since time)))))

(setq ntimes 10000)

(message "\nlooping call %d times" ntimes)

(measure-time
 (dotimes (i ntimes)
   (emwt-lisp-callable "Hello")))
