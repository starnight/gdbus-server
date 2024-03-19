#include <gio/gio.h>

#define DBUS_TEST_BUS_NAME              "io.starnight.dbus_test.TestServer"
#define DBUS_TEST_OBJECT_PATH           "/io/starnight/dbus_test/TestObject"
#define DBUS_TEST_INTERFACE_NAME        "io.starnight.dbus_test.TestInterface"

static GDBusNodeInfo *introspection_data = NULL;

/* Introspection data for the service we are exporting */
static const gchar introspection_xml[] =
  "<node>"
  "  <interface name='" DBUS_TEST_INTERFACE_NAME "'>"
  "    <method name='Login'>"
  "      <arg type='s' name='greeting' direction='in'/>"
  "      <arg type='s' name='response' direction='out'/>"
  "    </method>"
  "    <method name='SendMsg'>"
  "      <arg type='s' name='msg' direction='in'/>"
  "    </method>"
  "    <signal name='MsgNotification'>"
  "      <arg type='s' name='sender'/>"
  "      <arg type='s' name='msg'/>"
  "    </signal>"
  "    <property type='s' name='Title' access='readwrite'/>"
  "  </interface>"
  "</node>";

static void
handle_login(GDBusConnection       *connection,
             const gchar           *sender,
             const gchar           *object_path,
             const gchar           *interface_name,
             const gchar           *method_name,
             GVariant              *parameters,
             GDBusMethodInvocation *invocation,
             gpointer               user_data)
{
  const gchar *greeting;
  gchar *response;

  g_variant_get (parameters, "(&s)", &greeting);
  response = g_strdup_printf ("Got '%s' from %s", greeting, sender);
  g_dbus_method_invocation_return_value (invocation,
                                         g_variant_new ("(s)", response));
  g_free (response);
}

static void
handle_sendmsg(GDBusConnection       *connection,
               const gchar           *sender,
               const gchar           *object_path,
               const gchar           *interface_name,
               const gchar           *method_name,
               GVariant              *parameters,
               GDBusMethodInvocation *invocation,
               gpointer               user_data)
{
  GError *local_error = NULL;
  const gchar *msg;

  g_variant_get (parameters, "(&s)", &msg);
  g_dbus_connection_emit_signal (connection,
                                 NULL,
                                 object_path,
                                 interface_name,
                                 "MsgNotification",
                                 g_variant_new ("(ss)", sender, msg),
                                 &local_error);

  g_assert_no_error (local_error);
  g_dbus_method_invocation_return_value (invocation, NULL);
}

static void
handle_method_call (GDBusConnection       *connection,
                    const gchar           *sender,
                    const gchar           *object_path,
                    const gchar           *interface_name,
                    const gchar           *method_name,
                    GVariant              *parameters,
                    GDBusMethodInvocation *invocation,
                    gpointer               user_data)
{
  if (g_strcmp0 (method_name, "Login") == 0)
  {
    handle_login(connection,
                 sender,
                 object_path,
                 interface_name,
                 method_name,
                 parameters,
                 invocation,
                 user_data);
  }
  else if (g_strcmp0 (method_name, "SendMsg") == 0)
  {
    handle_sendmsg(connection,
                   sender,
                   object_path,
                   interface_name,
                   method_name,
                   parameters,
                   invocation,
                   user_data);
  }
  else
  {
    g_dbus_method_invocation_return_dbus_error (invocation,
                                                DBUS_TEST_INTERFACE_NAME ".Failed",
                                                "Unsupported method call");
  }
}

static gchar *_title = NULL;

static GVariant *
handle_get_title (GDBusConnection  *connection,
                  const gchar      *sender,
                  const gchar      *object_path,
                  const gchar      *interface_name,
                  const gchar      *property_name,
                  GError          **error,
                  gpointer          user_data)
{
  if (_title == NULL)
    _title = g_strdup ("Chat D-Bus!");

  return g_variant_new_string (_title);
}

