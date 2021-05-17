cmake_minimum_required (VERSION 3.6)

project (PDFMixTool)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

if (WIN32)
    if(NOT DEFINED QPDF_PATH)
        message(FATAL_ERROR "/!\\ QPDF_PATH is not set, please provide QPDF_PATH pointing to QPDF installation as cmake argument like so: -DQPDF_PATH=<...>")
    endif()
    message(STATUS "o [Note] : You might need to provide the path to QtX installation directory like so : -DCMAKE_PREFIX_PATH=<Qt installation directory")
    message(STATUS "o [Note] : For instance, you can use the following command line : -DCMAKE_PREFIX_PATH=[...]/Qt5/5.15.2/mingw81_64 for mingw64 binaries")
endif()

find_library(qpdf PATHS ${QPDF_PATH}/lib/libqpdf.dll.a )

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -Wno-deprecated-declarations")
endif()

file(GLOB TS_FILES
     LIST_DIRECTORIES false
     languages/pdfmixtool_*.ts
)

if(NOT(DEFINED QT_VERSION) OR QT_VERSION EQUAL 5)
    find_package (Qt5 REQUIRED COMPONENTS Widgets LinguistTools Core)
    set(QT_LIBRARIES Qt5::Widgets Qt5::Core )
    message(STATUS "[DEBUG] : QT_LIBRARIES = ${QT_LIBRARIES}")
    qt5_add_translation(QM_FILES ${TS_FILES} resources/pdfmixtool_en.ts)
elseif(QT_VERSION EQUAL 6)
    find_package (Qt6 REQUIRED COMPONENTS Widgets LinguistTools)
    set(QT_LIBRARIES ${Qt6Widgets_LIBRARIES})
    qt6_add_translation(QM_FILES ${TS_FILES} resources/pdfmixtool_en.ts)
endif()

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
    ${QM_FILES}
    ${EN_QM_FILE}
)

target_include_directories( pdfmixtool SYSTEM PUBLIC
    ${QPDF_PATH}/include
)

target_link_directories(pdfmixtool PUBLIC
    ${QPDF_PATH}/lib/
)

# Linking with Dynamic distribution of Qt on Windows
# So we have to provide Qt with such information so that
# headers will contain __declspec(dllimport) annotations for the symbols to be imported
# as expected by pdfmixtool
if(WIN32)
    target_compile_definitions( pdfmixtool PUBLIC
        QT_DLL
    )
endif()


target_link_libraries ( pdfmixtool
    qpdf
    ${QT_LIBRARIES}
)

file(GLOB ICONS
     LIST_DIRECTORIES false
     resources/*.svg
)

install(TARGETS pdfmixtool RUNTIME DESTINATION bin)
install(FILES resources/eu.scarpetta.PDFMixTool.desktop DESTINATION share/applications)
install(FILES resources/eu.scarpetta.PDFMixTool.appdata.xml DESTINATION share/metainfo)
install(FILES resources/icon.svg DESTINATION share/icons/hicolor/scalable/apps RENAME eu.scarpetta.PDFMixTool.svg)
install(FILES resources/icon_48.png DESTINATION share/icons/hicolor/48x48/apps RENAME eu.scarpetta.PDFMixTool.png)
install(FILES resources/icon_64.png DESTINATION share/icons/hicolor/64x64/apps RENAME eu.scarpetta.PDFMixTool.png)
install(FILES resources/icon_128.png DESTINATION share/icons/hicolor/128x128/apps RENAME eu.scarpetta.PDFMixTool.png)
install(FILES resources/icon_256.png DESTINATION share/icons/hicolor/256x256/apps RENAME eu.scarpetta.PDFMixTool.png)
install(FILES ${ICONS} DESTINATION share/pdfmixtool/icons)
install(FILES ${QM_FILES} DESTINATION share/pdfmixtool/translations)
install(FILES CHANGELOG.md DESTINATION share/pdfmixtool)
install(FILES AUTHORS.md DESTINATION share/pdfmixtool)
install(FILES TRANSLATORS.md DESTINATION share/pdfmixtool)

add_custom_command(
    TARGET pdfmixtool
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR} -P cmake_install.cmake
)
