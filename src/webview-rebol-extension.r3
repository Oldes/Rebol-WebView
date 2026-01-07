REBOL [
	title:  "Rebol/WebView module builder"
	type:    module
	date:    7-Jan-2026
	home:    https://github.com/Oldes/Rebol-WebView
	version: 0.1.0
	author: @Oldes
]

;- all extension command specifications ----------------------------------------
commands: [
	init-words:    [args [block!] type [block!]] ;; used internaly only!

	version: [
		"Get the library's version information"
	]
	create: [
		"Create a new WebView handle"
		/debug "Enable developer tools if supported by the backend"
	]
	eval: [
		"Evaluates arbitrary JavaScript code"
		webview [handle!] "webview instance"
		code    [string!] "JS code to evaluate"
	]
	set-title: [
		"Updates the title of the native window"
		webview [handle!] "webview instance"
		title   [string!] "new title"
	]
	set-size: [
		"Updates the size of the native window"
		webview [handle!] "webview instance"
		size    [pair!]   "new size"
		hint    [integer!] "NONE=0, MIN=1, MAX=2, FIXED=3"
	]
	set-html: [
		"Load HTML content into the webview"
		webview [handle!] "webview instance"
		html    [string!] "HTML content"
	]
	navigate: [
		"Navigates webview to the given URL"
		webview [handle!] "webview instance"
		url     [url! string!] "URL or properly encoded data URI"
	]
	init: [
		"Injects JavaScript code to be executed immediately upon loading a page"
		webview [handle!] "webview instance"
		code    [string!] "JS code to be executed before window.onload"
	]
	close: [
		"Destroys a webview instance and closes the native window"
		webview [handle!] "webview instance"
	]
]

ext-values: {}

handles: make map! [
	webview-handle: [
		"WebView handle"
		resources      block!  none                "example.."
	]
]

arg-words:   copy []
handles-doc: copy {}

foreach [name spec] handles [
	append handles-doc ajoin [
		LF LF "#### __" uppercase form name "__ - " spec/1 LF
		LF "```rebol"
		LF ";Refinement       Gets                Sets                          Description"
	]
	foreach [name gets sets desc] next spec [
		append handles-doc rejoin [
			LF
			#"/" pad name 17
			pad mold gets 20
			pad mold sets 30
			#"^"" desc #"^""
		]
		append arg-words name
	]
	append handles-doc "^/```"
]
;print handles-doc
arg-words: unique arg-words

type-words: [
	;@@ Order is important!
]