static GVariant *
handle_get_property (GDBusConnection  *connection,
                     const gchar      *sender,
                     const gchar      *object_path,
                     const gchar      *interface_name,
                     const gchar      *property_name,
                     GError          **error,
                     gpointer          user_data)
{
  GVariant *ret = NULL;

  if (g_strcmp0 (property_name, "Title") == 0)
  {
    ret = handle_get_title (connection,
                            sender,
                            object_path,
                            interface_name,
                            property_name,
                            error,
                            user_data);
  }

  return ret;
}

static gboolean
handle_set_title (GDBusConnection  *connection,
                  const gchar      *sender,
                  const gchar      *object_path,
                  const gchar      *interface_name,
                  const gchar      *property_name,
                  GVariant         *value,
                  GError          **error,
                  gpointer          user_data)
{
  GVariantBuilder *builder;
  GError *local_error = NULL;

  g_free (_title);
  _title = g_variant_dup_string (value, NULL);

  builder = g_variant_builder_new (G_VARIANT_TYPE_ARRAY);
  g_variant_builder_add (builder,
                         "{sv}",
                         "Title",
                         g_variant_new_string (_title));
  g_dbus_connection_emit_signal (connection,
                                 NULL,
                                 object_path,
                                 "org.freedesktop.DBus.Properties",
                                 "PropertiesChanged",
                                 g_variant_new ("(sa{sv}as)",
                                                interface_name,
                                                builder,
                                                NULL),
                                 &local_error);
  g_assert_no_error (local_error);

  return TRUE;
}

static gboolean
handle_set_property (GDBusConnection  *connection,
                     const gchar      *sender,
                     const gchar      *object_path,
                     const gchar      *interface_name,
                     const gchar      *property_name,
                     GVariant         *value,
                     GError          **error,
                     gpointer          user_data)
{
  gboolean ret = FALSE;

  if (g_strcmp0 (property_name, "Title") == 0)
  {
    ret = handle_set_title(connection,
                           sender,
                           object_path,
                           interface_name,
                           property_name,
                           value,
                           error,
                           user_data);
  }

  return ret;
}

static const GDBusInterfaceVTable interface_vtable =
{
  handle_method_call,
  handle_get_property,
  handle_set_property,
  { 0 }
};

static void
on_bus_acquired (GDBusConnection *connection,
                 const gchar     *name,
                 gpointer         user_data)
{
  guint registration_id;

  registration_id = g_dbus_connection_register_object (connection,
                                                       DBUS_TEST_OBJECT_PATH,
                                                       introspection_data->interfaces[0],
                                                       &interface_vtable,
                                                       NULL,  /* user_data */
                                                       NULL,  /* user_data_free_func */
                                                       NULL); /* GError** */
  g_assert (registration_id > 0);
}

static void
on_name_acquired (GDBusConnection *connection,
                  const gchar     *name,
                  gpointer         user_data)
{
}

static void
on_name_lost (GDBusConnection *connection,
              const gchar     *name,
              gpointer         user_data)
{
  exit (1);
}

int
main (int argc, char *argv[])
{
  guint owner_id;
  GMainLoop *loop;

  /* I am lazy here - I don't want to manually provide the introspection data
   * structures - so just build them from XML.
   */
  introspection_data = g_dbus_node_info_new_for_xml (introspection_xml, NULL);
  g_assert (introspection_data != NULL);

  owner_id = g_bus_own_name (G_BUS_TYPE_SESSION,
                             DBUS_TEST_BUS_NAME,
                             G_BUS_NAME_OWNER_FLAGS_NONE,
                             on_bus_acquired,
                             on_name_acquired,
                             on_name_lost,
                             NULL,
                             NULL);

  loop = g_main_loop_new (NULL, FALSE);
  g_main_loop_run (loop);

  g_bus_unown_name (owner_id);

  g_dbus_node_info_unref (introspection_data);

  return 0;
}
