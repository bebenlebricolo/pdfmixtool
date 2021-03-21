/* Copyright (C) 2017-2020 Marco Scarpetta
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
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QMenu>
#include <QToolBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QDesktopServices>
#include <QStackedWidget>
#include <QListWidget>
#include <QStatusBar>
#include <QRadioButton>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "aboutdialog.h"
#include "editpdfentrydialog.h"
#include "pdf_edit_lib/pdf_info.h"
#include "gui_utils.h"

MainWindow::MainWindow(MouseEventFilter *filter, QWidget *parent) :
    QMainWindow(parent),
    m_tab_widget(new QTabWidget(this)),
    m_alternate_mix(new QCheckBox(tr("Alternate mix"), this)),
    m_output_page_count(new QLabel(this)),
    m_output_pages_error_index(-1),
    m_progress_bar(new QProgressBar(this)),
    m_files_list_view(new QListView(this)),
    m_files_list_model(new QStandardItemModel(this)),
    m_edit_menu(new QMenu(this))
{
    // Main window properties
    this->setWindowIcon(QIcon(QString(ICON_PATH).arg(
                                  qApp->applicationDirPath())));
    this->setWindowTitle(qApp->applicationDisplayName());
    this->restoreGeometry(
                settings->value("main_window_geometry").toByteArray());

    // Delete profiles of old versions
    settings->beginGroup("custom_maltipage_profiles");
    settings->remove("");
    settings->endGroup();

    // Load custom multipage profiles
#if QT_VERSION < 0x060000
    qRegisterMetaTypeStreamOperators<Multipage>("Multipage");
#endif
    settings->beginGroup("maltipage_profiles");
    for (QString key : settings->childKeys())
        multipages[key.toInt()] =
                settings->value(key).value<Multipage>();
    settings->endGroup();

    if (multipages.size() == 0)
    {
        int i = 0;
        for (const Multipage &mp : multipage_defaults)
        {
            multipages[i] = mp;
            i++;
        }

    }

    // Create other windows
    m_multipage_profiles_manager = new MultipageProfilesManager(this);
    AboutDialog *about_dialog = new AboutDialog(new AboutDialog(this));

    // tab widget
    QVBoxLayout *main_layout = new QVBoxLayout();
    QWidget *multiple_mode = new QWidget(this);
    QWidget *single_mode = new QWidget(this);
    m_tab_widget->addTab(multiple_mode, tr("Multiple files"));
    m_tab_widget->addTab(single_mode, tr("Single file"));
    main_layout->addWidget(m_tab_widget);

    QStatusBar *m_status_bar = new QStatusBar(this);
    m_status_bar->addWidget(m_output_page_count);
    m_status_bar->addWidget(m_progress_bar, 100);
    m_status_bar->addWidget(new QWidget(this), 1);
    m_status_bar->addWidget(&m_saved_file);
    main_layout->addWidget(m_status_bar);

    QWidget *central_widget = new QWidget(this);
    this->setCentralWidget(central_widget);
    central_widget->setLayout(main_layout);

    // Hide progress bar and saved file label
    m_progress_bar->hide();
    m_saved_file.hide();
    m_saved_file.setOpenExternalLinks(true);

    // Create main menu and add actions
    QPushButton *main_menu_button = new QPushButton(
                QIcon::fromTheme("preferences-other"),
                tr("Menu"));
    main_menu_button->setDefault(true);
    QMenu *main_menu = new QMenu(main_menu_button);
    main_menu->addAction(
                QIcon::fromTheme("document-properties"),
                tr("Multipage profiles…"),
                m_multipage_profiles_manager,
                SLOT(show()));
    main_menu->addAction(
                QIcon::fromTheme("help-about"),
                tr("About"),
                about_dialog,
                SLOT(show()));
    main_menu->addAction(
                QIcon::fromTheme("application-exit"),
                tr("Exit"),
                this,
                SLOT(close()),
                QKeySequence::Quit);
    main_menu_button->setMenu(main_menu);
    m_tab_widget->setCornerWidget(main_menu_button);

    /// Multiple files mode
    // Create delegate for files list
    m_delegate =
            new InputPdfFileDelegate(
                filter,
                m_multipage_profiles_manager,
                this);

    // Set files list settings
    m_files_list_view->setWordWrap(false);
    m_files_list_view->setSelectionBehavior(QAbstractItemView::SelectItems);
    m_files_list_view->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_files_list_view->setEditTriggers(
                QAbstractItemView::DoubleClicked |
                QAbstractItemView::AnyKeyPressed);
    m_files_list_view->setModel(m_files_list_model);
    m_files_list_view->setItemDelegate(m_delegate);
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
                [=]() {
        QStringList files = QFileDialog::getOpenFileNames(
                    this,
                    tr("Select one or more PDF files to open"),
                    settings->value("open_directory", "").toString(),
                    tr("PDF files (*.pdf)"));
        add_pdf_files(files);
    });
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

    QWidget *spacer = new QWidget(this);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    spacer->setMinimumWidth(50);
    toolbar->addWidget(spacer);

    toolbar->addAction(QIcon::fromTheme("document-open"),
                       tr("Load files list"),
                       this,
                       &MainWindow::load_files_list_pressed);
    m_save_files_list_action = toolbar->addAction(
                QIcon::fromTheme("document-save-as"),
                tr("Save files list"),
                this,
                &MainWindow::save_files_list_pressed);
    m_save_files_list_action->setEnabled(false);

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

    // Create "Generate PDF" button
    m_generate_pdf_button = new QPushButton(
                QIcon::fromTheme("document-save-as"),
                tr("Generate PDF"),
                this);
    m_generate_pdf_button->setEnabled(false);
    QAction *generate_pdf_action = new QAction(
                tr("Generate PDF"),
                m_generate_pdf_button);
    generate_pdf_action->setShortcut(QKeySequence::Save);
    m_generate_pdf_button->addAction(generate_pdf_action);
    m_generate_pdf_button->setToolTip(
                QString(TOOLTIP_STRING)
                .arg(
                    m_generate_pdf_button->text(),
                    generate_pdf_action->shortcut().toString()));

    // Add widgets to the main window
    QVBoxLayout *v_layout = new QVBoxLayout();
    multiple_mode->setLayout(v_layout);

    v_layout->addWidget(toolbar);
    v_layout->addWidget(m_files_list_view);
    QHBoxLayout *h_layout = new QHBoxLayout();
    h_layout->addWidget(m_alternate_mix);
    h_layout->addItem(new QSpacerItem(
                          0, 0,
                          QSizePolicy::Expanding, QSizePolicy::Minimum));

    h_layout->addItem(new QSpacerItem(
                          0, 0,
                          QSizePolicy::Expanding, QSizePolicy::Minimum));

    h_layout->addWidget(m_generate_pdf_button);
    v_layout->addLayout(h_layout);

    // Connect signals to slots
    connect(m_tab_widget, &QTabWidget::currentChanged,
            this, &MainWindow::current_tab_changed);

    connect(m_files_list_view, SIGNAL(pressed(QModelIndex)),
            this, SLOT(item_mouse_pressed(QModelIndex)));

    connect(m_alternate_mix, &QCheckBox::toggled,
            this, &MainWindow::alternate_mix_checked);

    connect(m_delegate, SIGNAL(data_edit()),
            this, SLOT(update_output_pages_count()));

    connect(m_multipage_profiles_manager, SIGNAL(close_signal()),
            this, SLOT(update_output_pages_count()));

    connect(m_generate_pdf_button, SIGNAL(released()),
            this, SLOT(generate_pdf_button_pressed()));

    connect(generate_pdf_action, SIGNAL(triggered(bool)),
            this, SLOT(generate_pdf_button_pressed()));

    /// Single PDF file mode
    // opened PDF file line
    v_layout = new QVBoxLayout();
    single_mode->setLayout(v_layout);
    h_layout = new QHBoxLayout();
    v_layout->addLayout(h_layout);

    QPushButton *open_button = new QPushButton(
                QIcon::fromTheme("document-open"), tr("Open PDF file…"), this);
    open_button->setShortcut(QKeySequence::Open);
    open_button->setToolTip(
                QString(TOOLTIP_STRING)
                .arg(
                    open_button->text(),
                    open_button->shortcut().toString()));
    h_layout->addWidget(open_button);

    connect(open_button, &QPushButton::pressed,
            this, &MainWindow::open_file_pressed);

    m_opened_file_label = new PdfInfoLabel(this);
    h_layout->addWidget(m_opened_file_label);
    h_layout->setStretch(1, 1);

    m_view_opened_pdf_button = new QPushButton(
                QIcon::fromTheme("document-print-preview"), "", this);
    m_view_opened_pdf_button->setToolTip("View opened PDF file");
    h_layout->addWidget(m_view_opened_pdf_button);
    m_view_opened_pdf_button->setEnabled(false);
    connect(m_view_opened_pdf_button, &QPushButton::pressed,
            [=]() {
        QDesktopServices::openUrl(QString("file://") +
                                  m_opened_pdf_info.filename().c_str());});

    // operations UI
    v_layout->addWidget(&m_operations_splitter);
    QListWidget *operations_list = new QListWidget(this);
    QStackedWidget *operations_widget = new QStackedWidget(this);

    m_operations.push_back(new Booklet(m_opened_pdf_info, this));
    EditPageLayout *edit_page_layout = new EditPageLayout(m_opened_pdf_info,
                                                          this);
    edit_page_layout->update_multipage_profiles();
    connect(m_multipage_profiles_manager,
            &MultipageProfilesManager::close_signal,
            edit_page_layout,
            &EditPageLayout::update_multipage_profiles);
    connect(edit_page_layout,
            &EditPageLayout::trigger_new_profile,
            m_multipage_profiles_manager,
            &MultipageProfilesManager::new_profile_button_pressed);
    connect(m_multipage_profiles_manager,
            &MultipageProfilesManager::profile_created,
            edit_page_layout,
            &EditPageLayout::profile_created);
    m_operations.push_back(edit_page_layout);
    m_operations.push_back(new AddEmptyPages(m_opened_pdf_info, this));
    m_operations.push_back(new DeletePages(m_opened_pdf_info, this));
    m_operations.push_back(new ExtractPages(m_opened_pdf_info, this));

    for (AbstractOperation *operation : m_operations)
    {
        operations_list->addItem(operation->name());
        operations_widget->addWidget(operation);
        connect(operation, &AbstractOperation::write_started,
                this, &MainWindow::write_started);
        connect(operation, &AbstractOperation::progress_changed,
                this, &MainWindow::update_progress);
        connect(operation, &AbstractOperation::write_finished,
                this, &MainWindow::write_finished);
    }

    m_operations_splitter.addWidget(operations_list);
    m_operations_splitter.addWidget(operations_widget);
    m_operations_splitter.setOrientation(Qt::Horizontal);
    m_operations_splitter.setEnabled(false);
    m_operations_splitter.setChildrenCollapsible(false);
    m_operations_splitter.setHandleWidth(10);

    if (settings->contains("operations_list_width") &&
            settings->contains("operations_widget_width"))
    {
        QList<int> sizes;
        sizes.push_back(settings->value("operations_list_width").toInt());
        sizes.push_back(settings->value("operations_widget_width").toInt());
        m_operations_splitter.setSizes(sizes);
    }

    operations_list->setCurrentRow(0);

    connect(operations_list, &QListWidget::currentRowChanged,
            operations_widget, &QStackedWidget::setCurrentIndex);

    // register whether the splitter have been moved at least once
    m_operations_splitter_moved = false;
    QMetaObject::Connection * const connection = new QMetaObject::Connection;
    *connection = connect(&m_operations_splitter, &QSplitter::splitterMoved,
                          [this, connection](){
        m_operations_splitter_moved = true;
        QObject::disconnect(*connection);
        delete connection;
    });
}

bool MainWindow::load_json_files_list(const QString &filename)
{
    QFile json;
    json.setFileName(filename);
    json.open(QIODevice::ReadOnly);
    QJsonDocument doc = QJsonDocument::fromJson(json.readAll());
    json.close();

    if (doc.isNull() || doc.isArray())
        return false;

    QJsonObject data = doc.object();
    QJsonValue version = data.value("version");
    if (version.isUndefined() || version.toInt() != 1)
        return false;

    QJsonValue alternate_mix = data.value("alternate_mix");
    if (alternate_mix.isUndefined())
        return false;
    m_alternate_mix->setChecked(alternate_mix.toBool());

    QJsonValue files = data.value("files");
    if (!files.isArray())
        return false;

    m_files_list_model->clear();

    for (QJsonValue file : files.toArray())
    {
        if (!file.isObject())
            return false;

        QJsonObject file_obj = file.toObject();

        try
        {
            add_pdf_files(QStringList(file_obj.value("path").toString()));
        }
        catch(...)
        {
            return false;
        }

        QStandardItem *item = m_files_list_model->item(
                    m_files_list_model->rowCount() - 1);
        item->setData(file_obj.value("pages").toString(), OUTPUT_PAGES_ROLE);

        int mp_index = file_obj.value("multipage").toInt(-1);
        if (mp_index >= multipages.size())
            mp_index = -1;
        item->setData(mp_index, MULTIPAGE_ROLE);

        item->setData(file_obj.value("rotation").toInt(), ROTATION_ROLE);
        item->setData(file_obj.value("outline").toString(), OUTLINE_ENTRY_ROLE);
        item->setData(file_obj.value("reverse").toBool(), REVERSE_ORDER_ROLE);
    }

    return true;
}

void MainWindow::load_files_list_pressed()
{
    QString filename = QFileDialog::getOpenFileName(
                this,
                tr("Select the JSON file containing the files list"),
                settings->value("open_directory", "").toString(),
                tr("JSON files (*.json)"));

    if (!filename.isNull())
        if (!load_json_files_list(filename))
            QMessageBox::critical(
                        this,
                        tr("Error while reading the JSON file!"),
                        tr("An error occurred while reading the JSON file!"));
}

void MainWindow::save_files_list_pressed()
{
    QString filename = QFileDialog::getSaveFileName(
                this,
                tr("Select a JSON file"),
                settings->value("save_directory",
                                settings->value("open_directory", "")
                                ).toString(),
                tr("JSON files (*.json)"));

    if (!filename.isNull())
    {
        QJsonObject data;
        data.insert("version", 1);
        data.insert("alternate_mix", m_alternate_mix->isChecked());

        QJsonArray files;
        for (int i = 0; i < m_files_list_model->rowCount(); i++)
        {
            QStandardItem *item = m_files_list_model->item(i);
            QString path = item->data(FILE_PATH_ROLE).toString();
            QString output_pages = item->data(OUTPUT_PAGES_ROLE).toString();
            int mp_index = item->data(MULTIPAGE_ROLE).toInt();
            int rotation = item->data(ROTATION_ROLE).toInt();
            QString outline = item->data(OUTLINE_ENTRY_ROLE).toString();
            bool reverse = item->data(REVERSE_ORDER_ROLE).toBool();

            QJsonObject file_data;
            file_data.insert("path", path);
            file_data.insert("pages", output_pages);
            file_data.insert("multipage", mp_index);
            file_data.insert("rotation", rotation);
            file_data.insert("outline", outline);
            file_data.insert("reverse", reverse);

            files.push_back(file_data);
        }

        data.insert("files", files);

        QJsonDocument doc;

        doc.setObject(data);

        QFile json;
        json.setFileName(filename);
        json.open(QIODevice::WriteOnly);
        json.write(doc.toJson());
        json.close();
    }
}

void MainWindow::set_input_files(const QStringList &files)
{
    if (files.size() == 0)
        return;

    settings->setValue(
                "open_directory",
                QFileInfo(files[0]).dir().absolutePath());

    if (files.size() == 1)
    {
        m_tab_widget->setCurrentIndex(1);
        this->update_opened_file_label(files[0]);
        m_view_opened_pdf_button->setEnabled(true);
    }
    else
    {
        this->add_pdf_files(files);
    }
}

void MainWindow::current_tab_changed(int index)
{
    if (index == 0)
        update_output_pages_count();
    else
        m_output_page_count->hide();
}

void MainWindow::add_pdf_files(const QStringList &files)
{
    for (int i=0; i<files.count(); i++)
    {
        try
        {
            PdfInfo pdf_info = PdfInfo(files.at(i).toStdString());
            QString filename = QUrl(files.at(i)).fileName();
            if (filename.endsWith(".pdf", Qt::CaseInsensitive))
                filename.chop(4);

            QStandardItem *item = new QStandardItem();

            item->setData(files.at(i), FILE_PATH_ROLE);
            item->setData(pdf_info.width(), PAGE_WIDTH_ROLE);
            item->setData(pdf_info.height(), PAGE_HEIGHT_ROLE);
            item->setData(QString::fromStdString(pdf_info.paper_size()),
                        PAPER_SIZE_ROLE);
            item->setData(pdf_info.is_portrait(), IS_PORTRAIT_ROLE);
            item->setData(pdf_info.n_pages(), N_PAGES_ROLE);

            item->setData("", OUTPUT_PAGES_ROLE);
            item->setData(-1, MULTIPAGE_ROLE);
            item->setData(0, ROTATION_ROLE);
            item->setData(filename, OUTLINE_ENTRY_ROLE);
            item->setData(false, REVERSE_ORDER_ROLE);

            m_files_list_model->appendRow(item);
        }    
        catch (std::exception& e)
        {
            QMessageBox::critical(this,
                                  tr("Error opening file"),
                                  QString::fromStdString(e.what()));
        }
    }

    if (files.size() > 0)
    {
        settings->setValue(
                    "open_directory",
                    QFileInfo(files.at(0)).dir().absolutePath());

        this->update_output_pages_count();
        m_generate_pdf_button->setEnabled(true);
        m_save_files_list_action->setEnabled(true);
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

    if (m_files_list_model->rowCount() == 0)
    {
        m_generate_pdf_button->setEnabled(false);
        m_save_files_list_action->setEnabled(false);
    }
}

void MainWindow::edit_menu_activated()
{
    QModelIndexList indexes =
            m_files_list_view->selectionModel()->selectedIndexes();

    if (indexes.count() == 1 || m_alternate_mix->isChecked())
        m_files_list_view->edit(indexes.first());
    else
    {
        EditPdfEntryDialog dialog(m_files_list_model,
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

void MainWindow::alternate_mix_checked(bool checked)
{
    m_delegate->set_alternate_mix(checked);
    m_files_list_model->layoutChanged();
    m_files_list_view->viewport()->repaint();
    update_output_pages_count();
}

void MainWindow::update_output_pages_count()
{
    if (m_files_list_model->rowCount() == 0)
    {
        m_output_page_count->hide();
        return;
    }

    int pages_count = 0;

    if (m_alternate_mix->isChecked())
    {
        for (int i = 0; i < m_files_list_model->rowCount(); i++)
        {
            QStandardItem *item = m_files_list_model->item(i);
            pages_count += item->data(N_PAGES_ROLE).toInt();
        }

    }
    else
    {
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
                    if (multipages.find(mp_index) == multipages.end())
                        item->setData(-1, MULTIPAGE_ROLE);
                    else
                    {
                        Multipage mp = multipages[mp_index];

                        int subpages = mp.rows * mp.columns;

                        if (output_pages_count % subpages > 0)
                            output_pages_count = output_pages_count / subpages
                                    + 1;
                        else
                            output_pages_count = output_pages_count / subpages;
                    }
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
    }

    m_output_page_count->setText(tr("Output pages: %1").arg(pages_count));
    m_output_page_count->show();
}

void MainWindow::generate_pdf_button_pressed()
{
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
                settings->value("save_directory",
                                  settings->value("open_directory", "")
                                  ).toString(),
                tr("PDF files (*.pdf)"));

    if (!selected_file.isNull())
    {
        write_started();

        settings->setValue(
                    "save_directory",
                    QFileInfo(selected_file).dir().absolutePath());

        // alternate mix
        if (m_alternate_mix->isChecked())
        {
            std::vector<std::string> input_filenames;

            std::vector<int> num_pages{};
            std::vector<int> file_ids{};
            std::vector<bool> reverse_order;
            PdfEditor editor;

            // add files to editor
            for (int i = 0; i < m_files_list_model->rowCount(); i++)
            {
                QStandardItem *item = m_files_list_model->item(i);
                QString file_path = item->data(FILE_PATH_ROLE).toString();
                num_pages.push_back(item->data(N_PAGES_ROLE).toInt());
                file_ids.push_back(editor.add_file(file_path.toStdString()));
                reverse_order.push_back(
                            item->data(REVERSE_ORDER_ROLE).toBool());
            }

            int max_num_pages = *std::max_element(num_pages.begin(),
                                                  num_pages.end());

            // alternatively add pages
            for (int i{0}; i < max_num_pages; ++i)
            {
                for (size_t j{0}; j < file_ids.size(); ++j)
                {
                    if (i < num_pages[j])
                    {
                        int index = reverse_order[j] ? num_pages[j] - 1 - i : i;
                        editor.add_pages(file_ids[j], 0, nullptr,
                                         {{index, index}});
                    }
                }

                update_progress(100. * (i + 1) / (max_num_pages + 1));
            }

            editor.write(selected_file.toStdString());
        }

        // standard mode
        else
        {
            PdfEditor editor;

            int num_items{m_files_list_model->rowCount()};

            for (int i{0}; i < num_items; i++)
            {
                QStandardItem *item = m_files_list_model->item(i);

                QString file_path = item->data(FILE_PATH_ROLE).toString();
                QString output_pages = item->data(OUTPUT_PAGES_ROLE).toString();
                int mp_index = item->data(MULTIPAGE_ROLE).toInt();
                int rotation = item->data(ROTATION_ROLE).toInt();
                QString outline = item->data(OUTLINE_ENTRY_ROLE).toString();
                int n_pages = item->data(N_PAGES_ROLE).toInt();

                int output_pages_count;

                std::vector<std::pair<int, int>> intervals;
                parse_output_pages_string(output_pages.toStdString(),
                                                     n_pages,
                                                     intervals,
                                                     output_pages_count);

                unsigned int id = editor.add_file(file_path.toStdString());

                PdfEditor::PageLayout *page_layout{nullptr};
                if (mp_index >= 0)
                    page_layout = new PdfEditor::PageLayout(multipages[mp_index]);

                editor.add_pages(id, rotation, page_layout, intervals,
                                 outline.toStdString());

                update_progress(100. * (i + 1) / (num_items + 1));
            }

            editor.write(selected_file.toStdString());
        }

        write_finished(selected_file);
    }
}

void MainWindow::open_file_pressed()
{
    try
    {
        QString filename = QFileDialog::getOpenFileName(
                    this,
                    tr("Select a PDF file"),
                    settings->value("open_directory", "").toString(),
                    tr("PDF files (*.pdf)"));

        if (!filename.isNull())
        {
            settings->setValue(
                        "open_directory",
                        QFileInfo(filename).dir().absolutePath());

            this->update_opened_file_label(filename);
            m_view_opened_pdf_button->setEnabled(true);
        }
    }
    catch (std::exception& e)
    {
        QMessageBox::critical(this,
                              tr("Error opening file"),
                              QString::fromStdString(e.what()));
    }
}

void MainWindow::update_opened_file_label(const QString &filename)
{
    m_opened_pdf_info = PdfInfo(filename.toStdString());

    m_operations_splitter.setEnabled(true);

    m_opened_file_label->set_pdf_info(m_opened_pdf_info);

    for (AbstractOperation *operation : m_operations)
        operation->pdf_info_changed();
}

void MainWindow::write_started()
{
    m_progress_bar->setValue(0);
    m_progress_bar->show();
}

void MainWindow::update_progress(int progress)
{
    m_progress_bar->setValue(progress);
}

void MainWindow::write_finished(const QString &filename)
{
    m_progress_bar->setValue(100);

    if (filename == QString::fromStdString(m_opened_pdf_info.filename()))
        update_opened_file_label(filename);

    QTimer::singleShot(2000, m_progress_bar, SLOT(hide()));

    QFileInfo info(filename);
    if (info.isDir())
    {
        QString file_link = QString("<a href=\"file://%1\">%2</a>").arg(
                    filename, info.fileName());
        m_saved_file.setText(tr("Files saved in %1.").arg(file_link));
    }
    else
    {
        QString file_link = QString("<a href=\"file://%1\">%2</a>").arg(
                    filename, info.fileName());
        m_saved_file.setText(tr("File %1 saved.").arg(file_link));
    }

    m_saved_file.show();

    QTimer::singleShot(6000, &m_saved_file, SLOT(hide()));
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    // save operations splitter sizes only if the splitter was moved to
    // prevent saving default values
    if (m_operations_splitter_moved)
    {
        QList<int> sizes = m_operations_splitter.sizes();
        settings->setValue("operations_list_width", sizes[0]);
        settings->setValue("operations_widget_width", sizes[1]);
    }

    settings->setValue("main_window_geometry", this->saveGeometry());

    // Save custom multipage profiles
    settings->beginGroup("maltipage_profiles");

    for (QString key : settings->childKeys())
        settings->remove(key);

    QMap<int, Multipage>::const_iterator it;
    for (it = multipages.constBegin();
         it != multipages.constEnd();
         ++it)
        settings->setValue(
                    QString::number(it.key()),
                    QVariant::fromValue<Multipage>(it.value()));

    settings->endGroup();

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
