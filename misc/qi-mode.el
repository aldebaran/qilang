;;
;; Author(s):
;;  - Cedric GESTES <gestes@aldebaran-robotics.com>
;;
;; Copyright (C) 2014 Aldebaran Robotics
;;

;; Howto install:
;;  (load-file "/fast/src/qi/qilang/misc/qi-mode.el")
;;  (add-to-list 'auto-mode-alist '("\\.qi$" . qi-mode))

(require 'generic-x)

(define-generic-mode
  'qi-mode                                        ;; name of the mode
  '("#" "//")                                     ;; comments delimiter
  '("if" "package" "end" "for"                    ;; keywords
    "interface" "fn" "emit" "prop"
    "object" "at" "package" "from" "import"
    "const" "struct" "enum" "type"
    "char" "int" "uint" "int8" "uint8" "int16" "uint16" "int32" "uint32" "int64" "uint64"
    "float" "float32" "float64"
    "str" "list" "map"
    )
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
