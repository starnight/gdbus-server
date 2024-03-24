# D-Bus Server Example

This is a D-Bus server example written with GTK GLib C & Gio.

## Build & Execute

```sh
$ meson setup builddir
$ cd builddir
$ meson compile
$ ./gdbus-server
```

## Introspect the Bus' Object

Introspect the bus name `io.starnight.dbus_test.TestServer`'s object
`/io/starnight/dbus_test/TestObject` with `dbus-send` command:
```sh
$ dbus-send --session --print-reply \
            --type=method_call \
            --dest=io.starnight.dbus_test.TestServer \
            /io/starnight/dbus_test/TestObject \
            org.freedesktop.DBus.Introspectable.Introspect
method return time=1711180494.668633 sender=:1.188 -> destination=:1.189 serial=3 reply_serial=2
   string "<!DOCTYPE node PUBLIC "-//freedesktop//DTD D-BUS Object Introspection 1.0//EN"
                      "http://www.freedesktop.org/standards/dbus/1.0/introspect.dtd">
<!-- GDBus 2.78.4 -->
<node>
  <interface name="org.freedesktop.DBus.Properties">
    <method name="Get">
      <arg type="s" name="interface_name" direction="in"/>
      <arg type="s" name="property_name" direction="in"/>
      <arg type="v" name="value" direction="out"/>
    </method>
    <method name="GetAll">
      <arg type="s" name="interface_name" direction="in"/>
      <arg type="a{sv}" name="properties" direction="out"/>
    </method>
    <method name="Set">
      <arg type="s" name="interface_name" direction="in"/>
      <arg type="s" name="property_name" direction="in"/>
      <arg type="v" name="value" direction="in"/>
    </method>
    <signal name="PropertiesChanged">
      <arg type="s" name="interface_name"/>
      <arg type="a{sv}" name="changed_properties"/>
      <arg type="as" name="invalidated_properties"/>
    </signal>
  </interface>
  <interface name="org.freedesktop.DBus.Introspectable">
    <method name="Introspect">
      <arg type="s" name="xml_data" direction="out"/>
    </method>
  </interface>
  <interface name="org.freedesktop.DBus.Peer">
    <method name="Ping"/>
    <method name="GetMachineId">
      <arg type="s" name="machine_uuid" direction="out"/>
    </method>
  </interface>
  <interface name="io.starnight.dbus_test.TestInterface">
    <method name="Login">
      <arg type="s" name="greeting" direction="in">
      </arg>
      <arg type="s" name="response" direction="out">
      </arg>
    </method>
    <method name="SendMsg">
      <arg type="s" name="msg" direction="in">
      </arg>
    </method>
    <signal name="MsgNotification">
      <arg type="s" name="sender">
      </arg>
      <arg type="s" name="msg">
      </arg>
    </signal>
    <property type="s" name="Title" access="readwrite">
    </property>
  </interface>
</node>
"
```

## Reference

* [D-Bus](https://www.freedesktop.org/wiki/Software/dbus/)
* [GLib](https://docs.gtk.org/glib/index.html)
* [Gio's test gdbus-example-server.c](https://gitlab.gnome.org/GNOME/glib/-/blob/2.80.0/gio/tests/gdbus-example-server.c)
* [dbus-send](https://dbus.freedesktop.org/doc/dbus-send.1.html)
* D-Bus debug tools: [dbus-send](https://dbus.freedesktop.org/doc/dbus-send.1.html), [dbus-monitor](https://dbus.freedesktop.org/doc/dbus-monitor.1.html) and [Bustle](https://gitlab.gnome.org/World/bustle)