;-------------------------------------- ----------------------------------------
reb-code: ajoin [
	{REBOL [Title: "Rebol WebView Extension"}
	{ Name: webview Type: module}
	{ Version: 0.1.0}
	{ Needs: 3.20.0}
	{ Author: Oldes}
	{ Date: } now/utc
	{ License: MIT}
	{ Url: https://github.com/Oldes/Rebol-WebView}
	#"]"
]

logo: next {
//   ____  __   __        ______        __
//  / __ \/ /__/ /__ ___ /_  __/__ ____/ /
// / /_/ / / _  / -_|_-<_ / / / -_) __/ _ \
// \____/_/\_,_/\__/___(@)_/  \__/\__/_// /
//  ~~~ oldes.huhuman at gmail.com ~~~ /_/
//
// Project: Rebol/WebView extension
// SPDX-License-Identifier: MIT
// =============================================================================
// NOTE: auto-generated file, do not modify!
}

enu-commands:  "" ;; command name enumerations
cmd-declares:  "" ;; command function declarations
cmd-dispatch:  "" ;; command functionm dispatcher

ma-arg-words: "enum ma_arg_words {W_ARG_0"
ma-type-words: "enum ma_type_words {W_TYPE_0"

;- generate C and Rebol code from the command specifications -------------------
foreach [name spec] commands [
	append reb-code ajoin [lf name ": command "]
	new-line/all spec false
	append/only reb-code mold spec

	name: form name
	replace/all name #"-" #"_"
	replace/all name #"?" #"q"
	
	append enu-commands ajoin ["^/^-CMD_MINIAUDIO_" uppercase copy name #","]

	append cmd-declares ajoin ["^/int cmd_" name "(RXIFRM *frm, void *ctx);"]
	append cmd-dispatch ajoin ["^-cmd_" name ",^/"]
]

;- additional Rebol initialization code ----------------------------------------

foreach word arg-words [
	word: uppercase form word
	replace/all word #"-" #"_"
	replace/all word #"?" #"Q"
	append ma-arg-words ajoin [",^/^-W_ARG_" word]
]

foreach word type-words [
	word: uppercase form word
	replace/all word #"-" #"_"
	append ma-type-words ajoin [",^/^-W_TYPE_" word]
]

append ma-arg-words "^/};"
append ma-type-words "^/};"
append reb-code ajoin [{
init-words } mold/flat arg-words mold/flat type-words {
protect/hide 'init-words
} ext-values
]

;append reb-code {}

;print reb-code

;- convert Rebol code to C-string ----------------------------------------------
init-code: copy ""
foreach line split reb-code lf [
	replace/all line #"^"" {\"}
	append init-code ajoin [{\^/^-"} line {\n"}] 
]

;-- C file webviews -----------------------------------------------------------
header: {$logo

#include "rebol-extension.h"
#include "webview.h"
#include <stddef.h>

#ifdef _WIN32
#include <windows.h>
#endif

#define SERIES_TEXT(s)   ((char*)SERIES_DATA(s))

#define MIN_REBOL_VER 3
#define MIN_REBOL_REV 20
#define MIN_REBOL_UPD 0
#define VERSION(a, b, c) (a << 16) + (b << 8) + c
#define MIN_REBOL_VERSION VERSION(MIN_REBOL_VER, MIN_REBOL_REV, MIN_REBOL_UPD)

typedef struct reb_webview_context {
	uintptr_t thread;
	webview_t w;
	int debug;
	void *lock;
	int width;  // initial window width
	int height; // initial window height
	int hint;   // new size hints
} reb_webview_context;

extern REBCNT Handle_WebViewHandle;

extern u32* arg_words;
extern u32* type_words;

enum ext_commands {$enu-commands
};

$cmd-declares

$ma-arg-words
$ma-type-words

typedef int (*MyCommandPointer)(RXIFRM *frm, void *ctx);

#define MINIAUDIO_EXT_INIT_CODE $init-code

#ifdef  USE_TRACES
#include <stdio.h>
#define debug_print(fmt, ...) do { printf(fmt, __VA_ARGS__); } while (0)
#define trace(str) puts(str)
#else
#define debug_print(fmt, ...)
#define trace(str) 
#endif

}
;;------------------------------------------------------------------------------
ctable: {$logo

#include "webview-rebol-extension.h"
MyCommandPointer Command[] = {
$cmd-dispatch};
}

;- output generated files ------------------------------------------------------
write %webview-rebol-extension.h reword :header self
write %webview-commands-table.c  reword :ctable self



;; README documentation...
doc: clear ""
hdr: clear ""
arg: clear ""
cmd: desc: a: t: s: readme: r: none
parse commands [
	any [
		quote init-words: skip
		|
		set cmd: set-word! into [
			(clear hdr clear arg r: none)
			(append hdr ajoin [LF LF "#### `" cmd "`"])
			set desc: opt string!
			any [
				set a word!
				set t opt block!
				set s opt string!
				(
					unless r [append hdr ajoin [" `:" a "`"]]
					append arg ajoin [LF "* `" a "`"] 
					if t [append arg ajoin [" `" mold t "`"]]
					if s [append arg ajoin [" " s]]
				)
				|
				set r refinement!
				set s opt string!
				(
					append arg ajoin [LF "* `/" r "`"] 
					if s [append arg ajoin [" " s]]
				)
			]
			(
				append doc hdr
				append doc LF
				append doc any [desc ""]
				append doc arg
			)
		]
	]
]

try/except [
	readme: read/string %../README.md
	readme: clear find/tail readme "## Extension commands:"
	append readme ajoin [
		LF doc
		LF LF
	;	LF "## Used handles and its getters / setters" 
	;	handles-doc
	;	LF LF
	;	LF "## Other extension values:"
	;	LF "```rebol"
	;	trim/tail ext-values
	;	LF "```"
		LF
	]
	write %../README.md head readme
] :print


