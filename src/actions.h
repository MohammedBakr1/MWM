#ifndef ACTIONS_H
#define ACTIONS_H

typedef struct {
    const char *name;
    void (*func)(const char *arg);
} Action;

void action_spawn(const char *arg);
void action_kill(const char *arg);
void action_focus_next(const char *arg);
void action_focus_prev(const char *arg);
void action_toggle_float(const char *arg);
void action_toggle_fullscreen(const char *arg);
void action_set_layout(const char *arg);
void action_goto_workspace(const char *arg);
void action_move_to_workspace(const char *arg);
void action_resize_master(const char *arg);
void action_quit(const char *arg);
void action_reload_config(const char *arg);

extern Action action_table[];

#endif
