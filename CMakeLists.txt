cmake_minimum_required (VERSION 3.6)

project (PDFMixTool)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

option(FLATPAK_BUILD "Whether this is a build for flatpak" OFF)

if (FLATPAK_BUILD)
    add_compile_definitions(FLATPAK_BUILD)
endif ()

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic")
endif()

find_package (Qt5 REQUIRED COMPONENTS Widgets LinguistTools)

set (
    SRCS
    src/main.cpp
    src/aboutdialog.cpp
    src/aboutdialog.h
    src/editmultipageprofiledialog.cpp
    src/editmultipageprofiledialog.h
    src/editpdfentrydialog.cpp
    src/editpdfentrydialog.h
    src/gui_utils.cpp
    src/gui_utils.h
    src/inputpdffiledelegate.cpp
    src/inputpdffiledelegate.h
    src/inputpdffilewidget.cpp
    src/inputpdffilewidget.h
    src/mainwindow.cpp
    src/mainwindow.h
    src/mouseeventfilter.cpp
    src/mouseeventfilter.h
    src/multipageprofilesmanager.cpp
    src/multipageprofilesmanager.h
    src/pdfinfolabel.cpp
    src/pdfinfolabel.h
    src/pdf_edit_lib/definitions.h
    src/pdf_edit_lib/pdf_info.cpp
    src/pdf_edit_lib/pdf_info.h
    src/pdf_edit_lib/pdf_writer.cpp
    src/pdf_edit_lib/pdf_writer.h
    src/single_file_operations/abstract_operation.cpp
    src/single_file_operations/abstract_operation.h
    src/single_file_operations/add_empty_pages.cpp
    src/single_file_operations/add_empty_pages.h
    src/single_file_operations/booklet.cpp
    src/single_file_operations/booklet.h
    src/single_file_operations/delete_pages.cpp
    src/single_file_operations/delete_pages.h
    src/single_file_operations/extract_pages.cpp
    src/single_file_operations/extract_pages.h
    src/single_file_operations/edit_page_layout.cpp
    src/single_file_operations/edit_page_layout.h
)

file(GLOB TS_FILES
     LIST_DIRECTORIES false
     languages/pdfmixtool_*.ts
)

qt5_add_translation(QM_FILES ${TS_FILES} resources/pdfmixtool_en.ts)

add_executable (
    pdfmixtool
    ${SRCS}
    ${QM_FILES}
    ${EN_QM_FILE}
)

target_link_libraries (
    pdfmixtool
    qpdf
    ${Qt5Widgets_LIBRARIES}
)

install(TARGETS pdfmixtool RUNTIME DESTINATION bin)
if (FLATPAK_BUILD)
install(FILES resources/eu.scarpetta.PDFMixTool.flatpak.desktop DESTINATION share/applications RENAME eu.scarpetta.PDFMixTool.desktop)
else()
install(FILES resources/eu.scarpetta.PDFMixTool.desktop DESTINATION share/applications)
endif()
install(FILES resources/eu.scarpetta.PDFMixTool.appdata.xml DESTINATION share/metainfo)
install(FILES resources/icon.svg DESTINATION share/icons/hicolor/scalable/apps RENAME eu.scarpetta.PDFMixTool.svg)
install(FILES resources/icon_48.png DESTINATION share/icons/hicolor/48x48/apps RENAME eu.scarpetta.PDFMixTool.png)
install(FILES resources/icon_64.png DESTINATION share/icons/hicolor/64x64/apps RENAME eu.scarpetta.PDFMixTool.png)
install(FILES resources/icon_128.png DESTINATION share/icons/hicolor/128x128/apps RENAME eu.scarpetta.PDFMixTool.png)
install(FILES resources/icon_256.png DESTINATION share/icons/hicolor/256x256/apps RENAME eu.scarpetta.PDFMixTool.png)
install(FILES ${QM_FILES} DESTINATION share/pdfmixtool/translations)
install(FILES CHANGELOG.md DESTINATION share/pdfmixtool)
install(FILES AUTHORS.md DESTINATION share/pdfmixtool)
install(FILES TRANSLATORS.md DESTINATION share/pdfmixtool)

add_custom_command(
    TARGET pdfmixtool
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR} -P cmake_install.cmake
)
