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

enum ext_commands {
	CMD_MINIAUDIO_INIT_WORDS,
	CMD_MINIAUDIO_VERSION,
	CMD_MINIAUDIO_CREATE,
	CMD_MINIAUDIO_EVAL,
	CMD_MINIAUDIO_SET_TITLE,
	CMD_MINIAUDIO_SET_SIZE,
	CMD_MINIAUDIO_SET_HTML,
	CMD_MINIAUDIO_NAVIGATE,
	CMD_MINIAUDIO_INIT,
	CMD_MINIAUDIO_CLOSE,
};


int cmd_init_words(RXIFRM *frm, void *ctx);
int cmd_version(RXIFRM *frm, void *ctx);
int cmd_create(RXIFRM *frm, void *ctx);
int cmd_eval(RXIFRM *frm, void *ctx);
int cmd_set_title(RXIFRM *frm, void *ctx);
int cmd_set_size(RXIFRM *frm, void *ctx);
int cmd_set_html(RXIFRM *frm, void *ctx);
int cmd_navigate(RXIFRM *frm, void *ctx);
int cmd_init(RXIFRM *frm, void *ctx);
int cmd_close(RXIFRM *frm, void *ctx);

enum ma_arg_words {W_ARG_0,
	W_ARG_RESOURCES
};
enum ma_type_words {W_TYPE_0
};

typedef int (*MyCommandPointer)(RXIFRM *frm, void *ctx);

#define MINIAUDIO_EXT_INIT_CODE \
	"REBOL [Title: \"Rebol WebView Extension\" Name: webview Type: module Version: 0.1.0 Needs: 3.20.0 Author: Oldes Date: 7-Jan-2026/12:57:14 License: MIT Url: https://github.com/Oldes/Rebol-WebView]\n"\
	"init-words: command [args [block!] type [block!]]\n"\
	"version: command [\"Get the library's version information\"]\n"\
	"create: command [\"Create a new WebView handle\" /debug \"Enable developer tools if supported by the backend\"]\n"\
	"eval: command [\"Evaluates arbitrary JavaScript code\" webview [handle!] \"webview instance\" code [string!] \"JS code to evaluate\"]\n"\
	"set-title: command [\"Updates the title of the native window\" webview [handle!] \"webview instance\" title [string!] \"new title\"]\n"\
	"set-size: command [\"Updates the size of the native window\" webview [handle!] \"webview instance\" size [pair!] \"new size\" hint [integer!] \"NONE=0, MIN=1, MAX=2, FIXED=3\"]\n"\
	"set-html: command [\"Load HTML content into the webview\" webview [handle!] \"webview instance\" html [string!] \"HTML content\"]\n"\
	"navigate: command [\"Navigates webview to the given URL\" webview [handle!] \"webview instance\" url [url! string!] \"URL or properly encoded data URI\"]\n"\
	"init: command [{Injects JavaScript code to be executed immediately upon loading a page} webview [handle!] \"webview instance\" code [string!] \"JS code to be executed before window.onload\"]\n"\
	"close: command [{Destroys a webview instance and closes the native window} webview [handle!] \"webview instance\"]\n"\
	"init-words [resources][]\n"\
	"protect/hide 'init-words\n"\
	"\n"

#ifdef  USE_TRACES
#include <stdio.h>
#define debug_print(fmt, ...) do { printf(fmt, __VA_ARGS__); } while (0)
#define trace(str) puts(str)
#else
#define debug_print(fmt, ...)
#define trace(str) 
#endif

