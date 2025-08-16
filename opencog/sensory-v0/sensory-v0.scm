;
; Opencog sensory atom-types module
;
(define-module (opencog sensory-v0))

(use-modules (opencog))
(use-modules (opencog sensory-v0-config))

; Load the C library that calls the nameserver to load the types.
(load-extension
	(string-append opencog-ext-path-sensory-types "libsensory-v0-types")
	"sensory_v0_types_init")

(load-extension
	(string-append opencog-ext-path-sensory "libsensory-v0")
	"opencog_sensory_v0_init")

(load-extension
	(string-append opencog-ext-path-sensory "libsensory-v0-filedir")
	"opencog_sensory_v0_filedir_init")

(load-extension
	(string-append opencog-ext-path-sensory "libsensory-v0-irc")
	"opencog_sensory_v0_irc_init")

(load-extension
	(string-append opencog-ext-path-sensory "libsensory-v0-terminal")
	"opencog_sensory_v0_terminal_init")

(include-from-path "opencog/sensory-v0/sensory_v0_types.scm")
