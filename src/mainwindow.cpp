/* Copyright (C) 2017-2019 Marco Scarpetta
 *
 * This file is part of PDF Mix Tool.
 *
 * PDF Mix Tool is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * PDF Mix Tool is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with PDF Mix Tool. If not, see <http://www.gnu.org/licenses/>.
 */

#include "mainwindow.h"

#include <QApplication>
#include <QTimer>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMenu>
#include <QToolBar>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QDesktopServices>

#include "multipageprofilesmanager.h"
#include "aboutdialog.h"
#include "inputpdffiledelegate.h"
#include "editpdfentrydialog.h"
#include "pdf_edit_lib/pdf_info.h"
#include "pdf_edit_lib/pdf_writer.h"

#define TOOLTIP_STRING "%1 <br /><b>%2<?b>"
#define ICON "%1/../share/icons/hicolor/48x48/apps/eu.scarpetta.PDFMixTool.png"

QDataStream &operator<<(QDataStream &out, const Multipage &multipage)
{
    out << multipage.enabled;

    out << multipage.name.c_str();

    out << multipage.page_width;
    out << multipage.page_height;

    out << multipage.rows;
    out << multipage.columns;

    out << multipage.rotation;

    out << multipage.h_alignment;
    out << multipage.v_alignment;

    out << multipage.margin_left;
    out << multipage.margin_right;
    out << multipage.margin_top;
    out << multipage.margin_bottom;

    out << multipage.spacing;

    return out;
}

QDataStream &operator>>(QDataStream &in, Multipage &multipage)
{
    in >> multipage.enabled;

    char *name;
    in >> name;
    multipage.name = name;

    in >> multipage.page_width;
    in >> multipage.page_height;

    in >> multipage.rows;
    in >> multipage.columns;

    in >> multipage.rotation;

    int h_alignment;
    int v_alignment;
    in >> h_alignment;
    multipage.h_alignment = static_cast<Multipage::Alignment>(h_alignment);
    in >> v_alignment;
    multipage.v_alignment = static_cast<Multipage::Alignment>(v_alignment);

    in >> multipage.margin_left;
    in >> multipage.margin_right;
    in >> multipage.margin_top;
    in >> multipage.margin_bottom;

    in >> multipage.spacing;

    return in;
}

