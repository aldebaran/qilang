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
  '(("\\b[0-9]+\\b" . font-lock-constant-face)    ;; int constant
    ("^[ ]*\\([a-zA-Z]+\\)[ ]*\\(:\\)"            ;; blabla: value
     (1 font-lock-variable-name-face)
     (2 font-lock-keyword-face))
    ("@[a-zA-Z]+" . font-lock-preprocessor-face)) ;; @attr
  '("\\.qi$")                                     ;; files that trigger this mode
  nil                                             ;; any other functions to call
  "qilang highlighting mode"                      ;; doc string
  )
