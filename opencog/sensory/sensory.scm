;
; Opencog sensory atom-types module
;
(define-module (opencog sensory))

(use-modules (opencog))
(use-modules (opencog sensory-config))

; Load the C library that calls the nameserver to load the types.
(load-extension
	(string-append opencog-ext-path-sensory-types "libsensory-types")
	"sensory_types_init")

(load-extension
	(string-append opencog-ext-path-sensory "libsensory")
	"opencog_sensory_init")

(load-extension
	(string-append opencog-ext-path-sensory "libsensory-filedir")
	"opencog_sensory_filedir_init")

;;(load-extension
;;	(string-append opencog-ext-path-sensory "libsensory-irc")
;;	"opencog_sensory_irc_init")

(load-extension
	(string-append opencog-ext-path-sensory "libsensory-terminal")
	"opencog_sensory_terminal_init")

(include-from-path "opencog/sensory/sensory_types.scm")
