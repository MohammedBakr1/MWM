#define _XOPEN_SOURCE 700
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <X11/Xlib.h>
#include "client.h"

Client *client_create(Window w) {
    Client *c = calloc(1, sizeof(Client));
    c->win = w;
    c->workspace = wm.current_ws;
    c->floating = 0;
    c->fullscreen = 0;
    c->next = wm.clients;
    wm.clients = c;
    return c;
}

void client_destroy(Client *c) {
    if (!c) return;
    if (wm.clients == c) {
        wm.clients = c->next;
    } else {
        Client *p = wm.clients;
        while (p && p->next != c) p = p->next;
        if (p) p->next = c->next;
    }
    if (wm.focused == c) wm.focused = NULL;
    free(c);
}

Client *client_find(Window w) {
    for (Client *c = wm.clients; c; c = c->next)
        if (c->win == w) return c;
    return NULL;
}

Client *client_next_on_ws(Client *from, int ws) {
    Client *start = from ? from->next : wm.clients;
    for (Client *c = start; c; c = c->next)
        if (c->workspace == ws) return c;

    for (Client *c = wm.clients; c && c != from; c = c->next)
        if (c->workspace == ws) return c;
    return NULL;
}

int client_count_ws(int ws) {
    int n = 0;
    for (Client *c = wm.clients; c; c = c->next)
        if (c->workspace == ws) n++;
    return n;
}

void client_focus(Client *c) {
    if (!c) {
        XSetInputFocus(wm.dpy, wm.root, RevertToPointerRoot, CurrentTime);
        wm.focused = NULL;
        return;
    }
    for (Client *o = wm.clients; o; o = o->next) {
        if (o == c)
            XSetWindowBorder(wm.dpy, o->win, wm.cfg.border_focus);
        else if (o->workspace == c->workspace)
            XSetWindowBorder(wm.dpy, o->win, wm.cfg.border_unfocus);
    }
    XSetInputFocus(wm.dpy, c->win, RevertToPointerRoot, CurrentTime);
    XRaiseWindow(wm.dpy, c->win);
    wm.focused = c;
}

void client_apply_rules(Client *c, const char *class_name) {
    for (int i = 0; i < wm.cfg.nrules; i++) {
        Rule *r = &wm.cfg.rules[i];
        if (strcasecmp(r->class_name, class_name) == 0) {
            if (r->workspace >= 0) c->workspace = r->workspace;
            if (r->floating >= 0) c->floating = r->floating;
            return;
        }
    }
}
