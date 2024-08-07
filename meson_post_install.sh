#!/bin/sh

# this should not run for debian packaging, debian takes care of glib schemas with its own hooks
[ ! -z ${DEB_HOST_MULTIARCH} ] && exit 0

echo "Compiling glib schemas and updating desktop file database"
glib-compile-schemas $MESON_INSTALL_PREFIX/share/glib-2.0/schemas
update-desktop-database -q
