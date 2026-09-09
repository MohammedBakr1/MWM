#ifndef CLIENT_H
#define CLIENT_H

#include "mwm.h"

Client *client_create(Window w);
void client_destroy(Client *c);
Client *client_find(Window w);
Client *client_next_on_ws(Client *from, int ws);
void client_focus(Client *c);
void client_apply_rules(Client *c, const char *class_name);
int client_count_ws(int ws);

#endif
