//   ____  __   __        ______        __
//  / __ \/ /__/ /__ ___ /_  __/__ ____/ /
// / /_/ / / _  / -_|_-<_ / / / -_) __/ _ \
// \____/_/\_,_/\__/___(@)_/  \__/\__/_// /
//  ~~~ oldes.huhuman at gmail.com ~~~ /_/
//
// SPDX-License-Identifier: MIT
// =============================================================================
// Rebol/WebView extension
// =============================================================================


#include "webview-rebol-extension.h"
#include <stdio.h>
#include <process.h>  // For _beginthread/_endthread

#define COMMAND int

#define FRM_IS_HANDLE(n, t)   (RXA_TYPE(frm,n) == RXT_HANDLE && RXA_HANDLE_TYPE(frm, n) == t)
#define ARG_Is_WebView(n)     FRM_IS_HANDLE(n, Handle_WebViewHandle)
#define ARG_WebView(n)        (ARG_Is_WebView(n) ? (reb_webview_context*)(RXA_HANDLE_CONTEXT(frm, n)->handle) : NULL)
#define ARG_String(n)         (char*)(((REBSER*)(RXA_ARG(frm, n).series))->data) + RXA_INDEX(frm, n)
#define ARG_Double(n)         RXA_DEC64(frm,n)
#define ARG_Float(n)          (float)RXA_DEC64(frm,n)
#define ARG_Int32(n)          RXA_INT32(frm,n)
#define ARG_Handle_Series(n)  RXA_HANDLE_CONTEXT(frm, n)->series;

#define RETURN_HANDLE(hob)                   \
    RXA_HANDLE(frm, 1)       = hob;          \
    RXA_HANDLE_TYPE(frm, 1)  = hob->sym;     \
    RXA_HANDLE_FLAGS(frm, 1) = hob->flags;   \
    RXA_TYPE(frm, 1) = RXT_HANDLE;           \
    return RXR_VALUE

#define APPEND_STRING(str, ...) \
    len = snprintf(NULL,0,__VA_ARGS__);\
    if (len > SERIES_REST(str)-SERIES_LEN(str)) {\
        RL_EXPAND_SERIES(str, SERIES_TAIL(str), len);\
        SERIES_TAIL(str) -= len;\
    }\
    len = snprintf( \
        SERIES_TEXT(str)+SERIES_TAIL(str),\
        SERIES_REST(str)-SERIES_TAIL(str),\
        __VA_ARGS__\
    );\
    SERIES_TAIL(str) += len;

#define RETURN_ERROR(err)  do {RXA_SERIES(frm, 1) = err; return RXR_ERROR;} while(0)


REBLEN Common_mold(REBHOB *hob, REBSER *str) {
    REBLEN len;
    if (!str) return 0;
    SERIES_TAIL(str) = 0;
    APPEND_STRING(str, "0#%lx", (unsigned long)(uintptr_t)hob->handle);
    return len;
}



// Dispatch wrappers ////////////////////////////////////////
static void eval_cb(webview_t wv, void *arg) {
    webview_eval(wv, (char*)arg);
}
static void safe_eval_cb(webview_t w, void *arg) {
    webview_eval(w, (char*)arg);
    free((char*)arg);
}
static void set_title_cb(webview_t wv, void *arg) {
    webview_set_title(wv, (char*)arg);
}
static void set_size_cb(webview_t wv, void *arg) {
    reb_webview_context *wc = (reb_webview_context*)arg;
    webview_set_size(wv, wc->width, wc->height, wc->hint);
}
static void set_html_cb(webview_t wv, void *arg) {
    webview_set_html(wv, (char*)arg);
}
static void navigate_cb(webview_t w, void *arg) {
    webview_navigate(w, (char*)arg);
    free((char*)arg);
}
static void init_cb(webview_t w, void *arg) {
    webview_init(w, (char*)arg);
    free((char*)arg);
}
static void terminate_cb(webview_t w, void *arg) {
    webview_terminate(w);
}



