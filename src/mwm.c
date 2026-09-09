#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/XKBlib.h>

#include "mwm.h"
#include "config.h"
#include "client.h"
#include "layout.h"

WM wm;

static int xerror(Display *dpy, XErrorEvent *ee) {
    char msg[256];
    XGetErrorText(dpy, ee->error_code, msg, sizeof(msg));
    fprintf(stderr, "mwm: X error: %s (request %d)\n", msg, ee->request_code);
    return 0;
}

static void grab_keys(void) {
    XUngrabKey(wm.dpy, AnyKey, AnyModifier, wm.root);
    for (int i = 0; i < wm.cfg.nkeys; i++) {
        KeyBind *kb = &wm.cfg.keys[i];
        KeyCode code = XKeysymToKeycode(wm.dpy, kb->keysym);
        if (!code) continue;
        XGrabKey(wm.dpy, code, kb->mod, wm.root, True, GrabModeAsync, GrabModeAsync);

        XGrabKey(wm.dpy, code, kb->mod | LockMask, wm.root, True, GrabModeAsync, GrabModeAsync);
        XGrabKey(wm.dpy, code, kb->mod | Mod2Mask, wm.root, True, GrabModeAsync, GrabModeAsync);
        XGrabKey(wm.dpy, code, kb->mod | LockMask | Mod2Mask, wm.root, True, GrabModeAsync, GrabModeAsync);
    }
}

static void manage_window(Window w) {
    XWindowAttributes wa;
    if (!XGetWindowAttributes(wm.dpy, w, &wa)) return;
    if (wa.override_redirect) return;
    if (client_find(w)) return;

    Client *c = client_create(w);

    XClassHint ch = {0};
    if (XGetClassHint(wm.dpy, w, &ch)) {
        if (ch.res_class) client_apply_rules(c, ch.res_class);
        if (ch.res_class) XFree(ch.res_class);
        if (ch.res_name) XFree(ch.res_name);
    }

    XSelectInput(wm.dpy, w, EnterWindowMask | FocusChangeMask | PropertyChangeMask | StructureNotifyMask);
    XSetWindowBorderWidth(wm.dpy, w, wm.cfg.border_width);
    XSetWindowBorder(wm.dpy, w, wm.cfg.border_unfocus);

    if (c->workspace == wm.current_ws) {
        XMapWindow(wm.dpy, w);
        layout_apply(wm.current_ws);
        client_focus(c);
    }
}

static void unmanage_window(Window w) {
    Client *c = client_find(w);
    if (!c) return;
    int ws = c->workspace;
    client_destroy(c);
    layout_apply(ws);
    if (wm.focused == NULL)
        client_focus(client_next_on_ws(NULL, wm.current_ws));
}

static void on_keypress(XEvent *e) {
    XKeyEvent *ke = &e->xkey;
    KeySym keysym = XkbKeycodeToKeysym(wm.dpy, ke->keycode, 0, 0);
    unsigned int clean_state = ke->state & ~(LockMask | Mod2Mask);

    for (int i = 0; i < wm.cfg.nkeys; i++) {
        KeyBind *kb = &wm.cfg.keys[i];
        if (kb->keysym == keysym && kb->mod == clean_state) {
            kb->func(kb->arg);
            return;
        }
    }
}

static void on_map_request(XEvent *e) {
    manage_window(e->xmaprequest.window);
}

static void on_destroy_notify(XEvent *e) {
    unmanage_window(e->xdestroywindow.window);
}

static void on_unmap_notify(XEvent *e) {
    unmanage_window(e->xunmap.window);
}

static void on_enter_notify(XEvent *e) {

    Client *c = client_find(e->xcrossing.window);
    if (c && c->workspace == wm.current_ws) client_focus(c);
}

static void run(void) {
    XEvent e;
    wm.running = 1;
    while (wm.running && !XNextEvent(wm.dpy, &e)) {
        switch (e.type) {
            case KeyPress:       on_keypress(&e); break;
            case MapRequest:     on_map_request(&e); break;
            case DestroyNotify:  on_destroy_notify(&e); break;
            case UnmapNotify:    on_unmap_notify(&e); break;
            case EnterNotify:    on_enter_notify(&e); break;
            default: break;
        }
    }
}

int main(void) {
    wm.dpy = XOpenDisplay(NULL);
    if (!wm.dpy) {
        fprintf(stderr, "mwm: cannot open display\n");
        return 1;
    }

    XSetErrorHandler(xerror);

    wm.screen = DefaultScreen(wm.dpy);
    wm.root = RootWindow(wm.dpy, wm.screen);
    wm.sw = DisplayWidth(wm.dpy, wm.screen);
    wm.sh = DisplayHeight(wm.dpy, wm.screen);
    wm.clients = NULL;
    wm.focused = NULL;
    wm.current_ws = 0;
    for (int i = 0; i < MAX_WORKSPACES; i++) wm.layouts[i] = LAYOUT_TILE;

    config_load(&wm.cfg);
    grab_keys();

    XSelectInput(wm.dpy, wm.root, SubstructureRedirectMask | SubstructureNotifyMask);

    run();

    XCloseDisplay(wm.dpy);
    return 0;
}
