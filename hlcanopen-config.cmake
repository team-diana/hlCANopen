get_filename_component(SELF_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
set(hlcanopen_LIBRARIES ${SELF_DIR}/../../../)
get_filename_component(hlcanopen_INCLUDE_DIRS "${SELF_DIR}/../../../../include/hlcanopen" ABSOLUTE)