#ifdef _WIN32
static void set_icon_cb(webview_t w, void *arg) {
    HWND hwnd = (HWND)webview_get_window(w);
    if (!hwnd) return;

    HMODULE hExe = GetModuleHandle(NULL);  // Main app EXE
    HICON hIcon = LoadIcon(hExe, MAKEINTRESOURCE(101)); //icon ID
    if (hIcon) {
        SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
        SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
    }
}
#endif


static void hello(const char *id, const char *req, void *arg) {
    trace("hello....");
    //context_t *context = (context_t *)arg;
    //// Imagine that params->req is properly parsed or use your own JSON parser.
    //long direction = strtol(req + 1, NULL, 10);
    //char result[10] = {0};
    //(void)sprintf(result, "%ld", context->count += direction);
    //webview_return(context->w, id, 0, result);
}



int WebViewHandle_free(void* hndl) {
    REBHOB *hob;

    if (!hndl) return 0;
    hob = (REBHOB *)hndl;

    printf("release webview: %p is referenced: %i\n", hob->data, IS_MARK_HOB(hob) != 0);
    UNMARK_HOB(hob);
    reb_webview_context *ctx = (reb_webview_context*)hob->data;
    if (ctx->w) {
        webview_dispatch(ctx->w, terminate_cb, NULL);  // Dispatch terminate
#ifdef _WIN32
        DWORD wait_res = WaitForSingleObject((HANDLE)ctx->thread, 5000);  // 5s timeout
        CloseHandle((HANDLE)ctx->thread);
#else
    //TODO: Posix!
#endif
        ctx->thread = 0;
    }

    return 0;
}

int WebViewHandle_get_path(REBHOB *hob, REBCNT word, REBCNT *type, RXIARG *arg) {
//  ma_engine* engine = (ma_engine*)hob->data;
    word = RL_FIND_WORD(arg_words, word);
    switch (word) {
    case W_ARG_RESOURCES:
        *type = RXT_BLOCK;
        arg->series = hob->series;
        arg->index = 0;
        break;
    default:
        return PE_BAD_SELECT;   
    }
    return PE_USE;
}
int WebViewHandle_set_path(REBHOB *hob, REBCNT word, REBCNT *type, RXIARG *arg) {
//  ma_engine* engine = (ma_engine*)hob->data;
    word = RL_FIND_WORD(arg_words, word);
    switch (word) {
//  case W_ARG_VOLUME:
//      switch (*type) {
//      case RXT_DECIMAL:
//      case RXT_PERCENT:
//          ma_engine_set_volume(engine, arg->dec64);
//          break;
//      case RXT_INTEGER:
//          ma_engine_set_volume(engine, (float)arg->int64);
//          break;
//      default: 
//          return PE_BAD_SET_TYPE;
//      }
//      break;
    default:
        return PE_BAD_SET;  
    }
    return PE_OK;
}



void Run_WebView_Thread(void* arg) {
    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

    reb_webview_context *ctx = (reb_webview_context*)arg;

    ctx->w = webview_create(ctx->debug, NULL);

    webview_bind(ctx->w, "hello", hello, &ctx);

    SetEvent(ctx->lock);

    webview_run(ctx->w);
    trace("run ended");
    debug_print("ctx? %p %p\n", ctx, ctx->w);
    webview_destroy(ctx->w);
    ctx->w = NULL;
}



COMMAND cmd_init_words(RXIFRM *frm, void *ctx) {
    arg_words  = RL_MAP_WORDS(RXA_SERIES(frm,1));
    type_words = RL_MAP_WORDS(RXA_SERIES(frm,2));

    // custom initialization may be done here...

    return RXR_TRUE;
}

COMMAND cmd_version(RXIFRM *frm, void *ctx) {
    const webview_version_info_t *info = webview_version();

    RXA_TYPE(frm, 1) = RXT_TUPLE;
    RXA_TUPLE(frm, 1)[0] = (info->version.major) & 0xFF;
    RXA_TUPLE(frm, 1)[1] = (info->version.minor) & 0xFF;
    RXA_TUPLE(frm, 1)[2] =  info->version.patch  & 0xFF;
    RXA_TUPLE_LEN(frm, 1) = 3;

    return RXR_VALUE;
}

