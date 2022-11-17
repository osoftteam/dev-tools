(add-to-list 'load-path "~/.emacs.d/lisp")
(add-to-list 'load-path "~/.emacs.d/xterm-color")
(require 'xterm-color)
(add-to-list 'load-path "~/.emacs.d/color-theme-6.6.0")
(require 'color-theme)
(color-theme-initialize)
;;(color-theme-dark-laptop)
(color-theme-charcoal-black)
(require 'volatile-highlights)
(volatile-highlights-mode t)


(which-function-mode 1)
;;(tool-bar-mode -1)
(setq compilation-ask-about-save nil)

;; Maximum colors
;;(setq font-lock-maximum-decoration t)
 
(setq frame-background-mode 'dark)
 
(setq auto-mode-alist '(("\\.ad[bs]\\'"   . ada-mode)
                        ("\\.awk\\'"      . awk-mode)
                        ("\\.lex\\'"      . c-mode)
                        ("\\.[cy]\\'"     . c++-mode)
                        ("\\.h\\'"        . c++-mode)
                        ("\\.hxx\\'"      . c++-mode)
                        ("\\.[CH]\\'"     . c++-mode)
                        ("\\.java\\'"     . java-mode)
                        ("\\.cc\\'"       . c++-mode)
                        ("\\.hh\\'"       . c++-mode)
                        ("\\.cxx\\'"      . c++-mode)
                        ("\\.cpp\\'"      . c++-mode)
                        ("\\.rule\\'"      . c++-mode)
                        ("\\.rc\\'"       . c++-mode) ;; resource files
                        ("\\.rcv\\'"      . c++-mode)
                        ("\\.m\\'"        . matlab-mode)
                        ("\\.p[lm]\\'"    . perl-mode)
                        ("\\.cgi\\'"      . perl-mode)
                        ("\\.f\\'"      . fortran-mode)
                        ("\\.F\\'"      . fortran-mode)
                        ("\\.f90\\'"      . f90-mode)
                        ("\\.F90\\'"      . f90-mode)
                        ("\\.el\\'"       . emacs-lisp-mode)
                        ("\\.emacs\\'"    . emacs-lisp-mode)
                        ("\\.tex\\'"      . LaTeX-mode)
                        ("\\.bib\\'"      . bibtex-mode)
                        ("[Mm]akefile\\'" . makefile-mode)
                        ("\\.mak\\'"      . makefile-mode)
                        ("\\[Mm]akefile.\\'" . makefile-mode)
                        ("\\.bat\\'"      . shell-script-mode)
                        ("\\.tar\\'"      . tar-mode)
                        ("\\.php\\'"     . php-mode)
                        ("\\.html\\'"     . html-mode)
                        ("\\.jnlp\\'"     . html-mode)
                        ("\\.xml\\'"     . html-mode)
                        ("\\.pddl\\'"     . lisp-mode)
                        ("\\.css\\'"      . css-mode)
                        ("\\.py\\'"       . python-mode)
                        ("\\.yml\\'"      . yaml-mode)
                        ("\\CMakeLists\\.txt\\'" . cmake-mode)
                        ("\\.lisp\\'"     . lisp-mode)))
 
(global-font-lock-mode 1)
(transient-mark-mode t)
 
;; save backup files
(custom-set-variables
  ;; custom-set-variables was added by Custom.
  ;; If you edit it by hand, you could mess it up, so be careful.
  ;; Your init file should contain only one such instance.
  ;; If there is more than one, they won't work right.
 '(auto-save-file-name-transforms (quote ((".*" "~/.emacs.d/auto-save-list/\\1" t))))
 '(backup-directory-alist (quote ((".*" . "~/.emacs.d/auto-save-list/"))))
 '(inhibit-startup-screen t))

(defun yay ()
  "Build with cmake."
  (interactive)
  (setq-local compile-command (make --build build))
  (call-interactively 'compile))

;;shortcuts
(global-set-key (kbd "M-c") 'compile)
(custom-set-faces
  ;; custom-set-faces was added by Custom.
  ;; If you edit it by hand, you could mess it up, so be careful.
  ;; Your init file should contain only one such instance.
  ;; If there is more than one, they won't work right.
 )

(set-face-attribute 'default nil :height 220)

(setq-default tab-width 4) ; Assuming you want your tabs to be four spaces wide
(setq-default indent-tabs-mode nil)
(setq c-default-style "linux")
(defvaralias 'c-basic-offset 'tab-width)

;;colors in compile ;;;;;;;;;;;;;;
(setq compilation-environment '("TERM=xterm-256color"))

(defun my/advice-compilation-filter (f proc string)
  (funcall f proc (xterm-color-filter string)))

(advice-add 'compilation-filter :around #'my/advice-compilation-filter)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

