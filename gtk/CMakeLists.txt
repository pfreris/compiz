option (BUILD_GTK "Build gtk-window-decorator and Mate related toold" 1)
option (BUILD_MARCO "Gtk-window-decorator marco theme support" 1)
option (BUILD_MATE "MATE integration" 1)

compiz_set (USE_GTK ${BUILD_GTK})
compiz_set (USE_MARCO ${BUILD_MARCO})
compiz_set (USE_MATE ${BUILD_MATE})

if (USE_GTK)
    pkg_check_modules (GTK_WINDOW_DECORATOR
	xrender>=0.8.4
	gtk+-2.0>=2.18.0
	libwnck-1.0
	pangocairo
    )
    if (GTK_WINDOW_DECORATOR_FOUND)
        set (CMAKE_REQUIRED_FLAGS ${GTK_WINDOW_DECORATOR_CFLAGS})
        set (CMAKE_REQUIRED_LIBRARIES ${GTK_WINDOW_DECORATOR_LIBRARIES})
        list (FIND CMAKE_REQUIRED_FLAGS "-D_REENTRANT" REENTRANT_INDEX)
        if (REENTRANT_INDEX)
          list (REMOVE_AT CMAKE_REQUIRED_FLAGS REENTRANT_INDEX)
          list (APPEND CMAKE_REQUIRED_FLAGS "-D_REENTRANT=1")
        endif (REENTRANT_INDEX)
        check_function_exists (wnck_window_has_name HAVE_WNCK_WINDOW_HAS_NAME)
        set (CMAKE_REQUIRED_FLAGS "")
        set (CMAKE_REQUIRED_LIBRARIES "")

        compiz_pkg_check_modules (HAVE_LIBWNCK_2_18_1 libwnck-1.0>=2.18.1)
        compiz_pkg_check_modules (HAVE_LIBWNCK_2_19_4 libwnck-1.0>=2.19.4)

        if (USE_MARCO)
            pkg_check_modules (MARCO libmarco-private)
            if (MARCO_FOUND)
		compiz_pkg_check_modules (HAVE_MARCO_2_15_21 libmarco-private>=2.15.21)
		compiz_pkg_check_modules (HAVE_MARCO_2_17_0 libmarco-private>=2.17.0)
		compiz_pkg_check_modules (HAVE_MARCO_2_23_2 libmarco-private>=2.23.2)
            else (MARCO_FOUND)
		compiz_set (USE_MARCO 0)
            endif (MARCO_FOUND)
        endif (USE_MARCO)

	if (USE_MATE)
	    compiz_pkg_check_modules (MATE_WINDOW_SETTINGS
		mate-window-settings-2.0
		mate-desktop-2.0
	    )

	    if (MATE_WINDOW_SETTINGS_FOUND)
	        exec_program (${PKG_CONFIG_EXECUTABLE} 
		  ARGS "--variable=prefix mate-window-settings-2.0" 
		  OUTPUT_VARIABLE windowsettingsdatadir
		)
		set (windowsettingsdatadir ${windowsettingsdatadir}/share)
		exec_program (${PKG_CONFIG_EXECUTABLE} 
		  ARGS "--variable=libdir mate-window-settings-2.0"
		  OUTPUT_VARIABLE windowsettingslibdir
		)
	    else (MATE_WINDOW_SETTINGS_FOUND)
		compiz_set (USE_MATE 0)
	    endif (MATE_WINDOW_SETTINGS_FOUND)
	endif (USE_MATE)

	if (USE_MATE_KEYBINDINGS)
	    compiz_pkg_check_modules (MATE_KEY_BINDINGS mate-keybindings)
	    if (MATE_KEY_BINDINGS_FOUND)
		exec_program (${PKG_CONFIG_EXECUTABLE}
		  ARGS "--variable=keysdir mate-keybindings"
		  OUTPUT_VARIABLE keybindingsdir
		)
	    else (MATE_KEY_BINDINGS_FOUND)
		compiz_set (USE_MATE_KEYBINDINGS 0)
	    endif (MATE_KEY_BINDINGS_FOUND)
        endif (USE_MATE_KEYBINDINGS)
    else (GTK_WINDOW_DECORATOR_FOUND)
	compiz_set (USE_GTK 0)
    endif (GTK_WINDOW_DECORATOR_FOUND)
endif (USE_GTK)

if (NOT USE_GTK)
    compiz_set (USE_MARCO 0)
endif (NOT USE_GTK)

configure_file (
    ${CMAKE_CURRENT_SOURCE_DIR}/config.h.gtk.in
    ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

add_subdirectory (window-decorator)
add_subdirectory (mate)
