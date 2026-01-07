# Rebol/WebView

WebView extension for [Rebol3](https://github.com/Oldes/Rebol3) (version 3.20.0+). Embeds native WebView controls (Windows WebView2, macOS WKWebView, Linux WebKitGTK) into Rebol, enabling HTML/JS/CSS rendering and bi-directional interaction in separate threads. Supports window creation, navigation, HTML/JS evaluation, title setting, and handle management.

Currently a work in progress on Linux/macOS support.


## Usage
```rebol
webview: import webview

;; Create two WebView windows (each running in its own thread)
w1: webview/create
w2: webview/create

;; Set window titles
webview/set-title w1 "Rebol Function Dictionary"
webview/set-title w2 "Hello from Rebol!"

;; Navigate to a web page in the first window
webview/navigate w1 https://rebol.tech/docs/functions.html

;; Set HTML content in the second window
webview/set-html w2 {<font color="red">Hello from Rebol!</font>}

;; Set size of the window
webview/set-size w2 600x200 3 ;; 3 = fixed size

;; Work in Rebol as needed...
wait 1

;; Evaluate JavaScript code
webview/eval w2 "alert('Hello from JavaScript!');"
wait 10

;; Close the first window
webview/close w1

;; Or close by releasing the window handle
release w2
```

## Extension commands:


#### `version`
Get the library's version information

#### `create`
Create a new WebView handle
* `/debug` Enable developer tools if supported by the backend

#### `eval` `:webview` `:code`
Evaluates arbitrary JavaScript code
* `webview` `[handle!]` webview instance
* `code` `[string!]` JS code to evaluate

#### `set-title` `:webview` `:title`
Updates the title of the native window
* `webview` `[handle!]` webview instance
* `title` `[string!]` new title

#### `set-size` `:webview` `:size` `:hint`
Updates the size of the native window
* `webview` `[handle!]` webview instance
* `size` `[pair!]` new size
* `hint` `[integer!]` NONE=0, MIN=1, MAX=2, FIXED=3

#### `set-html` `:webview` `:html`
Load HTML content into the webview
* `webview` `[handle!]` webview instance
* `html` `[string!]` HTML content

#### `navigate` `:webview` `:url`
Navigates webview to the given URL
* `webview` `[handle!]` webview instance
* `url` `[url! string!]` URL or properly encoded data URI

#### `init` `:webview` `:code`
Injects JavaScript code to be executed immediately upon loading a page
* `webview` `[handle!]` webview instance
* `code` `[string!]` JS code to be executed before window.onload

#### `close` `:webview`
Destroys a webview instance and closes the native window
* `webview` `[handle!]` webview instance


