cmake_minimum_required (VERSION 3.6)

project (PDFMixTool)

include(GNUInstallDirs)
include(FindPkgConfig)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic")
endif()

pkg_check_modules(QPDF libqpdf)
if (DEFINED QPDF_VERSION)
    string(REPLACE "." ";" QPDF_VERSION_LIST ${QPDF_VERSION})
    list(GET QPDF_VERSION_LIST 0 QPDF_VERSION_MAJOR)
else()
    set(QPDF_VERSION_MAJOR 10)
endif()
configure_file(src/qpdf_version.h.in qpdf_version.h @ONLY)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

file(GLOB TS_FILES
     LIST_DIRECTORIES false
     languages/pdfmixtool_*.ts
)

if(NOT(DEFINED QT_VERSION) OR QT_VERSION EQUAL 5)
    find_package (Qt5 REQUIRED COMPONENTS Widgets LinguistTools)
    set(QT_LIBRARIES ${Qt5Widgets_LIBRARIES})
    qt5_add_translation(QM_FILES ${TS_FILES} resources/pdfmixtool_en.ts)
elseif(QT_VERSION EQUAL 6)
    find_package (Qt6 REQUIRED COMPONENTS Widgets LinguistTools)
    set(QT_LIBRARIES ${Qt6Widgets_LIBRARIES})
    qt6_add_translation(QM_FILES ${TS_FILES} resources/pdfmixtool_en.ts)
endif()

set(TRANSLATIONS_QRC "${CMAKE_CURRENT_BINARY_DIR}/tr.qrc")
file(WRITE ${TRANSLATIONS_QRC} "<RCC><qresource prefix=\"/tr\">")
foreach(QM_FILE ${QM_FILES})
    get_filename_component(QM_FILE_NAME ${QM_FILE} NAME)
    file(APPEND ${TRANSLATIONS_QRC} "<file>${QM_FILE_NAME}</file>")
endforeach()
file(APPEND ${TRANSLATIONS_QRC} "</qresource></RCC>")

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
    src/pdf_edit_lib/definitions.cpp
    src/pdf_edit_lib/definitions.h
    src/pdf_edit_lib/pdf_editor.cpp
    src/pdf_edit_lib/pdf_editor.h
    src/pdf_edit_lib/pdf_info.cpp
    src/pdf_edit_lib/pdf_info.h
    src/operations/abstract_operation.cpp
    src/operations/abstract_operation.h
    src/operations/add_empty_pages.cpp
    src/operations/add_empty_pages.h
    src/operations/alternate_mix.cpp
    src/operations/alternate_mix.h
    src/operations/booklet.cpp
    src/operations/booklet.h
    src/operations/delete_pages.cpp
    src/operations/delete_pages.h
    src/operations/edit_document_info.cpp
    src/operations/edit_document_info.h
    src/operations/extract_pages.cpp
    src/operations/extract_pages.h
    src/operations/edit_page_layout.cpp
    src/operations/edit_page_layout.h
    src/operations/merge.cpp
    src/operations/merge.h
    src/operations/rotate.cpp
    src/operations/rotate.h
    src/widgets/multipage_editor.cpp
    src/widgets/multipage_editor.h
    src/widgets/output_preview.cpp
    src/widgets/output_preview.h
    src/widgets/pages_selector.cpp
    src/widgets/pages_selector.h
    src/widgets/pdfinfolabel.cpp
    src/widgets/pdfinfolabel.h
)

add_executable (
    pdfmixtool
    ${SRCS}
    resources/resources.qrc
    ${TRANSLATIONS_QRC}
)

target_link_libraries (
    pdfmixtool
    qpdf
    ${QT_LIBRARIES}
)

INSTALL(TARGETS pdfmixtool RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
INSTALL(FILES resources/eu.scarpetta.PDFMixTool.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
INSTALL(FILES resources/eu.scarpetta.PDFMixTool.appdata.xml DESTINATION ${CMAKE_INSTALL_DATADIR}/metainfo)
INSTALL(FILES resources/icon.svg DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps RENAME eu.scarpetta.PDFMixTool.svg)
INSTALL(FILES resources/icon_48.png DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/48x48/apps RENAME eu.scarpetta.PDFMixTool.png)
INSTALL(FILES resources/icon_64.png DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/64x64/apps RENAME eu.scarpetta.PDFMixTool.png)
INSTALL(FILES resources/icon_128.png DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/128x128/apps RENAME eu.scarpetta.PDFMixTool.png)
INSTALL(FILES resources/icon_256.png DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/256x256/apps RENAME eu.scarpetta.PDFMixTool.png)
