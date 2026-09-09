#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <X11/Xlib.h>
#include "mwm.h"
#include "actions.h"
#include "client.h"
#include "layout.h"
#include "config.h"

void action_spawn(const char *arg) {
    if (fork() == 0) {
        if (wm.dpy) close(ConnectionNumber(wm.dpy));
        setsid();
        execl("/bin/sh", "sh", "-c", arg, (char *)NULL);
        _exit(1);
    }
}

void action_kill(const char *arg) {
    (void)arg;
    if (!wm.focused) return;
    XEvent ev = {0};
    Atom wm_protocols = XInternAtom(wm.dpy, "WM_PROTOCOLS", False);
    Atom wm_delete = XInternAtom(wm.dpy, "WM_DELETE_WINDOW", False);
    ev.xclient.type = ClientMessage;
    ev.xclient.window = wm.focused->win;
    ev.xclient.message_type = wm_protocols;
    ev.xclient.format = 32;
    ev.xclient.data.l[0] = wm_delete;
    ev.xclient.data.l[1] = CurrentTime;
    XSendEvent(wm.dpy, wm.focused->win, False, NoEventMask, &ev);
}

void action_focus_next(const char *arg) {
    (void)arg;
    Client *n = client_next_on_ws(wm.focused, wm.current_ws);
    if (n) client_focus(n);
}

void action_focus_prev(const char *arg) {
    (void)arg;

    if (!wm.focused) { action_focus_next(arg); return; }
    Client *prev = NULL, *c;
    for (c = wm.clients; c; c = c->next)
        if (c->workspace == wm.current_ws && c != wm.focused) prev = c;
    if (prev) client_focus(prev);
}

void action_toggle_float(const char *arg) {
    (void)arg;
    if (!wm.focused) return;
    wm.focused->floating = !wm.focused->floating;
    layout_apply(wm.current_ws);
}

void action_toggle_fullscreen(const char *arg) {
    (void)arg;
    if (!wm.focused) return;
    wm.focused->fullscreen = !wm.focused->fullscreen;
    layout_apply(wm.current_ws);
}

void action_set_layout(const char *arg) {
    if (strcmp(arg, "tile") == 0) wm.layouts[wm.current_ws] = LAYOUT_TILE;
    else if (strcmp(arg, "float") == 0) wm.layouts[wm.current_ws] = LAYOUT_FLOAT;
    else if (strcmp(arg, "monocle") == 0) wm.layouts[wm.current_ws] = LAYOUT_MONOCLE;
    layout_apply(wm.current_ws);
}

void action_goto_workspace(const char *arg) {
    int ws = atoi(arg) - 1;
    if (ws < 0 || ws >= MAX_WORKSPACES) return;
    for (Client *c = wm.clients; c; c = c->next) {
        if (c->workspace == wm.current_ws) XUnmapWindow(wm.dpy, c->win);
        if (c->workspace == ws) XMapWindow(wm.dpy, c->win);
    }
    wm.current_ws = ws;
    layout_apply(ws);
    client_focus(client_next_on_ws(NULL, ws));
}

void action_move_to_workspace(const char *arg) {
    int ws = atoi(arg) - 1;
    if (ws < 0 || ws >= MAX_WORKSPACES || !wm.focused) return;
    wm.focused->workspace = ws;
    XUnmapWindow(wm.dpy, wm.focused->win);
    layout_apply(wm.current_ws);
    wm.focused = NULL;
}

void action_resize_master(const char *arg) {
    float delta = atof(arg);
    wm.cfg.master_ratio += delta;
    if (wm.cfg.master_ratio < 0.1f) wm.cfg.master_ratio = 0.1f;
    if (wm.cfg.master_ratio > 0.9f) wm.cfg.master_ratio = 0.9f;
    layout_apply(wm.current_ws);
}

void action_quit(const char *arg) {
    (void)arg;
    wm.running = 0;
}

void action_reload_config(const char *arg) {
    (void)arg;
    config_reload();
}

Action action_table[] = {
    {"spawn",             action_spawn},
    {"kill",               action_kill},
    {"focus_next",         action_focus_next},
    {"focus_prev",         action_focus_prev},
    {"toggle_float",       action_toggle_float},
    {"toggle_fullscreen",  action_toggle_fullscreen},
    {"set_layout",         action_set_layout},
    {"goto_workspace",     action_goto_workspace},
    {"move_to_workspace",  action_move_to_workspace},
    {"resize_master",      action_resize_master},
    {"quit",               action_quit},
    {"reload_config",      action_reload_config},
    {NULL, NULL}
};
