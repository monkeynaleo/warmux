FILE(GLOB_RECURSE BOX2D_SRCS Common/*.cpp Collision/*.cpp Dynamics/*.cpp)

add_library( box2d STATIC ${BOX2D_SRCS} )
