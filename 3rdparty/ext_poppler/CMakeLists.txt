SET(PREFIX_ext_poppler "${EXTPREFIX}" )

list(APPEND _PopplerPatch_COMMAND
        ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/no_tests.diff
    )
if(APPLE)
    list(APPEND _PopplerPatch_COMMAND
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/mac_popplerIconv.patch
    )
endif(APPLE)
ExternalProject_Add( ext_poppler
	DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
	URL https://poppler.freedesktop.org/poppler-0.62.0.tar.xz
	URL_MD5 42b801f2defaccb6b6cf1bf783ee1552
	
    PATCH_COMMAND ${_PopplerPatch_COMMAND}

	INSTALL_DIR ${PREFIX_ext_poppler}
	CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_poppler} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE} -DBUILD_GTK_TESTS=OFF -DBUILD_QT5_TESTS=FALSE -DBUILD_CPP_TESTS=FALSE  -DENABLE_UTILS=FALSE -DENABLE_GLIB=FALSE -DENABLE_LIBOPENJPEG=none
	
    UPDATE_COMMAND ""
)
