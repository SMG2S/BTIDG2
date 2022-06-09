find_package(Doxygen REQUIRED)
find_package(Python REQUIRED)

message(${Python_EXECUTABLE})

execute_process(COMMAND ${Python_EXECUTABLE} -c "import sphinx;import breathe"  RESULT_VARIABLE sphinx_ret)

if (sphinx_ret EQUAL "0")
  set(Sphinx_FOUND 1)
else()
  set(Sphinx_FOUND 0)
endif()

if(Sphinx_FOUND)
  message("Sphinx_FOUND")
endif()

set(PYTHON_DEP_STRING "Please install Python package breathe (at least version 4.18.0) and sphinx_rtd_theme.")

execute_process(COMMAND ${Python_EXECUTABLE} -c "from packaging import version; import sys; import breathe; sys.exit(version.parse(\"4.18.0\") > version.parse(breathe.__version__))" RESULT_VARIABLE ret)
if (ret)
  message(FATAL_ERROR "Missing Documentation dependency breathe. ${PYTHON_DEP_STRING}")
endif()
execute_process(COMMAND ${Python_EXECUTABLE} -c "import sphinx_rtd_theme" RESULT_VARIABLE ret)
if (ret)
  message(FATAL_ERROR "Missing Documentation dependency sphinx_rtd_theme. ${PYTHON_DEP_STRING}")
endif()


set(DOXYGEN_INPUT_DIRECTORY ${PROJECT_SOURCE_DIR})
message("DOXYGEN_INPUT_DIRECTORY = ${DOXYGEN_INPUT_DIRECTORY}")
set(DOXYGEN_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/doxygen)
message("DOXYGEN_OUTPUT_DIRECTORY = ${DOXYGEN_OUTPUT_DIRECTORY}")
set(DOXYGEN_HTML_OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/doxygen/html)
message("DOXYGEN_HTML_OUTPUT = ${DOXYGEN_HTML_OUTPUT}")

set(DOXYGEN_INDEX_FILE ${DOXYGEN_OUTPUT_DIRECTORY}/xml/index.xml)
set(DOXYGEN_GENERATE_HTML YES)
set(DOXYGEN_GENERATE_XML YES)
set(DOXYGEN_ENABLE_PREPROCESSING YES)
set(DOXYGEN_MACRO_EXPANSION YES)
set(DOXYGEN_EXPAND_ONLY_PREDEF YES)
set(DOXYGEN_INTERNAL_DOCS NO)

make_directory(${DOXYGEN_HTML_OUTPUT})

doxygen_add_docs(Doxygen
    ${DOXYGEN_INPUT_DIRECTORY}
    DOXYGEN_PROJECT_NAME ChASE
    DOXYGEN_PROJECT_NUMBER 0.91
    COMMENT "Generating docs")

set(SPHINX_SOURCE ${CMAKE_CURRENT_BINARY_DIR})
set(SPHINX_BUILD ${CMAKE_BINARY_DIR}/sphinx/html)
set(SPHINX_INDEX_FILE ${SPHINX_BUILD}/index.html)

if (UNIX)
  set(separator ":")
else ()
  set(separator ";")
endif()

set(RST_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/*.rst
  )

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/conf.py.in ${CMAKE_CURRENT_BINARY_DIR}/conf.py)

add_custom_command(OUTPUT ${SPHINX_INDEX_FILE}
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different ${RST_FILES} ${CMAKE_CURRENT_BINARY_DIR}    
                   COMMAND  ${Python_EXECUTABLE} -m sphinx -b html
                   ${SPHINX_SOURCE} ${SPHINX_BUILD}
                   WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                   DEPENDS
                   ${RST_FILES}
                   Doxygen
                   MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/conf.py.in
                   COMMENT "Generating HTML documentation with Sphinx into {CMAKE_BINARY_DIR}/sphinx/html/")

add_custom_target(Sphinx ALL DEPENDS ${SPHINX_INDEX_FILE})


find_package(Git)

if(Git_FOUND)
    add_custom_target(Deploy
        COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/deploy.sh ${CMAKE_BINARY_DIR}/sphinx/old_html ${CMAKE_BINARY_DIR}/sphinx/html
        COMMENT "Uploading documentation into Github Pages")
endif()

find_package(LATEX)
set(SPHINX_TEX_BUILD ${CMAKE_BINARY_DIR}/sphinx/latex)
set(SPHINX_TEX_INDEX_FILE ${SPHINX_TEX_BUILD}/index.tex)

add_custom_command(OUTPUT ${SPHINX_TEX_INDEX_FILE}
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different ${RST_FILES} ${CMAKE_CURRENT_BINARY_DIR}
                   COMMAND ${Python_EXECUTABLE} -m sphinx -b latex
                   ${SPHINX_SOURCE} ${SPHINX_TEX_BUILD}
                   WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                   DEPENDS
                   ${RST_FILES}
                   Doxygen
                   MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/conf.py.in
                   COMMENT "Generating LATEX documentation with Sphinx into {CMAKE_BINARY_DIR}/sphinx/latex/")

set(LATEX_PDF_FILE ${CMAKE_PROJECT_NAME}_${CMAKE_PROJECT_VERSION}.pdf)
make_directory(${CMAKE_BINARY_DIR}/sphinx/pdf)
add_custom_target(PDF
    COMMAND ${XELATEX_COMPILER} -output-directory ${SPHINX_TEX_BUILD} -synctex=1 -interaction=nonstopmode ${CMAKE_BINARY_DIR}/sphinx/latex/*.tex
                  COMMAND ${CMAKE_COMMAND} -E copy *.pdf ${CMAKE_BINARY_DIR}/sphinx/pdf/${LATEX_PDF_FILE}.pdf
                  WORKING_DIRECTORY ${SPHINX_TEX_BUILD}
                  DEPENDS ${SPHINX_TEX_INDEX_FILE}
                  COMMENT "Generating PDF documentation into ${CMAKE_BINARY_DIR}/sphinx/pdf/")

