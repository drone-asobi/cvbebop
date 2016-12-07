#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "opencv_world" for configuration "Release"
set_property(TARGET opencv_world APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(opencv_world PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_world310.lib"
  IMPORTED_LINK_INTERFACE_LIBRARIES_RELEASE ""
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_world310.dll"
  )

list(APPEND _IMPORT_CHECK_TARGETS opencv_world )
list(APPEND _IMPORT_CHECK_FILES_FOR_opencv_world "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_world310.lib" "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_world310.dll" )

# Import target "opencv_ximgproc" for configuration "Release"
set_property(TARGET opencv_ximgproc APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(opencv_ximgproc PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_ximgproc310.lib"
  IMPORTED_LINK_INTERFACE_LIBRARIES_RELEASE "opencv_world"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_ximgproc310.dll"
  )

list(APPEND _IMPORT_CHECK_TARGETS opencv_ximgproc )
list(APPEND _IMPORT_CHECK_FILES_FOR_opencv_ximgproc "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_ximgproc310.lib" "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_ximgproc310.dll" )

# Import target "opencv_xobjdetect" for configuration "Release"
set_property(TARGET opencv_xobjdetect APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(opencv_xobjdetect PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_xobjdetect310.lib"
  IMPORTED_LINK_INTERFACE_LIBRARIES_RELEASE "opencv_world"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_xobjdetect310.dll"
  )

list(APPEND _IMPORT_CHECK_TARGETS opencv_xobjdetect )
list(APPEND _IMPORT_CHECK_FILES_FOR_opencv_xobjdetect "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_xobjdetect310.lib" "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_xobjdetect310.dll" )

# Import target "opencv_xphoto" for configuration "Release"
set_property(TARGET opencv_xphoto APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(opencv_xphoto PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_xphoto310.lib"
  IMPORTED_LINK_INTERFACE_LIBRARIES_RELEASE "opencv_world"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_xphoto310.dll"
  )

list(APPEND _IMPORT_CHECK_TARGETS opencv_xphoto )
list(APPEND _IMPORT_CHECK_FILES_FOR_opencv_xphoto "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_xphoto310.lib" "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_xphoto310.dll" )

# Import target "opencv_aruco" for configuration "Release"
set_property(TARGET opencv_aruco APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(opencv_aruco PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_aruco310.lib"
  IMPORTED_LINK_INTERFACE_LIBRARIES_RELEASE "opencv_world"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_aruco310.dll"
  )

list(APPEND _IMPORT_CHECK_TARGETS opencv_aruco )
list(APPEND _IMPORT_CHECK_FILES_FOR_opencv_aruco "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_aruco310.lib" "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_aruco310.dll" )

# Import target "opencv_bgsegm" for configuration "Release"
set_property(TARGET opencv_bgsegm APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(opencv_bgsegm PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_bgsegm310.lib"
  IMPORTED_LINK_INTERFACE_LIBRARIES_RELEASE "opencv_world"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_bgsegm310.dll"
  )

list(APPEND _IMPORT_CHECK_TARGETS opencv_bgsegm )
list(APPEND _IMPORT_CHECK_FILES_FOR_opencv_bgsegm "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_bgsegm310.lib" "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_bgsegm310.dll" )

# Import target "opencv_bioinspired" for configuration "Release"
set_property(TARGET opencv_bioinspired APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(opencv_bioinspired PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_bioinspired310.lib"
  IMPORTED_LINK_INTERFACE_LIBRARIES_RELEASE "opencv_world"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_bioinspired310.dll"
  )

list(APPEND _IMPORT_CHECK_TARGETS opencv_bioinspired )
list(APPEND _IMPORT_CHECK_FILES_FOR_opencv_bioinspired "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_bioinspired310.lib" "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_bioinspired310.dll" )

# Import target "opencv_ccalib" for configuration "Release"
set_property(TARGET opencv_ccalib APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(opencv_ccalib PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_ccalib310.lib"
  IMPORTED_LINK_INTERFACE_LIBRARIES_RELEASE "opencv_world"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_ccalib310.dll"
  )

list(APPEND _IMPORT_CHECK_TARGETS opencv_ccalib )
list(APPEND _IMPORT_CHECK_FILES_FOR_opencv_ccalib "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_ccalib310.lib" "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_ccalib310.dll" )

# Import target "opencv_dpm" for configuration "Release"
set_property(TARGET opencv_dpm APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(opencv_dpm PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_dpm310.lib"
  IMPORTED_LINK_INTERFACE_LIBRARIES_RELEASE "opencv_world"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_dpm310.dll"
  )

list(APPEND _IMPORT_CHECK_TARGETS opencv_dpm )
list(APPEND _IMPORT_CHECK_FILES_FOR_opencv_dpm "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_dpm310.lib" "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_dpm310.dll" )

# Import target "opencv_face" for configuration "Release"
set_property(TARGET opencv_face APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(opencv_face PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_face310.lib"
  IMPORTED_LINK_INTERFACE_LIBRARIES_RELEASE "opencv_world"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_face310.dll"
  )

list(APPEND _IMPORT_CHECK_TARGETS opencv_face )
list(APPEND _IMPORT_CHECK_FILES_FOR_opencv_face "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_face310.lib" "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_face310.dll" )

# Import target "opencv_fuzzy" for configuration "Release"
set_property(TARGET opencv_fuzzy APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(opencv_fuzzy PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_fuzzy310.lib"
  IMPORTED_LINK_INTERFACE_LIBRARIES_RELEASE "opencv_world"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_fuzzy310.dll"
  )

list(APPEND _IMPORT_CHECK_TARGETS opencv_fuzzy )
list(APPEND _IMPORT_CHECK_FILES_FOR_opencv_fuzzy "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_fuzzy310.lib" "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_fuzzy310.dll" )

# Import target "opencv_line_descriptor" for configuration "Release"
set_property(TARGET opencv_line_descriptor APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(opencv_line_descriptor PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_line_descriptor310.lib"
  IMPORTED_LINK_INTERFACE_LIBRARIES_RELEASE "opencv_world"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_line_descriptor310.dll"
  )

list(APPEND _IMPORT_CHECK_TARGETS opencv_line_descriptor )
list(APPEND _IMPORT_CHECK_FILES_FOR_opencv_line_descriptor "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_line_descriptor310.lib" "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_line_descriptor310.dll" )

# Import target "opencv_optflow" for configuration "Release"
set_property(TARGET opencv_optflow APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(opencv_optflow PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_optflow310.lib"
  IMPORTED_LINK_INTERFACE_LIBRARIES_RELEASE "opencv_world;opencv_ximgproc"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_optflow310.dll"
  )

list(APPEND _IMPORT_CHECK_TARGETS opencv_optflow )
list(APPEND _IMPORT_CHECK_FILES_FOR_opencv_optflow "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_optflow310.lib" "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_optflow310.dll" )

# Import target "opencv_plot" for configuration "Release"
set_property(TARGET opencv_plot APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(opencv_plot PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_plot310.lib"
  IMPORTED_LINK_INTERFACE_LIBRARIES_RELEASE "opencv_world"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_plot310.dll"
  )

list(APPEND _IMPORT_CHECK_TARGETS opencv_plot )
list(APPEND _IMPORT_CHECK_FILES_FOR_opencv_plot "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_plot310.lib" "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_plot310.dll" )

# Import target "opencv_reg" for configuration "Release"
set_property(TARGET opencv_reg APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(opencv_reg PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_reg310.lib"
  IMPORTED_LINK_INTERFACE_LIBRARIES_RELEASE "opencv_world"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_reg310.dll"
  )

list(APPEND _IMPORT_CHECK_TARGETS opencv_reg )
list(APPEND _IMPORT_CHECK_FILES_FOR_opencv_reg "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_reg310.lib" "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_reg310.dll" )

# Import target "opencv_rgbd" for configuration "Release"
set_property(TARGET opencv_rgbd APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(opencv_rgbd PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_rgbd310.lib"
  IMPORTED_LINK_INTERFACE_LIBRARIES_RELEASE "opencv_world"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_rgbd310.dll"
  )

list(APPEND _IMPORT_CHECK_TARGETS opencv_rgbd )
list(APPEND _IMPORT_CHECK_FILES_FOR_opencv_rgbd "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_rgbd310.lib" "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_rgbd310.dll" )

# Import target "opencv_saliency" for configuration "Release"
set_property(TARGET opencv_saliency APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(opencv_saliency PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_saliency310.lib"
  IMPORTED_LINK_INTERFACE_LIBRARIES_RELEASE "opencv_world"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_saliency310.dll"
  )

list(APPEND _IMPORT_CHECK_TARGETS opencv_saliency )
list(APPEND _IMPORT_CHECK_FILES_FOR_opencv_saliency "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_saliency310.lib" "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_saliency310.dll" )

# Import target "opencv_stereo" for configuration "Release"
set_property(TARGET opencv_stereo APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(opencv_stereo PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_stereo310.lib"
  IMPORTED_LINK_INTERFACE_LIBRARIES_RELEASE "opencv_world"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_stereo310.dll"
  )

list(APPEND _IMPORT_CHECK_TARGETS opencv_stereo )
list(APPEND _IMPORT_CHECK_FILES_FOR_opencv_stereo "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_stereo310.lib" "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_stereo310.dll" )

# Import target "opencv_structured_light" for configuration "Release"
set_property(TARGET opencv_structured_light APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(opencv_structured_light PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_structured_light310.lib"
  IMPORTED_LINK_INTERFACE_LIBRARIES_RELEASE "opencv_world;opencv_rgbd"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_structured_light310.dll"
  )

list(APPEND _IMPORT_CHECK_TARGETS opencv_structured_light )
list(APPEND _IMPORT_CHECK_FILES_FOR_opencv_structured_light "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_structured_light310.lib" "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_structured_light310.dll" )

# Import target "opencv_surface_matching" for configuration "Release"
set_property(TARGET opencv_surface_matching APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(opencv_surface_matching PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_surface_matching310.lib"
  IMPORTED_LINK_INTERFACE_LIBRARIES_RELEASE "opencv_world"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_surface_matching310.dll"
  )

list(APPEND _IMPORT_CHECK_TARGETS opencv_surface_matching )
list(APPEND _IMPORT_CHECK_FILES_FOR_opencv_surface_matching "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_surface_matching310.lib" "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_surface_matching310.dll" )

# Import target "opencv_text" for configuration "Release"
set_property(TARGET opencv_text APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(opencv_text PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_text310.lib"
  IMPORTED_LINK_INTERFACE_LIBRARIES_RELEASE "opencv_world"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_text310.dll"
  )

list(APPEND _IMPORT_CHECK_TARGETS opencv_text )
list(APPEND _IMPORT_CHECK_FILES_FOR_opencv_text "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_text310.lib" "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_text310.dll" )

# Import target "opencv_ts" for configuration "Release"
set_property(TARGET opencv_ts APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(opencv_ts PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LINK_INTERFACE_LIBRARIES_RELEASE "opencv_world;ippicv"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_ts310.lib"
  )

list(APPEND _IMPORT_CHECK_TARGETS opencv_ts )
list(APPEND _IMPORT_CHECK_FILES_FOR_opencv_ts "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_ts310.lib" )

# Import target "opencv_datasets" for configuration "Release"
set_property(TARGET opencv_datasets APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(opencv_datasets PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_datasets310.lib"
  IMPORTED_LINK_INTERFACE_LIBRARIES_RELEASE "opencv_world;opencv_face;opencv_text"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_datasets310.dll"
  )

list(APPEND _IMPORT_CHECK_TARGETS opencv_datasets )
list(APPEND _IMPORT_CHECK_FILES_FOR_opencv_datasets "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_datasets310.lib" "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_datasets310.dll" )

# Import target "opencv_tracking" for configuration "Release"
set_property(TARGET opencv_tracking APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(opencv_tracking PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_tracking310.lib"
  IMPORTED_LINK_INTERFACE_LIBRARIES_RELEASE "opencv_world;opencv_face;opencv_text;opencv_datasets"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_tracking310.dll"
  )

list(APPEND _IMPORT_CHECK_TARGETS opencv_tracking )
list(APPEND _IMPORT_CHECK_FILES_FOR_opencv_tracking "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_tracking310.lib" "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_tracking310.dll" )

# Import target "opencv_contrib_world" for configuration "Release"
set_property(TARGET opencv_contrib_world APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(opencv_contrib_world PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_contrib_world310.lib"
  IMPORTED_LINK_INTERFACE_LIBRARIES_RELEASE ""
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_contrib_world310.dll"
  )

list(APPEND _IMPORT_CHECK_TARGETS opencv_contrib_world )
list(APPEND _IMPORT_CHECK_FILES_FOR_opencv_contrib_world "${_IMPORT_PREFIX}/x86/vc14/lib/opencv_contrib_world310.lib" "${_IMPORT_PREFIX}/x86/vc14/bin/opencv_contrib_world310.dll" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
