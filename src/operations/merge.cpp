/* Copyright (C) 2021 Marco Scarpetta
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

#include "merge.h"

#include <QApplication>
#include <QToolBar>
#include <QFileDialog>
#include <QBoxLayout>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMessageBox>
#include <QDesktopServices>

#include "../gui_utils.h"
#include "../editpdfentrydialog.h"

Merge::Merge(QWidget *parent) :
    AbstractOperation(parent),
    m_triggered_new_profile{-1},
    m_output_pages_error_index(-1),
    m_files_list_view(new QListView(this)),
    m_files_list_model(new QStandardItemModel(this)),
    m_edit_menu(new QMenu(this))
{
    m_name = tr("Merge PDF files");
    m_icon = QIcon(":/icons/merge.svg");
    m_is_single_file_operation = false;

    // Create delegate for files list
    MouseEventFilter *filter = new MouseEventFilter(qApp);
    qApp->installEventFilter(filter);
    m_delegate =
            new InputPdfFileDelegate(
                filter,
                this);
    m_delegate->set_alternate_mix(false);

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
                    tr("PDF files") + " (*.pdf *.PDF)");
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
                       &Merge::load_files_list_pressed);
    m_save_files_list_action = toolbar->addAction(
                QIcon::fromTheme("document-save-as"),
                tr("Save files list"),
                this,
                &Merge::save_files_list_pressed);
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
    setLayout(v_layout);

    v_layout->addWidget(toolbar);
    v_layout->addWidget(m_files_list_view);
    QHBoxLayout *h_layout = new QHBoxLayout();
    h_layout->addItem(new QSpacerItem(
                          0, 0,
                          QSizePolicy::Expanding, QSizePolicy::Minimum));

    h_layout->addItem(new QSpacerItem(
                          0, 0,
                          QSizePolicy::Expanding, QSizePolicy::Minimum));

    h_layout->addWidget(m_generate_pdf_button);
    v_layout->addLayout(h_layout);

    // Connect signals to slots
    connect(m_delegate, SIGNAL(data_edit()),
            this, SLOT(update_output_pages_count()));

    connect(m_delegate, &InputPdfFileDelegate::trigger_new_profile,
            this, &Merge::new_profile_triggered);

    connect(m_generate_pdf_button, SIGNAL(released()),
            this, SLOT(generate_pdf_button_pressed()));

    connect(generate_pdf_action, SIGNAL(triggered(bool)),
            this, SLOT(generate_pdf_button_pressed()));
}

int Merge::output_pages_count()
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
            if (mp_index > -1)
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

    return pages_count;
}

void Merge::update_multipage_profiles()
{
    if (m_active)
        update_output_pages_count();
}

void Merge::profile_created(int index)
{
    if (m_active && m_triggered_new_profile > -1 && index > -1)
    {
        QStandardItem *item = m_files_list_model->item(m_triggered_new_profile);
        item->setData(index, MULTIPAGE_ROLE);
        m_triggered_new_profile = -1;
    }
}

void Merge::new_profile_triggered(int index)
{
    m_triggered_new_profile = index;
    emit trigger_new_profile();
}

bool Merge::load_json_files_list(const QString &filename)
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
    if (version.isUndefined() || version.toInt() < 0 || version.toInt() > 2)
        return false;

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

void Merge::load_files_list_pressed()
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

void Merge::save_files_list_pressed()
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
        data.insert("version", 2);

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

void Merge::add_pdf_files(const QStringList &files)
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

void Merge::move_up()
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

void Merge::move_down()
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

void Merge::remove_pdf_file()
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

void Merge::edit_menu_activated()
{
    QModelIndexList indexes =
            m_files_list_view->selectionModel()->selectedIndexes();

    if (indexes.count() == 1)
        m_files_list_view->edit(indexes.first());
    else
    {
        EditPdfEntryDialog dialog(m_files_list_model,
                                  indexes);
        dialog.exec();

        this->update_output_pages_count();
    }
}

void Merge::view_menu_activated()
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

void Merge::update_output_pages_count()
{
    emit output_pages_count_changed(output_pages_count());
}

void Merge::generate_pdf_button_pressed()
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
                tr("PDF files") + " (*.pdf *.PDF)");

    if (!selected_file.isNull())
    {
        emit write_started();

        settings->setValue(
                    "save_directory",
                    QFileInfo(selected_file).dir().absolutePath());

        try
        {
            write(selected_file);

            emit write_finished(selected_file);
        }
        catch (std::exception &e)
        {
            emit write_error(QString::fromStdString(e.what()));
        }
    }
}

void Merge::write(const QString &filename)
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

        emit progress_changed(100. * (i + 1) / (num_items + 1));
    }

    editor.write(filename.toStdString());
}

bool Merge::eventFilter(QObject *obj, QEvent *event)
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

    return QWidget::eventFilter(obj, event);
}

const QList<int> Merge::selected_indexes()
{
    QList<int> indexes;
    for(const QModelIndex &index :
        m_files_list_view->selectionModel()->selectedIndexes())
        indexes.append(index.row());

    std::sort(indexes.begin(), indexes.end());

    return indexes;
}
