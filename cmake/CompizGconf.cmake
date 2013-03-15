option (
    COMPIZ_DISABLE_SCHEMAS_INSTALL
    "Disables mateconf schema installation with mateconftool"
    OFF
)

set (
    COMPIZ_INSTALL_MATECONF_SCHEMA_DIR ${COMPIZ_INSTALL_MATECONF_SCHEMA_DIR} CACHE PATH
    "Installation path of the mateconf schema file"
)

function (compiz_install_mateconf_schema _src _dst)
    find_program (MATECONFTOOL_EXECUTABLE mateconftool-2)
    mark_as_advanced (FORCE MATECONFTOOL_EXECUTABLE)

    if (MATECONFTOOL_EXECUTABLE AND NOT COMPIZ_DISABLE_SCHEMAS_INSTALL)
	install (CODE "
		if (\"\$ENV{USER}\" STREQUAL \"root\")
		    exec_program (${MATECONFTOOL_EXECUTABLE}
			ARGS \"--get-default-source\"
			OUTPUT_VARIABLE ENV{MATECONF_CONFIG_SOURCE})
		    exec_program (${MATECONFTOOL_EXECUTABLE}
			ARGS \"--makefile-install-rule ${_src} > /dev/null\")
		else (\"\$ENV{USER}\" STREQUAL \"root\")
		    exec_program (${MATECONFTOOL_EXECUTABLE}
			ARGS \"--install-schema-file=${_src} > /dev/null\")
		endif (\"\$ENV{USER}\" STREQUAL \"root\")
		")
    endif ()
    install (
	FILES "${_src}"
	DESTINATION "${_dst}"
    )
endfunction ()

# generate mateconf schema
function (compiz_mateconf_schema _src _dst _inst)
    find_program (XSLTPROC_EXECUTABLE xsltproc)
    mark_as_advanced (FORCE XSLTPROC_EXECUTABLE)

    if (XSLTPROC_EXECUTABLE)
	add_custom_command (
	    OUTPUT ${_dst}
	    COMMAND ${XSLTPROC_EXECUTABLE}
		    -o ${_dst}
		    ${COMPIZ_MATECONF_SCHEMAS_XSLT}
		    ${_src}
	    DEPENDS ${_src}
	)
	compiz_install_mateconf_schema (${_dst} ${_inst})
    endif ()
endfunction ()
