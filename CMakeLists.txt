cmake_minimum_required(VERSION 3.8)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules")

set(CMAKE_CXX_STANDARD 20)

find_package(Alembic)
find_package(OpenEXR)

add_executable(abc_attrib_db main.cc)

target_include_directories(abc_attrib_db PUBLIC ${INCLUSIONS_ALEMBIC} ${INCLUSIONS_OPENEXR})
target_link_libraries(abc_attrib_db ${BIBLIOTHEQUES_ALEMBIC} ${BIBLIOTHEQUES_OPENEXR})
