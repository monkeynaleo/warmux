FILE(GLOB PO_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.po) 

SET(COPYRIGHT_HOLDER "Wormux Development Team")
SET(BUGS_ADDRESS "wormux-dev@gna.org")
SET(POT_FILE ${CMAKE_CURRENT_BINARY_DIR}/wormux.pot)

# Create PO Template file 'wormux.pot'

ADD_CUSTOM_COMMAND(
  OUTPUT ${POT_FILE}
  COMMAND ${XGETTEXT_EXECUTABLE}
  ARGS -o ${POT_FILE} -s --directory=${CMAKE_SOURCE_DIR} --omit-header --msgid-bugs-address=${BUGS_ADDRESS} -c --keyword=_ --keyword=N_ --copyright-holder=${COPYRIGHT_HOLDER} ${WORMUX_SRCS}
  DEPENDS ${WORMUX_SRCS}
)


# Update .po files and compile them to binary .gmo files

SET(_gmofiles)
FOREACH(_po ${PO_FILES})
  GET_FILENAME_COMPONENT(_lang ${_po} NAME_WE)
  SET(_gmofile ${CMAKE_CURRENT_BINARY_DIR}/${_lang}.gmo)
  ADD_CUSTOM_COMMAND(
    OUTPUT ${_gmofile}
    COMMAND ${MSGMERGE_EXECUTABLE} ARGS --quiet --update -s ${CMAKE_CURRENT_SOURCE_DIR}/${_po} ${POT_FILE}
    COMMAND ${MSGFMT_EXECUTABLE} ARGS -o ${_gmofile} ${CMAKE_CURRENT_SOURCE_DIR}/${_po}
    DEPENDS ${POT_FILE} ${CMAKE_CURRENT_SOURCE_DIR}/${_po}
  )
  INSTALL(FILES ${_gmofile} DESTINATION ${PO_DESTINATION}/${_lang}/LC_MESSAGES RENAME wormux.mo)
  LIST(APPEND _gmofiles ${_gmofile})
ENDFOREACH(_po ${PO_FILES})

ADD_CUSTOM_TARGET(translations ALL DEPENDS ${_gmofiles})
