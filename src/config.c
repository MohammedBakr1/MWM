#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <X11/keysym.h>
#include "config.h"
#include "actions.h"

static void set_defaults(Config *cfg) {
    memset(cfg, 0, sizeof(Config));
    cfg->mod_key = Mod4Mask;
    cfg->border_width = 2;
    cfg->border_focus = 0x5e81ac;
    cfg->border_unfocus = 0x3b4252;
    cfg->master_ratio = 0.55f;
    cfg->gap = 8;
    cfg->nkeys = 0;
    cfg->nrules = 0;
}

static char *trim(char *s) {
    while (isspace((unsigned char)*s)) s++;
    if (*s == 0) return s;
    char *end = s + strlen(s) - 1;
    while (end > s && isspace((unsigned char)*end)) *end-- = '\0';
    return s;
}

static unsigned long parse_color(const char *s) {
    if (s[0] == '#') s++;
    return strtoul(s, NULL, 16);
}

static KeySym parse_keysym(const char *s) {
    return XStringToKeysym(s);
}

static void (*resolve_action(const char *name))(const char *) {
    for (int i = 0; action_table[i].name; i++)
        if (strcmp(action_table[i].name, name) == 0)
            return action_table[i].func;
    return NULL;
}

static void parse_bind(Config *cfg, char *value) {
    if (cfg->nkeys >= MAX_KEYBINDS) return;

    char *combo = strtok(value, ",");
    char *action = strtok(NULL, ",");
    char *arg = strtok(NULL, "");

    if (!combo || !action) return;
    combo = trim(combo);
    action = trim(action);

    unsigned int mods = 0;
    KeySym key = NoSymbol;

    char buf[128];
    strncpy(buf, combo, sizeof(buf) - 1);
    buf[sizeof(buf)-1] = '\0';

    char *tok = strtok(buf, "+");
    while (tok) {
        char *t = trim(tok);
        if (strcasecmp(t, "mod") == 0) mods |= cfg->mod_key;
        else if (strcasecmp(t, "shift") == 0) mods |= ShiftMask;
        else if (strcasecmp(t, "ctrl") == 0) mods |= ControlMask;
        else if (strcasecmp(t, "alt") == 0) mods |= Mod1Mask;
        else key = parse_keysym(t);
        tok = strtok(NULL, "+");
    }

    void (*func)(const char *) = resolve_action(action);
    if (!func || key == NoSymbol) return;

    KeyBind *kb = &cfg->keys[cfg->nkeys++];
    kb->mod = mods;
    kb->keysym = key;
    kb->func = func;
    if (arg) {
        strncpy(kb->arg, trim(arg), sizeof(kb->arg) - 1);
        kb->arg[sizeof(kb->arg)-1] = '\0';
    } else {
        kb->arg[0] = '\0';
    }
}

static void parse_rule(Config *cfg, char *value) {
    if (cfg->nrules >= MAX_RULES) return;

    char *class_name = strtok(value, ",");
    if (!class_name) return;
    class_name = trim(class_name);

    Rule *r = &cfg->rules[cfg->nrules++];
    strncpy(r->class_name, class_name, sizeof(r->class_name) - 1);
    r->class_name[sizeof(r->class_name)-1] = '\0';
    r->workspace = -1;
    r->floating = -1;

    char *tok;
    while ((tok = strtok(NULL, ","))) {
        tok = trim(tok);
        char key[32], val[32];
        if (sscanf(tok, "%31[^=]=%31s", key, val) == 2) {
            char *k = trim(key);
            if (strcmp(k, "workspace") == 0) r->workspace = atoi(val);
            else if (strcmp(k, "floating") == 0) r->floating = atoi(val);
        }
    }
}

static void parse_line(Config *cfg, char *line) {
    line = trim(line);
    if (line[0] == '\0' || line[0] == '#') return;

    char *eq = strchr(line, '=');
    if (!eq) return;
    *eq = '\0';
    char *key = trim(line);
    char *value = trim(eq + 1);

    if (strcmp(key, "mod") == 0) {
        if (strcasecmp(value, "super") == 0 || strcasecmp(value, "mod4") == 0)
            cfg->mod_key = Mod4Mask;
        else if (strcasecmp(value, "alt") == 0 || strcasecmp(value, "mod1") == 0)
            cfg->mod_key = Mod1Mask;
    } else if (strcmp(key, "border_width") == 0) {
        cfg->border_width = atoi(value);
    } else if (strcmp(key, "border_focus") == 0) {
        cfg->border_focus = parse_color(value);
    } else if (strcmp(key, "border_unfocus") == 0) {
        cfg->border_unfocus = parse_color(value);
    } else if (strcmp(key, "master_ratio") == 0) {
        cfg->master_ratio = atof(value);
    } else if (strcmp(key, "gap") == 0) {
        cfg->gap = atoi(value);
    } else if (strcmp(key, "bind") == 0) {
        parse_bind(cfg, value);
    } else if (strcmp(key, "rule") == 0) {
        parse_rule(cfg, value);
    }
}

static const char *config_path(char *buf, size_t len) {
    const char *home = getenv("HOME");
    if (!home) return NULL;
    snprintf(buf, len, "%s/.config/mwm/mwmrc", home);
    return buf;
}

void config_load(Config *cfg) {
    set_defaults(cfg);

    char path[512];
    if (!config_path(path, sizeof(path))) return;

    FILE *f = fopen(path, "r");
    if (!f) {
        fprintf(stderr, "mwm: no config at %s, using defaults\n", path);
        return;
    }

    char line[512];
    while (fgets(line, sizeof(line), f))
        parse_line(cfg, line);

    fclose(f);
}

void config_reload(void) {

    extern WM wm;
    config_load(&wm.cfg);
}