COMMAND cmd_create(RXIFRM *frm, void *ctx) {
    
    REBHOB *hob = RL_MAKE_HANDLE_CONTEXT(Handle_WebViewHandle);
    if (!hob) return RXR_FALSE;
    reb_webview_context *wc = (reb_webview_context*)hob->data;

    wc->debug = RXA_REF(frm, 1);

#ifdef _WIN32
    wc->lock = CreateEvent(
        NULL,                       // default security attributes
        TRUE,                       // manual-reset event
        FALSE,                      // initial state is nonsignaled
        TEXT("WebView_Task_Launch") // object name
    );
    if (!wc->lock) return RXR_FALSE;
    wc->thread = _beginthread(Run_WebView_Thread, 0, hob->data);
    if (wc->thread == 0) {
         CloseHandle(wc->lock);
         return RXR_FALSE;
    }
    WaitForSingleObject(wc->lock, 5000);
    CloseHandle(wc->lock);
    wc->lock = NULL;
    // As the WebView is running in its own thread, we must set
    // the icon (used from application resources)
    webview_dispatch(wc->w, set_icon_cb, NULL);
#else
    //TODO: Posix!
#endif

    RXA_HANDLE(frm, 1)       = hob;
    RXA_HANDLE_TYPE(frm, 1)  = hob->sym;
    RXA_HANDLE_FLAGS(frm, 1) = hob->flags;
    RXA_TYPE(frm, 1) = RXT_HANDLE;

    return RXR_VALUE;
}



COMMAND cmd_eval(RXIFRM *frm, void *ctx) {
    reb_webview_context *wc = ARG_WebView(1);
    char *str = ARG_String(2);
    if (!wc || !wc->w) return RXR_FALSE;

    webview_dispatch(wc->w, safe_eval_cb, _strdup(str));

    return RXR_UNSET;
}

COMMAND cmd_set_title(RXIFRM *frm, void *ctx) {
    reb_webview_context *wc = ARG_WebView(1);
    char *str = ARG_String(2);
    if (!wc || !wc->w) return RXR_FALSE;

    webview_dispatch(wc->w, set_title_cb, str);

    return RXR_UNSET;
}

COMMAND cmd_set_size(RXIFRM *frm, void *ctx) {
    reb_webview_context *wc = ARG_WebView(1);
    char *str = ARG_String(2);
    if (!wc || !wc->w) return RXR_FALSE;

    wc->width  = (int)RXA_PAIR(frm, 2).x;
    wc->height = (int)RXA_PAIR(frm, 2).y;
    wc->hint   = RXA_INT32(frm,3);

    webview_dispatch(wc->w, set_size_cb, wc);

    return RXR_UNSET;
}

COMMAND cmd_set_html(RXIFRM *frm, void *ctx) {
    reb_webview_context *wc = ARG_WebView(1);
    char *str = ARG_String(2);
    if (!wc || !wc->w) return RXR_FALSE;

    webview_dispatch(wc->w, set_html_cb, str);

    return RXR_UNSET;
}

COMMAND cmd_navigate(RXIFRM *frm, void *ctx) {
    reb_webview_context *wc = ARG_WebView(1);
    char *str = ARG_String(2);
    if (!wc || !wc->w) return RXR_FALSE;

    webview_dispatch(wc->w, navigate_cb, _strdup(str));

    return RXR_UNSET;
}

COMMAND cmd_init(RXIFRM *frm, void *ctx) {
    reb_webview_context *wc = ARG_WebView(1);
    char *str = ARG_String(2);
    if (!wc || !wc->w) return RXR_FALSE;

    webview_dispatch(wc->w, init_cb, _strdup(str));

    return RXR_UNSET;
}

COMMAND cmd_close(RXIFRM *frm, void *ctx) {
    reb_webview_context *wc = ARG_WebView(1);
    if (!wc || !wc->w) return RXR_FALSE;

    webview_dispatch(wc->w, terminate_cb, NULL);  // Dispatch terminate

    DWORD wait_res = WaitForSingleObject((HANDLE)wc->thread, 5000);  // 5s timeout
    CloseHandle((HANDLE)wc->thread);
    wc->thread = 0;

    return RXR_TRUE;
}