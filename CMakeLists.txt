# Versión de CMake, 3.15+ es una buena base para estas características.
cmake_minimum_required(VERSION 3.15)

# Definición del proyecto, habilitando C++ y C.
project(ExamGrader VERSION 1.0 LANGUAGES CXX C)

# Política para silenciar el warning y adoptar comportamiento moderno.
cmake_policy(SET CMP0074 NEW)

# Establecer el estándar de C++ a C++17 y hacerlo un requisito.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Dependencias ---
find_package(OpenCV REQUIRED)
message(STATUS "OpenCV ${OpenCV_VERSION} encontrado.")

add_library(inih STATIC third_party/ini.c)
set_target_properties(inih PROPERTIES LINKER_LANGUAGE C)
target_include_directories(inih PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/third_party)

# --- Estructura de Módulos ---
add_library(grader_lib STATIC
    src/ExamGrader.cpp
    src/ConfigModule.cpp
)

target_include_directories(grader_lib PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(grader_lib PUBLIC
    inih
    ${OpenCV_LIBS}
)

# --- Ejecutable ---
add_executable(exam_grader src/main.cpp)

target_link_libraries(exam_grader PRIVATE grader_lib) # Simplificado, la dependencia con inih es transitiva.

# --- Opciones de Compilación (MÉTODO CORREGIDO) ---

# Se eliminaron las variables CMAKE_CXX_FLAGS_DEBUG y CMAKE_C_FLAGS_DEBUG.
# Ahora aplicamos las opciones directamente a cada objetivo.

# Opciones para 'inih' (es una biblioteca C)
target_compile_options(inih PRIVATE
    # Nota: MSVC ignora las banderas -W, lo cual está bien.
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
    $<$<CONFIG:Release>:-O2>
    # Lógica de depuración portable para C
    $<$<CONFIG:Debug>:$<IF:$<C_COMPILER_ID:MSVC>,/Zi,-g>>
)

# Opciones para 'grader_lib' (es una biblioteca C++)
target_compile_options(grader_lib PRIVATE
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
    $<$<CONFIG:Release>:-O3>
    # Lógica de depuración portable para C++
    $<$<CONFIG:Debug>:$<IF:$<CXX_COMPILER_ID:MSVC>,/Zi,-g>>
)

# Opciones para 'exam_grader' (es un ejecutable C++)
target_compile_options(exam_grader PRIVATE
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
    $<$<CONFIG:Release>:-O3>
    # Lógica de depuración portable para C++
    $<$<CONFIG:Debug>:$<IF:$<CXX_COMPILER_ID:MSVC>,/Zi,-g>>
)

message(STATUS "Proyecto configurado. Usa 'cmake --build build' para compilar.")