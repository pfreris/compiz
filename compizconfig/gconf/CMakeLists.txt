if (USE_MATECONF)

    find_package (CompizConfig REQUIRED)

    include (LibCompizConfigCommon)

    include_directories (${CMAKE_SOURCE_DIR}/compizconfig/integration/mate/include
			 ${CMAKE_SOURCE_DIR}/compizconfig/integration/mate/mateconf/include)
    link_directories (${CMAKE_BINARY_DIR}/compizconfig/integration/mate
		      ${CMAKE_BINARY_DIR}/compizconfig/integration/mate/mateconf)

    compizconfig_backend (mateconf PKGDEPS glib-2.0 mateconf-2.0 LIBRARIES
							   compizconfig_mate_integration
							   compizconfig_mate_integration_mateconf_setting_factory
							   compizconfig_mate_integration_mateconf_integrated_setting)
endif (USE_MATECONF)
