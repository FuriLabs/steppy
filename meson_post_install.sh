#!/bin/sh

glib-compile-schemas $MESON_INSTALL_PREFIX/share/glib-2.0/schemas
update-desktop-database -q
