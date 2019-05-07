;;(load "/usr/share/emacs/24.5/lisp/emacs-lisp/regexp-opt.el")

(defvar badge-mode-hook nil)

(defvar badge-mode-map
  (let ((map (make-keymap)))
    map)
  "Keymap for Badge major mode")

(add-to-list 'auto-mode-alist '("\\.bdg\\'" . badge-mode))

"font-lock-builtin-face 	font-lock-comment-delimiter-face
font-lock-comment-face 	font-lock-constant-face
font-lock-doc-face 	font-lock-function-name-face
font-lock-keyword-face 	font-lock-negation-char-face
font-lock-preprocessor-face 	font-lock-reference-face
font-lock-string-face 	font-lock-syntactic-face-function
font-lock-type-face 	font-lock-variable-name-face
font-lock-warning-face"

(defconst badge-keywords
  (list "let" "set" "lambda" "return"
		"if" "then" "elif"
		"else" "loop" "break"))
(defun get-badge-keywords ()
  (regexp-opt badge-keywords 'symbols))

(defconst badge-builtins
  (list "for" "this" "or" "and" "not" "print" "println"))
(defun get-badge-builtins ()
  (regexp-opt badge-builtins 'symbols))

(defconst badge-constants
  (list "nothing"))
(defun get-badge-constants ()
  (regexp-opt badge-constants 'symbols))

(defvar badge-font-lock nil
  "Default highlighting expressions for Badge mode")
(setq badge-font-lock-keywords
	  (list `(,(get-badge-keywords) (0 font-lock-keyword-face))
			`(,(get-badge-builtins) (0 font-lock-builtin-face))
			`(,(get-badge-constants) (0 font-lock-constant-face))
			'("\\(@\\)\\([_A-Za-z0-9]+\\)" (0 font-lock-preprocessor-face))
			'("'" (0 font-lock-constant-face))
			;;'("%[^\n]*" (0 font-lock-comment-face))))
			))

(defvar badge-mode-syntax-table nil
  "Badge mode syntax table")
(setq badge-mode-syntax-table
	  (let ((table (make-syntax-table)))
		(modify-syntax-entry ?% "<" table)
		(modify-syntax-entry ?\n ">" table)
		(modify-syntax-entry ?\" "\"" table)
		(modify-syntax-entry ?\[ ". 1b" table)  ; `.` indicates
												; punctuation. `1b`
												; indicates that it is
												; the first character
												; of a two-character
												; comment sequence
		(modify-syntax-entry ?\- ". 23b" table) ; 23b indicates that
												; it is the second
												; character of a
												; two-character
												; comment sequence,
												; and first character
												; of a two-character
												; comment-ending
												; sequence
		(modify-syntax-entry ?\] ". 4b" table)  ; 4b indicates that it
												; is the second
												; character of a
												; two-character
												; commend-ending
												; sequence
		table))

(define-derived-mode badge-mode text-mode
  "Major mode for editing Badge source files"
  (interactive)
  (kill-all-local-variables)
  (set-syntax-table badge-mode-syntax-table)
  (use-local-map badge-mode-map)
  
  (set (make-local-variable 'font-lock-defaults) '(badge-font-lock-keywords))

  (setq major-mode 'badge-mode)
  (setq mode-name "Badge")
  (run-hooks 'badge-mode-hook))

(provide 'badge-mode)
