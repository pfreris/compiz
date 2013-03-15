find_package (Compiz REQUIRED)

include (CompizPlugin)

compiz_plugin (notification PKGDEPS libmatenotify gtk+-2.0)
compiz_pkg_check_modules (HAVE_LIBMATENOTIFY_0_6_1 libmatenotify>=0.6.1)
if (HAVE_LIBMATENOTIFY_0_6_1_FOUND)
	add_definitions (-DHAVE_LIBMATENOTIFY_0_6_1)
endif (HAVE_LIBMATENOTIFY_0_6_1_FOUND)
