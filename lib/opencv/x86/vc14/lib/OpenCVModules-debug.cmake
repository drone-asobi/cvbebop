#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "opencv_world" for configuration "Debug"
set_property(TARGET opencv_world APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(opencv_world PROPERTIES
  IMPORTED_IMPLIB_DEBUG "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_world310d.lib"
  IMPORTED_LINK_INTERFACE_LIBRARIES_DEBUG ""
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_world310d.dll"
  )

list(APPEND _IMPORT_CHECK_TARGETS opencv_world )
list(APPEND _IMPORT_CHECK_FILES_FOR_opencv_world "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_world310d.lib" "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_world310d.dll" )

# Import target "opencv_ximgproc" for configuration "Debug"
set_property(TARGET opencv_ximgproc APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(opencv_ximgproc PROPERTIES
  IMPORTED_IMPLIB_DEBUG "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_ximgproc310d.lib"
  IMPORTED_LINK_INTERFACE_LIBRARIES_DEBUG "opencv_world"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_ximgproc310d.dll"
  )

list(APPEND _IMPORT_CHECK_TARGETS opencv_ximgproc )
list(APPEND _IMPORT_CHECK_FILES_FOR_opencv_ximgproc "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_ximgproc310d.lib" "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_ximgproc310d.dll" )

# Import target "opencv_xobjdetect" for configuration "Debug"
set_property(TARGET opencv_xobjdetect APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(opencv_xobjdetect PROPERTIES
  IMPORTED_IMPLIB_DEBUG "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_xobjdetect310d.lib"
  IMPORTED_LINK_INTERFACE_LIBRARIES_DEBUG "opencv_world"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_xobjdetect310d.dll"
  )

list(APPEND _IMPORT_CHECK_TARGETS opencv_xobjdetect )
list(APPEND _IMPORT_CHECK_FILES_FOR_opencv_xobjdetect "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_xobjdetect310d.lib" "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_xobjdetect310d.dll" )

# Import target "opencv_xphoto" for configuration "Debug"
set_property(TARGET opencv_xphoto APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(opencv_xphoto PROPERTIES
  IMPORTED_IMPLIB_DEBUG "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_xphoto310d.lib"
  IMPORTED_LINK_INTERFACE_LIBRARIES_DEBUG "opencv_world"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_xphoto310d.dll"
  )

list(APPEND _IMPORT_CHECK_TARGETS opencv_xphoto )
list(APPEND _IMPORT_CHECK_FILES_FOR_opencv_xphoto "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_xphoto310d.lib" "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_xphoto310d.dll" )

# Import target "opencv_aruco" for configuration "Debug"
set_property(TARGET opencv_aruco APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(opencv_aruco PROPERTIES
  IMPORTED_IMPLIB_DEBUG "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_aruco310d.lib"
  IMPORTED_LINK_INTERFACE_LIBRARIES_DEBUG "opencv_world"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_aruco310d.dll"
  )

list(APPEND _IMPORT_CHECK_TARGETS opencv_aruco )
list(APPEND _IMPORT_CHECK_FILES_FOR_opencv_aruco "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_aruco310d.lib" "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_aruco310d.dll" )

# Import target "opencv_bgsegm" for configuration "Debug"
set_property(TARGET opencv_bgsegm APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(opencv_bgsegm PROPERTIES
  IMPORTED_IMPLIB_DEBUG "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_bgsegm310d.lib"
  IMPORTED_LINK_INTERFACE_LIBRARIES_DEBUG "opencv_world"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_bgsegm310d.dll"
  )

list(APPEND _IMPORT_CHECK_TARGETS opencv_bgsegm )
list(APPEND _IMPORT_CHECK_FILES_FOR_opencv_bgsegm "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_bgsegm310d.lib" "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_bgsegm310d.dll" )

# Import target "opencv_bioinspired" for configuration "Debug"
set_property(TARGET opencv_bioinspired APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(opencv_bioinspired PROPERTIES
  IMPORTED_IMPLIB_DEBUG "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_bioinspired310d.lib"
  IMPORTED_LINK_INTERFACE_LIBRARIES_DEBUG "opencv_world"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_bioinspired310d.dll"
  )

list(APPEND _IMPORT_CHECK_TARGETS opencv_bioinspired )
list(APPEND _IMPORT_CHECK_FILES_FOR_opencv_bioinspired "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_bioinspired310d.lib" "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_bioinspired310d.dll" )

# Import target "opencv_ccalib" for configuration "Debug"
set_property(TARGET opencv_ccalib APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(opencv_ccalib PROPERTIES
  IMPORTED_IMPLIB_DEBUG "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_ccalib310d.lib"
  IMPORTED_LINK_INTERFACE_LIBRARIES_DEBUG "opencv_world"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_ccalib310d.dll"
  )

list(APPEND _IMPORT_CHECK_TARGETS opencv_ccalib )
list(APPEND _IMPORT_CHECK_FILES_FOR_opencv_ccalib "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_ccalib310d.lib" "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_ccalib310d.dll" )

# Import target "opencv_dpm" for configuration "Debug"
set_property(TARGET opencv_dpm APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(opencv_dpm PROPERTIES
  IMPORTED_IMPLIB_DEBUG "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_dpm310d.lib"
  IMPORTED_LINK_INTERFACE_LIBRARIES_DEBUG "opencv_world"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_dpm310d.dll"
  )

list(APPEND _IMPORT_CHECK_TARGETS opencv_dpm )
list(APPEND _IMPORT_CHECK_FILES_FOR_opencv_dpm "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_dpm310d.lib" "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_dpm310d.dll" )

# Import target "opencv_face" for configuration "Debug"
set_property(TARGET opencv_face APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(opencv_face PROPERTIES
  IMPORTED_IMPLIB_DEBUG "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_face310d.lib"
  IMPORTED_LINK_INTERFACE_LIBRARIES_DEBUG "opencv_world"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_face310d.dll"
  )

list(APPEND _IMPORT_CHECK_TARGETS opencv_face )
list(APPEND _IMPORT_CHECK_FILES_FOR_opencv_face "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_face310d.lib" "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_face310d.dll" )

# Import target "opencv_fuzzy" for configuration "Debug"
set_property(TARGET opencv_fuzzy APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(opencv_fuzzy PROPERTIES
  IMPORTED_IMPLIB_DEBUG "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_fuzzy310d.lib"
  IMPORTED_LINK_INTERFACE_LIBRARIES_DEBUG "opencv_world"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_fuzzy310d.dll"
  )

list(APPEND _IMPORT_CHECK_TARGETS opencv_fuzzy )
list(APPEND _IMPORT_CHECK_FILES_FOR_opencv_fuzzy "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_fuzzy310d.lib" "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_fuzzy310d.dll" )

# Import target "opencv_line_descriptor" for configuration "Debug"
set_property(TARGET opencv_line_descriptor APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(opencv_line_descriptor PROPERTIES
  IMPORTED_IMPLIB_DEBUG "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_line_descriptor310d.lib"
  IMPORTED_LINK_INTERFACE_LIBRARIES_DEBUG "opencv_world"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_line_descriptor310d.dll"
  )

list(APPEND _IMPORT_CHECK_TARGETS opencv_line_descriptor )
list(APPEND _IMPORT_CHECK_FILES_FOR_opencv_line_descriptor "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_line_descriptor310d.lib" "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_line_descriptor310d.dll" )

# Import target "opencv_optflow" for configuration "Debug"
set_property(TARGET opencv_optflow APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(opencv_optflow PROPERTIES
  IMPORTED_IMPLIB_DEBUG "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_optflow310d.lib"
  IMPORTED_LINK_INTERFACE_LIBRARIES_DEBUG "opencv_world;opencv_ximgproc"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_optflow310d.dll"
  )

list(APPEND _IMPORT_CHECK_TARGETS opencv_optflow )
list(APPEND _IMPORT_CHECK_FILES_FOR_opencv_optflow "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_optflow310d.lib" "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_optflow310d.dll" )

# Import target "opencv_plot" for configuration "Debug"
set_property(TARGET opencv_plot APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(opencv_plot PROPERTIES
  IMPORTED_IMPLIB_DEBUG "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_plot310d.lib"
  IMPORTED_LINK_INTERFACE_LIBRARIES_DEBUG "opencv_world"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_plot310d.dll"
  )

list(APPEND _IMPORT_CHECK_TARGETS opencv_plot )
list(APPEND _IMPORT_CHECK_FILES_FOR_opencv_plot "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_plot310d.lib" "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_plot310d.dll" )

# Import target "opencv_reg" for configuration "Debug"
set_property(TARGET opencv_reg APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(opencv_reg PROPERTIES
  IMPORTED_IMPLIB_DEBUG "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_reg310d.lib"
  IMPORTED_LINK_INTERFACE_LIBRARIES_DEBUG "opencv_world"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_reg310d.dll"
  )

list(APPEND _IMPORT_CHECK_TARGETS opencv_reg )
list(APPEND _IMPORT_CHECK_FILES_FOR_opencv_reg "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_reg310d.lib" "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_reg310d.dll" )

# Import target "opencv_rgbd" for configuration "Debug"
set_property(TARGET opencv_rgbd APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(opencv_rgbd PROPERTIES
  IMPORTED_IMPLIB_DEBUG "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_rgbd310d.lib"
  IMPORTED_LINK_INTERFACE_LIBRARIES_DEBUG "opencv_world"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_rgbd310d.dll"
  )

list(APPEND _IMPORT_CHECK_TARGETS opencv_rgbd )
list(APPEND _IMPORT_CHECK_FILES_FOR_opencv_rgbd "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_rgbd310d.lib" "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_rgbd310d.dll" )

# Import target "opencv_saliency" for configuration "Debug"
set_property(TARGET opencv_saliency APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(opencv_saliency PROPERTIES
  IMPORTED_IMPLIB_DEBUG "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_saliency310d.lib"
  IMPORTED_LINK_INTERFACE_LIBRARIES_DEBUG "opencv_world"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_saliency310d.dll"
  )

list(APPEND _IMPORT_CHECK_TARGETS opencv_saliency )
list(APPEND _IMPORT_CHECK_FILES_FOR_opencv_saliency "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_saliency310d.lib" "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_saliency310d.dll" )

# Import target "opencv_stereo" for configuration "Debug"
set_property(TARGET opencv_stereo APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(opencv_stereo PROPERTIES
  IMPORTED_IMPLIB_DEBUG "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_stereo310d.lib"
  IMPORTED_LINK_INTERFACE_LIBRARIES_DEBUG "opencv_world"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_stereo310d.dll"
  )

list(APPEND _IMPORT_CHECK_TARGETS opencv_stereo )
list(APPEND _IMPORT_CHECK_FILES_FOR_opencv_stereo "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_stereo310d.lib" "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_stereo310d.dll" )

# Import target "opencv_structured_light" for configuration "Debug"
set_property(TARGET opencv_structured_light APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(opencv_structured_light PROPERTIES
  IMPORTED_IMPLIB_DEBUG "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_structured_light310d.lib"
  IMPORTED_LINK_INTERFACE_LIBRARIES_DEBUG "opencv_world;opencv_rgbd"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_structured_light310d.dll"
  )

list(APPEND _IMPORT_CHECK_TARGETS opencv_structured_light )
list(APPEND _IMPORT_CHECK_FILES_FOR_opencv_structured_light "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_structured_light310d.lib" "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_structured_light310d.dll" )

# Import target "opencv_surface_matching" for configuration "Debug"
set_property(TARGET opencv_surface_matching APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(opencv_surface_matching PROPERTIES
  IMPORTED_IMPLIB_DEBUG "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_surface_matching310d.lib"
  IMPORTED_LINK_INTERFACE_LIBRARIES_DEBUG "opencv_world"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_surface_matching310d.dll"
  )

list(APPEND _IMPORT_CHECK_TARGETS opencv_surface_matching )
list(APPEND _IMPORT_CHECK_FILES_FOR_opencv_surface_matching "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_surface_matching310d.lib" "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_surface_matching310d.dll" )

# Import target "opencv_text" for configuration "Debug"
set_property(TARGET opencv_text APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(opencv_text PROPERTIES
  IMPORTED_IMPLIB_DEBUG "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_text310d.lib"
  IMPORTED_LINK_INTERFACE_LIBRARIES_DEBUG "opencv_world"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_text310d.dll"
  )

list(APPEND _IMPORT_CHECK_TARGETS opencv_text )
list(APPEND _IMPORT_CHECK_FILES_FOR_opencv_text "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_text310d.lib" "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_text310d.dll" )

# Import target "opencv_ts" for configuration "Debug"
set_property(TARGET opencv_ts APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(opencv_ts PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
  IMPORTED_LINK_INTERFACE_LIBRARIES_DEBUG "opencv_world;ippicv"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_ts310d.lib"
  )

list(APPEND _IMPORT_CHECK_TARGETS opencv_ts )
list(APPEND _IMPORT_CHECK_FILES_FOR_opencv_ts "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_ts310d.lib" )

# Import target "opencv_datasets" for configuration "Debug"
set_property(TARGET opencv_datasets APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(opencv_datasets PROPERTIES
  IMPORTED_IMPLIB_DEBUG "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_datasets310d.lib"
  IMPORTED_LINK_INTERFACE_LIBRARIES_DEBUG "opencv_world;opencv_face;opencv_text"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_datasets310d.dll"
  )

list(APPEND _IMPORT_CHECK_TARGETS opencv_datasets )
list(APPEND _IMPORT_CHECK_FILES_FOR_opencv_datasets "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_datasets310d.lib" "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_datasets310d.dll" )

# Import target "opencv_tracking" for configuration "Debug"
set_property(TARGET opencv_tracking APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(opencv_tracking PROPERTIES
  IMPORTED_IMPLIB_DEBUG "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_tracking310d.lib"
  IMPORTED_LINK_INTERFACE_LIBRARIES_DEBUG "opencv_world;opencv_face;opencv_text;opencv_datasets"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_tracking310d.dll"
  )

list(APPEND _IMPORT_CHECK_TARGETS opencv_tracking )
list(APPEND _IMPORT_CHECK_FILES_FOR_opencv_tracking "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_tracking310d.lib" "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_tracking310d.dll" )

# Import target "opencv_contrib_world" for configuration "Debug"
set_property(TARGET opencv_contrib_world APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(opencv_contrib_world PROPERTIES
  IMPORTED_IMPLIB_DEBUG "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_contrib_world310d.lib"
  IMPORTED_LINK_INTERFACE_LIBRARIES_DEBUG ""
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_contrib_world310d.dll"
  )

list(APPEND _IMPORT_CHECK_TARGETS opencv_contrib_world )
list(APPEND _IMPORT_CHECK_FILES_FOR_opencv_contrib_world "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_contrib_world310d.lib" "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_contrib_world310d.dll" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
