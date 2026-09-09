#include <X11/Xlib.h>
#include "layout.h"

static void apply_tile(int ws) {
    int gap = wm.cfg.gap;
    Client *tiled[256];
    int n = 0;
    for (Client *c = wm.clients; c; c = c->next)
        if (c->workspace == ws && !c->floating && !c->fullscreen)
            tiled[n++] = c;

    if (n == 0) return;

    if (n == 1) {
        Client *c = tiled[0];
        c->x = gap; c->y = gap;
        c->w = wm.sw - 2 * gap - 2 * wm.cfg.border_width;
        c->h = wm.sh - 2 * gap - 2 * wm.cfg.border_width;
        XMoveResizeWindow(wm.dpy, c->win, c->x, c->y, c->w, c->h);
        return;
    }

    int master_w = (int)((wm.sw - 3 * gap) * wm.cfg.master_ratio);
    int stack_w = wm.sw - master_w - 3 * gap;

    Client *m = tiled[0];
    m->x = gap; m->y = gap;
    m->w = master_w - 2 * wm.cfg.border_width;
    m->h = wm.sh - 2 * gap - 2 * wm.cfg.border_width;
    XMoveResizeWindow(wm.dpy, m->win, m->x, m->y, m->w, m->h);

    int stack_n = n - 1;
    int stack_h = (wm.sh - gap - stack_n * gap) / stack_n;
    int sy = gap;
    for (int i = 1; i < n; i++) {
        Client *c = tiled[i];
        c->x = master_w + 2 * gap;
        c->y = sy;
        c->w = stack_w - 2 * wm.cfg.border_width;
        c->h = stack_h - 2 * wm.cfg.border_width;
        XMoveResizeWindow(wm.dpy, c->win, c->x, c->y, c->w, c->h);
        sy += stack_h + gap;
    }
}

static void apply_monocle(int ws) {
    int gap = wm.cfg.gap;
    for (Client *c = wm.clients; c; c = c->next) {
        if (c->workspace != ws || c->floating || c->fullscreen) continue;
        c->x = gap; c->y = gap;
        c->w = wm.sw - 2 * gap - 2 * wm.cfg.border_width;
        c->h = wm.sh - 2 * gap - 2 * wm.cfg.border_width;
        XMoveResizeWindow(wm.dpy, c->win, c->x, c->y, c->w, c->h);
        XRaiseWindow(wm.dpy, c->win);
    }
}

void layout_apply(int ws) {

    for (Client *c = wm.clients; c; c = c->next) {
        if (c->workspace == ws && c->fullscreen) {
            XMoveResizeWindow(wm.dpy, c->win, 0, 0, wm.sw, wm.sh);
        }
    }

    switch (wm.layouts[ws]) {
        case LAYOUT_TILE:    apply_tile(ws); break;
        case LAYOUT_MONOCLE: apply_monocle(ws); break;
        case LAYOUT_FLOAT:    break;
    }
}
