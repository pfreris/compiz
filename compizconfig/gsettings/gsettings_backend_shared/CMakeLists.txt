include (FindPkgConfig)

compiz_pkg_check_modules (GSETTINGS_UTIL ${_deps})

if (GSETTINGS_UTIL_FOUND)

    set (GSETTINGS_UTIL_LIB_INCLUDES
         ${CMAKE_CURRENT_SOURCE_DIR}
         ${CMAKE_SOURCE_DIR}/compizconfig/integration/mate/include
         ${CMAKE_SOURCE_DIR}/compizconfig/integration/mate/mateconf/include
         ${CMAKE_SOURCE_DIR}/compizconfig/integration/mate/gsettings/include
	 ${compiz_SOURCE_DIR}/compizconfig/libcompizconfig/include
	 ${GSETTINGS_UTIL_INCLUDE_DIRS}
	 ${CMAKE_CURRENT_SOURCE_DIR}/../src)

    include_directories (${GSETTINGS_UTIL_LIB_INCLUDES})

    link_directories (${GSETTINGS_UTIL_LIBRARY_DIRS}
		      ${compiz_BINARY_DIR}/compizconfig/libcompizconfig
                      ${compiz_BINARY_DIR}/compizconfig/integration/mate
                      ${CMAKE_BINARY_DIR}/compizconfig/integration/mate/mateconf
                      ${CMAKE_BINARY_DIR}/compizconfig/integration/mate/gsettings)

    add_library (compizconfig_gsettings_backend_interface STATIC
                 ${CMAKE_CURRENT_SOURCE_DIR}/ccs_gsettings_backend_interface.c)

    add_library (compizconfig_gsettings_wrapper_factory_interface STATIC
                 ${CMAKE_CURRENT_SOURCE_DIR}/ccs_gsettings_wrapper_factory_interface.c)

    add_library (compizconfig_gsettings_wrapper_factory STATIC
                 ${CMAKE_CURRENT_SOURCE_DIR}/ccs_gsettings_wrapper_factory.c)

    add_library (compizconfig_gsettings_wrapper_interface STATIC
                 ${CMAKE_CURRENT_SOURCE_DIR}/ccs_gsettings_interface.c)

    add_library (compizconfig_gsettings_wrapper STATIC
                 ${CMAKE_CURRENT_SOURCE_DIR}/ccs_gsettings_interface_wrapper.c)

    target_link_libraries (compizconfig_gsettings_wrapper
                           compizconfig
                           ${GSETTINGS_UTIL_LIBRARIES})

    add_library (compizconfig_gsettings_util STATIC
                 ${CMAKE_CURRENT_SOURCE_DIR}/gsettings_constants.c
                 ${CMAKE_CURRENT_SOURCE_DIR}/gsettings_util.c)

    target_link_libraries (compizconfig_gsettings_util
                           compizconfig_gsettings_backend_interface
                           compizconfig_gsettings_wrapper_interface
                           compizconfig
                           ${GSETTINGS_UTIL_LIBRARIES})

    add_library (compizconfig_gsettings_backend SHARED
                 ${CMAKE_CURRENT_SOURCE_DIR}/ccs_gsettings_backend.c)

    set (GSETTINGS_BACKEND_SHARED_ADDITIONAL_LIBRARIES
	 compizconfig_gsettings_backend_interface
	 compizconfig_gsettings_util
	 compizconfig_gsettings_wrapper
	 compizconfig_mate_integration
	 compizconfig_mate_integration_gsettings_integrated_setting
	 compizconfig_mate_integration_gsettings_setting_factory
	 compizconfig_mate_integration_gsettings_wrapper_factory
	 compizconfig_gsettings_wrapper_factory_interface
	 compizconfig_gsettings_wrapper_factory
	 compizconfig)

    if (USE_MATECONF)
	list (APPEND GSETTINGS_BACKEND_SHARED_ADDITIONAL_LIBRARIES
	      compizconfig_mate_integration_mateconf_setting_factory)
	list (APPEND GSETTINGS_BACKEND_SHARED_ADDITIONAL_LIBRARIES
	      compizconfig_mate_integration_mateconf_integrated_setting)
    endif (USE_MATECONF)

    target_link_libraries (compizconfig_gsettings_backend
			   ${GSETTINGS_BACKEND_SHARED_ADDITIONAL_LIBRARIES}
                           ${GSETTINGS_UTIL_LIBRARIES})

    install (TARGETS compizconfig_gsettings_backend
	     DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

endif (GSETTINGS_UTIL_FOUND)
