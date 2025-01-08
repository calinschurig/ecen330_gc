idf_component_register(SRCS main.c chunk.c d2deque.c render.c game.c gen.c
                       INCLUDE_DIRS .
                       PRIV_REQUIRES config lcd joy pin)
# PRIV_REQUIRES ...
# target_compile_options(${COMPONENT_LIB} PRIVATE "-Wno-format")