MainWindow::MainWindow(MouseEventFilter *filter, QWidget *parent) :
    QMainWindow(parent),
    m_settings(new QSettings(this)),
    m_output_page_count(new QLabel(this)),
    m_output_pages_error_index(-1),
    m_progress_bar(new QProgressBar(this)),
    m_files_list_view(new QListView(this)),
    m_files_list_model(new QStandardItemModel(this)),
    m_edit_menu(new QMenu(this))
{
    // Main winow properties
    this->setWindowIcon(QIcon(QString(ICON).arg(qApp->applicationDirPath())));
    this->setWindowTitle(tr("PDF Mix Tool"));
    this->restoreGeometry(
                m_settings->value("main_window_geometry").toByteArray()
                );

    // Load custom multipage profiles
    qRegisterMetaTypeStreamOperators<Multipage>("Multipage");
    m_settings->beginGroup("custom_maltipage_profiles");
    for (QString key : m_settings->childKeys())
         m_custom_multipages[key.toInt()] =
                 m_settings->value(key).value<Multipage>();
    m_settings->endGroup();

    // Create other windows
    MultipageProfilesManager *multipage_profiles_manager =
            new MultipageProfilesManager(
                &m_custom_multipages,
                m_settings,
                this);
    AboutDialog *about_dialog = new AboutDialog(new AboutDialog(this));

    // Hide progress bar
    m_progress_bar->hide();

    // Create delegate for files list
    InputPdfFileDelegate *pdfinputfile_delegate =
            new InputPdfFileDelegate(
                filter,
                m_custom_multipages,
                this);

    // Set files list settings
    m_files_list_view->setWordWrap(false);
    m_files_list_view->setSelectionBehavior(QAbstractItemView::SelectItems);
    m_files_list_view->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_files_list_view->setEditTriggers(
                QAbstractItemView::DoubleClicked |
                QAbstractItemView::AnyKeyPressed);
    m_files_list_view->setModel(m_files_list_model);
    m_files_list_view->setItemDelegate(pdfinputfile_delegate);
    m_files_list_view->setFocusPolicy(Qt::WheelFocus);
    m_files_list_view->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_files_list_view->setSpacing(2);
    m_files_list_view->setTabKeyNavigation(true);
    m_files_list_view->viewport()->installEventFilter(this);

    // Add edit menu actions
    m_edit_menu->addAction(tr("Edit"), this, SLOT(edit_menu_activated()));
    m_edit_menu->addAction(tr("View"), this, SLOT(view_menu_activated()));

    // Create toolbar and add actions
    QToolBar *toolbar = new QToolBar(tr("Main toolbar"), this);
    toolbar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    toolbar->setFloatable(false);
    toolbar->setMovable(false);

    QAction *add_file_action = toolbar->addAction(
                QIcon::fromTheme("list-add"),
                tr("Add PDF file"),
                this,
                SLOT(add_pdf_files()));
    QAction *move_up_action = toolbar->addAction(
                QIcon::fromTheme("go-up"),
                tr("Move up"),
                this,
                SLOT(move_up()));
    QAction *move_down_action = toolbar->addAction(
                QIcon::fromTheme("go-down"),
                tr("Move down"),
                this,
                SLOT(move_down()));
    QAction *remove_file_action = toolbar->addAction(
                QIcon::fromTheme("list-remove"),
                tr("Remove file"),
                this,
                SLOT(remove_pdf_file()));

    // Set shortcuts for toolbar buttons
    add_file_action->setShortcut(QKeySequence::Open);
    move_up_action->setShortcut(QKeySequence("Ctrl+up"));
    move_down_action->setShortcut(QKeySequence("Ctrl+down"));
    remove_file_action->setShortcut(QKeySequence::Delete);
    add_file_action->setToolTip(
                QString(TOOLTIP_STRING)
                .arg(
                    add_file_action->text(),
                    add_file_action->shortcut().toString()));
    move_up_action->setToolTip(
                QString(TOOLTIP_STRING)
                .arg(
                    move_up_action->text(),
                    move_up_action->shortcut().toString()));
    move_down_action->setToolTip(
                QString(TOOLTIP_STRING)
                .arg(
                    move_down_action->text(),
                    move_down_action->shortcut().toString()));
    remove_file_action->setToolTip(
                QString(TOOLTIP_STRING)
                .arg(
                    remove_file_action->text(),
                    remove_file_action->shortcut().toString()));

    // Create main menu and add actions
    QPushButton *main_menu_button = new QPushButton(
                QIcon::fromTheme("preferences-other"),
                tr("Menu"));
    main_menu_button->setDefault(true);
    QMenu *main_menu = new QMenu(main_menu_button);
    main_menu->addAction(
                QIcon::fromTheme("document-properties"),
                tr("Multipage profiles…"),
                multipage_profiles_manager,
                SLOT(show()));
    main_menu->addAction(
                QIcon::fromTheme("help-about"),
                tr("About"),
                about_dialog,
                SLOT(show()));
    main_menu->addAction(
                QIcon::fromTheme("application-exit"),
                tr("Exit"),
                qApp,
                SLOT(quit()),
                QKeySequence::Quit);
    main_menu_button->setMenu(main_menu);

    // Create "Generate PDF" button
    QPushButton *generate_pdf_button = new QPushButton(
                QIcon::fromTheme("document-save-as"),
                tr("Generate PDF"),
                this);
    QAction *generate_pdf_action = new QAction(
                tr("Generate PDF"),
                generate_pdf_button);
    generate_pdf_action->setShortcut(QKeySequence::Save);
    generate_pdf_button->addAction(generate_pdf_action);
    generate_pdf_button->setToolTip(
                QString(TOOLTIP_STRING)
                .arg(
                    generate_pdf_button->text(),
                    generate_pdf_action->shortcut().toString()));

    // Add widgets to the main window
    QVBoxLayout *v_layout = new QVBoxLayout();
    QWidget *central_widget = new QWidget(this);
    central_widget->setLayout(v_layout);
    this->setCentralWidget(central_widget);

    QHBoxLayout *layout = new QHBoxLayout();
    layout->addWidget(toolbar);
    layout->addItem(new QSpacerItem(
                        0, 0,
                        QSizePolicy::Expanding, QSizePolicy::Minimum));
    layout->addWidget(main_menu_button);
    v_layout->addLayout(layout);

    v_layout->addWidget(m_files_list_view);

    layout = new QHBoxLayout();
    layout->addWidget(m_output_page_count);
    layout->addWidget(m_progress_bar, 1);
    layout->addItem(new QSpacerItem(
                        0, 0,
                        QSizePolicy::Expanding, QSizePolicy::Minimum));
    layout->addWidget(generate_pdf_button);
    v_layout->addLayout(layout);

    // Connect signals to slots
    connect(m_files_list_view, SIGNAL(pressed(QModelIndex)),
            this, SLOT(item_mouse_pressed(QModelIndex)));

    connect(pdfinputfile_delegate, SIGNAL(data_edit()),
            this, SLOT(update_output_pages_count()));

    connect(generate_pdf_button, SIGNAL(released()),
            this, SLOT(generate_pdf_button_pressed()));

    connect(generate_pdf_action, SIGNAL(triggered(bool)),
            this, SLOT(generate_pdf_button_pressed()));
}

