#include <dbus-1.0/dbus/dbus.h>
#include <dbus.h>

static DBusConnection* connection;

bool init_dbus() {
  return true;
  //return connection = dbus_bus_get(DBUS_BUS_SESSION, NULL);
}

void deinit_dbus() {
  /*
  if (connection)
    dbus_connection_unref(connection);
  */
}

void dbus_send(const char* SMRY, const char* BODY,
  const enum Urg URG, const unsigned TOMS) {
  /*
  if (connection == NULL)
    return;
  */
  DBusConnection* connection = { dbus_bus_get(DBUS_BUS_SESSION, NULL) };
  if (connection) {
  
    DBusMessage* msg = { 
      dbus_message_new_method_call("org.freedesktop.Notifications",
      "/org/freedesktop/Notifications",
      "org.freedesktop.Notifications",
      "Notify") };
    DBusMessageIter I[4];
    dbus_message_iter_init_append(msg, I);
    const char* APP = { "notify_send" };
    dbus_message_iter_append_basic(I, 's', &APP);
    unsigned id = { 0 };
    dbus_message_iter_append_basic(I, 'u', &id);
    const char* ICO = { "dialog-information" };
    dbus_message_iter_append_basic(I, 's', &ICO);
    dbus_message_iter_append_basic(I, 's', &SMRY);
    dbus_message_iter_append_basic(I, 's', &BODY);
    dbus_message_iter_open_container(I, 'a', "s", I + 1);
    dbus_message_iter_close_container(I, I + 1);
    dbus_message_iter_open_container(I, 'a', "{sv}", I + 1);
    dbus_message_iter_open_container(I + 1, 'e', 0, I + 2);
    static const char* URGENCY = { "urgency" };
    dbus_message_iter_append_basic(I + 2, 's', &URGENCY);
    dbus_message_iter_open_container(I + 2, 'v', "y", I + 3);
    dbus_message_iter_append_basic(I + 3, 'y', &URG);
    dbus_message_iter_close_container(I + 2, I + 3);
    dbus_message_iter_close_container(I + 1, I + 2);
    dbus_message_iter_close_container(I, I + 1);
    dbus_message_iter_append_basic(I, 'i', &TOMS);
    
    dbus_connection_send(connection, msg, NULL);
    dbus_message_unref(msg);
    dbus_connection_unref(connection);
    //dbus_connection_flush(connection);
  }
}
