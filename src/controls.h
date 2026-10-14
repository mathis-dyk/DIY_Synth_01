#pragma once
#include "state.h"

void controls_setup();
void controls_update();

// menu réglages (pour l'affichage)
const char* menu_name();
void        menu_value(char* buf, int len);
int         menu_count();