void MainWindow::add_pdf_files()
{
    QStringList selected = QFileDialog::getOpenFileNames(
                            this,
                            tr("Select one or more PDF files to open"),
                            m_settings->value("open_directory", "").toString(),
                            tr("PDF files (*.pdf)"));

    for (int i=0; i<selected.count(); i++)
    {
        PdfInfo pdf_info = PdfInfo(selected.at(i).toStdString());

        QStandardItem *item = new QStandardItem();

        item->setData(selected.at(i), FILE_PATH_ROLE);
        item->setData(pdf_info.width(), PAGE_WIDTH_ROLE);
        item->setData(pdf_info.height(), PAGE_HEIGHT_ROLE);
        item->setData(QString::fromStdString(pdf_info.paper_size()),
                      PAPER_SIZE_ROLE);
        item->setData(pdf_info.is_portrait(), IS_PORTRAIT_ROLE);
        item->setData(pdf_info.n_pages(), N_PAGES_ROLE);

        item->setData("", OUTPUT_PAGES_ROLE);
        item->setData(0, MULTIPAGE_ROLE);
        item->setData(0, ROTATION_ROLE);

        m_files_list_model->appendRow(item);
    }

    if (selected.size() > 0)
    {
        m_settings->setValue(
                    "open_directory",
                    QFileInfo(selected.at(0)).dir().absolutePath());
        this->update_output_pages_count();
    }
}

void MainWindow::move_up()
{
    QList<int> indexes = this->selected_indexes();

    if (indexes.size() > 0 && indexes.at(0) > 0)
    {
        QItemSelection sel;

        // Move items up
        for (int i : indexes)
        {
            QList<QStandardItem *> row = m_files_list_model->takeRow(i);
            m_files_list_model->insertRow(i - 1, row);

            sel.push_back(
                        QItemSelectionRange(
                            m_files_list_model->index(i - 1, 0)));
        }

        // Restore selection
        m_files_list_view->setCurrentIndex(sel.indexes().first());
        m_files_list_view->selectionModel()->select(
                    sel,
                    QItemSelectionModel::ClearAndSelect);
    }
}

void MainWindow::move_down()
{
    QList<int> indexes = this->selected_indexes();

    if (indexes.size() > 0 &&
            indexes.back() < m_files_list_model->rowCount() - 1)
    {
        QItemSelection sel;

        /* Qt >= 5.6
        // Move items down
        for (
             QList<int>::reverse_iterator it = indexes.rbegin();
             it != indexes.rend();
             ++it)
        {
            QList<QStandardItem *> row = m_files_list_model->takeRow(*it);
            m_files_list_model->insertRow(*it + 1, row);

            sel.push_back(QItemSelectionRange(
                              m_files_list_model->index(*it + 1, 0)));
        }*/

        for (int i = indexes.size() - 1; i >= 0; --i)
        {
            QList<QStandardItem *> row =
                    m_files_list_model->takeRow(indexes.at(i));
            m_files_list_model->insertRow(indexes.at(i) + 1, row);

            sel.push_back(
                        QItemSelectionRange(
                            m_files_list_model->index(indexes.at(i) + 1, 0)));
        }

        // Restore selection
        m_files_list_view->setCurrentIndex(sel.indexes().last());
        m_files_list_view->selectionModel()->select(
                    sel,
                    QItemSelectionModel::ClearAndSelect);
    }
}

