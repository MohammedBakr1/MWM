#ifndef MWM_H
#define MWM_H

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#define MAX_WORKSPACES 9
#define MAX_KEYBINDS   64
#define MAX_RULES      32

typedef enum { LAYOUT_TILE, LAYOUT_FLOAT, LAYOUT_MONOCLE } Layout;

typedef struct Client {
    Window win;
    int x, y, w, h;
    int fx, fy, fw, fh;
    int workspace;
    int floating;
    int fullscreen;
    struct Client *next;
} Client;

typedef struct {
    unsigned int mod;
    KeySym keysym;
    void (*func)(const char *arg);
    char arg[128];
} KeyBind;

typedef struct {
    char class_name[64];
    int workspace;
    int floating;
} Rule;

typedef struct {

    unsigned int mod_key;
    int border_width;
    unsigned long border_focus;
    unsigned long border_unfocus;
    float master_ratio;
    int gap;

    KeyBind keys[MAX_KEYBINDS];
    int nkeys;

    Rule rules[MAX_RULES];
    int nrules;
} Config;

typedef struct {
    Display *dpy;
    Window root;
    int screen;
    int sw, sh;

    Client *clients;
    Client *focused;
    int current_ws;
    Layout layouts[MAX_WORKSPACES];

    Config cfg;
    int running;
} WM;

extern WM wm;

#endif
