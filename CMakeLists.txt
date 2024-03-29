cmake_minimum_required(VERSION 3.7)
project(mcc VERSION 0.1.0 LANGUAGES CXX)

set (SOURCE_FILES
	"src/mcc/entry.cpp"
	"src/mcc/result.hpp"
	"src/mcc/config.hpp"
	"src/mcc/config.cpp"

	"src/mcc/memory/endianness.hpp"
	
	"src/mcc/gl/usage.hpp"
	"src/mcc/gl/shader.hpp"
	"src/mcc/gl/shader.cpp"
	"src/mcc/gl/index_buffer.hpp"
	"src/mcc/gl/index_buffer.cpp"
	"src/mcc/gl/vertex_buffer.hpp"
	"src/mcc/gl/vertex_buffer.cpp"
	"src/mcc/gl/vertex_array.hpp"
	"src/mcc/gl/vertex_array.cpp"
	"src/mcc/gl/mesh.hpp"
	"src/mcc/gl/mesh.cpp"
	"src/mcc/gl/voxel.hpp"
	"src/mcc/gl/voxel.cpp"
	"src/mcc/gl/debug.hpp"
	"src/mcc/gl/debug.cpp"

	"src/mcc/data/manager.hpp"
	"src/mcc/data/handle.hpp"
	"src/mcc/data/model.hpp"
	"src/mcc/data/model.cpp"
	"src/mcc/data/manager.cpp"
	
	"src/mcc/data/loader.hpp"
	"src/mcc/data/loader.cpp"
	"src/mcc/data/qb_parser.hpp"
	"src/mcc/data/qb_parser.cpp"

	"src/mcc/entity/entity.hpp"
	"src/mcc/entity/entity.cpp"
	"src/mcc/entity/bounding_box.hpp"
	"src/mcc/entity/bounding_box.cpp"

	"src/mcc/map/chunk.hpp"
	"src/mcc/map/chunk.cpp"
	"src/mcc/map/generator.hpp"
	"src/mcc/map/generator.cpp"

	"src/mcc/ui/camera.hpp"
	"src/mcc/ui/camera.cpp"
 "src/mcc/gl/deferred_renderer.hpp"  "src/mcc/gl/deferred_renderer.cpp")

add_executable(mcc-game ${SOURCE_FILES})
set_target_properties(mcc-game PROPERTIES
	CXX_STANDARD 17
	RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin
)

target_compile_definitions(mcc-game
	PRIVATE
		MCC_VERSION="${PROJECT_VERSION}"
		MCC_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
		MCC_VERSION_MINOR=${PROJECT_VERSION_MINOR}
		MCC_VERSION_PATCH=${PROJECT_VERSION_PATCH}
		GLEW_STATIC
)

target_include_directories(mcc-game
	PRIVATE
		"src/"
)

# Dependencies
find_package(GLEW REQUIRED)
target_link_libraries(mcc-game PRIVATE GLEW::GLEW)

find_package(OpenGL REQUIRED)
target_link_libraries(mcc-game PRIVATE OpenGL::GL)

find_package(glm REQUIRED)
target_link_libraries(mcc-game PRIVATE glm)

find_package(glfw3 REQUIRED)
target_link_libraries(mcc-game PRIVATE glfw)

find_package(freetype CONFIG REQUIRED)
target_link_libraries(mcc-game PRIVATE freetype)