void MainWindow::remove_pdf_file()
{
    QList<int> indexes = this->selected_indexes();

    for (int i=indexes.count() - 1; i >= 0; i--)
        m_files_list_model->removeRow(indexes[i]);

    this->update_output_pages_count();
}

void MainWindow::edit_menu_activated()
{
    QModelIndexList indexes =
            m_files_list_view->selectionModel()->selectedIndexes();

    if (indexes.count() == 1)
        m_files_list_view->edit(indexes.first());
    else
    {
        EditPdfEntryDialog dialog(m_custom_multipages,
                                  m_files_list_model,
                                  indexes);
        dialog.exec();

        this->update_output_pages_count();
    }
}

void MainWindow::view_menu_activated()
{
    QModelIndexList indexes =
            m_files_list_view->selectionModel()->selectedIndexes();
    for (int i=0; i < indexes.count(); i++)
    {
        QDesktopServices::openUrl(QString("file://") +
                                  m_files_list_model
                                  ->itemFromIndex(indexes[i])
                                  ->data(FILE_PATH_ROLE)
                                  .toString()
                                  .toStdString()
                                  .c_str()
                                  );
    }
}

void MainWindow::item_mouse_pressed(const QModelIndex &index) //eventfilter
{
    if (qApp->mouseButtons() == Qt::RightButton)
    {
        QList<int> indexes = this->selected_indexes();
        index.row();
    }
}

void MainWindow::update_output_pages_count()
{
    int pages_count = 0;
    bool output_pages_errors = false;

    for (int i = 0; i < m_files_list_model->rowCount(); i++)
    {
        QStandardItem *item = m_files_list_model->item(i);
        QString output_pages = item->data(OUTPUT_PAGES_ROLE).toString();
        int n_pages = item->data(N_PAGES_ROLE).toInt();

        int output_pages_count;
        std::vector<std::pair<int, int>> intervals;
        if (parse_output_pages_string(output_pages.toStdString(),
                                      n_pages,
                                      intervals,
                                      output_pages_count))
        {
            int mp_index = item->data(MULTIPAGE_ROLE).toInt();
            if (mp_index > 0)
            {
                Multipage mp;

                if (mp_index < CUSTOM_MULTIPAGE_INDEX)
                    mp = multipage_defaults[mp_index];
                else
                    mp = m_custom_multipages[mp_index];

                int subpages = mp.rows * mp.columns;

                if (output_pages_count % subpages > 0)
                    output_pages_count = output_pages_count / subpages + 1;
                else
                    output_pages_count = output_pages_count / subpages;
            }
        }
        else if (!output_pages_errors)
        {
            m_output_pages_error_index = i;
            output_pages_errors = true;
        }

        pages_count += output_pages_count;
    }

    if (!output_pages_errors)
        m_output_pages_error_index = -1;

    m_output_page_count->setText(tr("Output pages: %1").arg(pages_count));
}

