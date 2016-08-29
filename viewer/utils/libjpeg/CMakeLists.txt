
add_library (jpeg jpeg-data.c jpeg-marker.c)
SET_TARGET_PROPERTIES( jpeg PROPERTIES COMPILE_FLAGS "-fPIC")
