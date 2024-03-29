#pragma once

enum Urg { LOW, NORMAL, CRITICAL };

bool init_dbus();
void deinit_dbus();
void dbus_send(const char*, const char*, const enum Urg, const unsigned);
