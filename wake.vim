" TO INSTALL:
" 1. add this line to ~/.vimrc
" 	au BufRead,BufNewFile *.wk set filetype=wake
" 2. move this file to ~/.vim/syntax/wake.vim

" Vim syntax file
" Language:     Wake
" Maintainer:   Michael Fairhurst
" Last Change:  September 12, 2013
" Version:      1

" Copied from the cfengine, ruby, and perl syntax files
" For version 5.x: Clear all syntax items
" For version 6.x: Quit when a syntax file was already loaded
if version < 600
  syntax clear
elseif exists("b:current_syntax")
  finish
endif

setlocal iskeyword+=:
syn keyword	basickeyword	with public new return if else while foreach in import provides needs then nothing
syn match	withinregionkeyword	"\(every\|is:\)"
syn region	classdecl	start="every" end="is:" contains=inheritancekeyword,interfacekeyword,ancestrysymbols,withinregionkeyword transparent keepend
syn region	comment	start="\/\/" end="$"
syn match	hashcomment	"#\<[a-zA-Z]\+\>\?"
syn region	blockcomment	start="##" end="##"
syn keyword	inheritancekeyword	a an contained
syn keyword	interfacekeyword	capable
syn keyword	mytype	Int Truth Text Char
syn match	alias	"@\<[a-zA-Z]\+\>"
syn match	shadow	"\$"
syn match	returntype	"\(\<[a-zA-Z]\+\>\s\+\)\?--"
syn match	functioncall	")\?\<[a-zA-Z]\+\>(" contained
syn match	methodnameseg	")\?\<[a-zA-Z]\+\>("
syn match	symbols	"\((\|)\|;\||\|=\|+\|{\|}\|,\|\.\|<\|>\)"
syn match	ancestrysymbols "\((\|)\|,\)" contained
syn match	typesymbols "\(\[\|\]\)"
syn match	currysymbols "???"
syn region	mystring start=/"/ skip=/\\\\\|\\"/ end=/"/
syn region	mystring2 start=/'/ skip=/\\\\\|\\'/ end=/'/
syn match	injectionlabel	"\({\)\@<=[a-zA-Z0-9]\+\(}\)\@="

hi link comment		Comment
hi link hashcomment	Comment
hi link blockcomment	Comment

hi link basickeyword	Keyword
hi link withinregionkeyword	Keyword
hi link inheritancekeyword	Type
hi link interfacekeyword	Function

hi link mytype		Type
hi link returntype	String
hi link alias		Function
hi link shadow		Function

hi link symbols	Special
hi link ancestrysymbols	Keyword
hi link currysymbols		Function
hi link typesymbols		Type

hi link methodnameseg		Special
hi link functioncall		String
hi link mystring2		String
hi link mystring		String
hi link injectionlabel		Function

let b:current_syntax = "wake"