void MainWindow::generate_pdf_button_pressed()
{
    // Check if there is at least one input file
    if (m_files_list_model->rowCount() == 0)
    {
        QMessageBox::critical(this,
                              tr("PDF generation error"),
                              tr("You must add at least one PDF file."));
        return;
    }

    if (m_output_pages_error_index > -1)
    {
        QString error_message(
                    tr("<p>Output pages of file <b>%1</b> are badly formatted. "
                       "Please make sure you complied with the following "
                       "rules:</p><ul>"
                       "<li>intervals of pages must be written indicating the "
                       "first page and the last page separated by a dash "
                       "(e.g. \"1-5\");</li>"
                       "<li>single pages and intervals of pages must be "
                       "separated by spaces, commas or both "
                       "(e.g. \"1, 2, 3, 5-10\" or \"1 2 3 5-10\");</li>"
                       "<li>all pages and intervals of pages must be between "
                       "1 and the number of pages of the PDF file;</li>"
                       "<li>only numbers, spaces, commas and dashes can be "
                       "used. All other characters are not allowed.</li>"
                       "</ul>").arg(m_files_list_model
                                    ->item(m_output_pages_error_index)
                                    ->data(FILE_PATH_ROLE).toString()));
        QMessageBox::critical(this,
                              tr("PDF generation error"),
                              error_message);
        return;
    }

    QString selected_file = QFileDialog::getSaveFileName(
                this,
                tr("Save PDF file"),
                m_settings->value("save_directory", "").toString(),
                tr("PDF files (*.pdf)"));

    if (!selected_file.isNull())
    {
        // Generate configuration
        Conf conf;

        conf.output_path = selected_file.toStdString();

        for (int i = 0; i < m_files_list_model->rowCount(); i++)
        {
            QStandardItem *item = m_files_list_model->item(i);
            QString file_path = item->data(FILE_PATH_ROLE).toString();
            QString output_pages = item->data(OUTPUT_PAGES_ROLE).toString();
            int mp_index = item->data(MULTIPAGE_ROLE).toInt();
            int rotation = item->data(ROTATION_ROLE).toInt();

            FileConf fileconf;
            fileconf.path = file_path.toStdString();
            fileconf.ouput_pages = output_pages.toStdString();
            if (mp_index > 0)
            {
                if (mp_index < CUSTOM_MULTIPAGE_INDEX)
                    fileconf.multipage = &multipage_defaults[mp_index];
                else
                    fileconf.multipage = &m_custom_multipages[mp_index];
            }
            fileconf.rotation = rotation;

            conf.files.push_back(fileconf);
        }

        QProgressBar *pb = m_progress_bar;
        std::function<void (int)> progress = [pb] (int p)
        {
            pb->setValue(p);
        };

        m_progress_bar->setValue(0);
        m_progress_bar->show();

        write_pdf(conf, progress);

        QTimer::singleShot(4000, m_progress_bar, SLOT(hide()));
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    m_settings->setValue("main_window_geometry", this->saveGeometry());

    // Save custom multipage profiles
    m_settings->beginGroup("custom_maltipage_profiles");

    for (QString key : m_settings->childKeys())
         m_settings->remove(key);

    QMap<int, Multipage>::const_iterator it;
    for (
         it = m_custom_multipages.constBegin();
         it != m_custom_multipages.constEnd();
         ++it)
        m_settings->setValue(
                    QString::number(it.key()),
                    QVariant::fromValue<Multipage>(it.value()));

    m_settings->endGroup();

    QMainWindow::closeEvent(event);
}


bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == m_files_list_view->viewport())
    {
        if (event->type() == QEvent::MouseButtonPress)
        {
            QMouseEvent *mouse_event = static_cast<QMouseEvent*>(event);
            if (mouse_event->button() == Qt::RightButton)
            {
                QList<int> indexes = this->selected_indexes();
                QModelIndex under_mouse =
                        m_files_list_view->indexAt(mouse_event->pos());

                if (!indexes.contains(under_mouse.row()))
                {
                    m_files_list_view->selectionModel()->select(
                                under_mouse,
                                QItemSelectionModel::ClearAndSelect);
                    m_files_list_view->setCurrentIndex(under_mouse);
                }

                m_edit_menu->exec(
                            m_files_list_view->
                            viewport()->
                            mapToGlobal(mouse_event->pos()));

                return true;
            }
        }
    }

    return QMainWindow::eventFilter(obj, event);
}

const QList<int> MainWindow::selected_indexes()
{
    QList<int> indexes;
    for(const QModelIndex &index :
        m_files_list_view->selectionModel()->selectedIndexes())
       indexes.append(index.row());

    std::sort(indexes.begin(), indexes.end());

    return indexes;
}
