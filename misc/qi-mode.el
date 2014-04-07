;;
;; Author(s):
;;  - Cedric GESTES <gestes@aldebaran-robotics.com>
;;
;; Copyright (C) 2014 Aldebaran Robotics
;;

(require 'generic-x)

(define-generic-mode
  'qi-mode                                        ;; name of the mode
  '("#" "//")                                     ;; comments delimiter
  '("if" "package" "end" "for"
    "interface" "fn" "in" "out" "prop"
    "object" "at")      ;; some keywords
  '(;; int constant
    ("\\b[0-9]+\\b" . font-lock-constant-face)
    ;; blabla: value
    ("^[ ]*\\([a-zA-Z]+\\)[ ]*\\(:\\)"
     (1 font-lock-variable-name-face)
     (2 font-lock-keyword-face))
    ;; @attr
    ("@[a-zA-Z]+" . font-lock-preprocessor-face)
    ;; fn/in/out/prop names
    ("^[ ]*\\(fn\\|in\\|out\\|prop\\)[ ]*\\([a-zA-Z]+\\)"
     2 font-lock-function-name-face))

  '("\\.qi$")                                     ;; files that trigger this mode
  nil                                             ;; any other functions to call
  "qilang highlighting mode"                      ;; doc string
  )
