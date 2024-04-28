;
; filesys.scm -- filesystem demo
;
; Demo agent commands for navigating a filesystem.
;
(use-modules (opencog) (opencog exec) (opencog sensory))

(define fsys-stream
	(cog-execute! (Lookup (Type 'FileSysStream))))

(cog-value->list fsys-stream)

; Throws because wrong number of args. XXX FIXME
(cog-execute! (Lookup (Type 'TextFileStream)))

; --------------------------------------------------------
; The End! That's All, Folks!
