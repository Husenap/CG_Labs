add_library (external_libs)

target_sources(
	external_libs
	PUBLIC
		[[GLAD/glad/glad.h]]
		[[GLAD/KHR/khrplatform.h]]
	PRIVATE
		[[GLAD/glad.c]]
)

target_include_directories (
	external_libs
	SYSTEM
	PUBLIC
		"${CMAKE_SOURCE_DIR}/src/external/GLAD"
)

target_link_libraries (
	external_libs
	PRIVATE
		glfw
		$<$<NOT:$<BOOL:${WIN32}>>:dl>
)

install (TARGETS external_libs DESTINATION lib)
