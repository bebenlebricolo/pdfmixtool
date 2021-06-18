/* Copyright (C) 2017-2021 Marco Scarpetta
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

#include "aboutdialog.h"
#include "editpdfentrydialog.h"
#include "pdf_edit_lib/pdf_info.h"
#include "gui_utils.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_output_page_count(new QLabel(this)),
    m_progress_bar(new QProgressBar(this)),
    m_operations_list(new QListWidget(this))
{
    // Main window properties
    this->setWindowIcon(QIcon(":/icons/icon.svg"));
    this->setWindowTitle(qApp->applicationDisplayName());
    this->restoreGeometry(
                settings->value("main_window_geometry").toByteArray());

    // Load custom multipage profiles
    settings->beginGroup("multipage_profiles");
    for (QString key : settings->childKeys())
        multipages[key.toInt()] = settings->value(key).value<Multipage>();
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
    AboutDialog *about_dialog = new AboutDialog(this);

    // main layout
    QHBoxLayout *splitter_layout = new QHBoxLayout();

    QWidget *central_widget = new QWidget(this);
    this->setCentralWidget(central_widget);
    central_widget->setLayout(splitter_layout);

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

    // left side
    QWidget *left_side = new QWidget(this);
    QVBoxLayout *v_layout = new QVBoxLayout();
    left_side->setLayout(v_layout);
    v_layout->addWidget(main_menu_button, 0, Qt::AlignLeft);
    v_layout->addWidget(m_operations_list, 1);

    m_operations.push_back(new Merge(this));
    m_operations.push_back(new ExtractPages(this));
    m_operations.push_back(new DeletePages(this));
    m_operations.push_back(new Rotate(this));
    m_operations.push_back(new EditPageLayout(this));
    m_operations.push_back(new EditDocumentInfo(this));
    m_operations.push_back(new AlternateMix(this));
    m_operations.push_back(new Booklet(this));
    m_operations.push_back(new AddEmptyPages(this));

    for (AbstractOperation *operation : m_operations)
    {
        QListWidgetItem *item = new QListWidgetItem();
        QWidget *item_widget = new QWidget(this);
        QVBoxLayout *item_widget_layout = new QVBoxLayout();
        item_widget->setLayout(item_widget_layout);
        QLabel *icon_label = new QLabel(item_widget);
        icon_label->setPixmap(operation->icon().pixmap(128, 64));
        item_widget_layout->addWidget(icon_label, 0, Qt::AlignCenter);
        QLabel *item_label = new QLabel(operation->name(), item_widget);
        item_widget_layout->addWidget(item_label, 0, Qt::AlignCenter);
        m_operations_list->addItem(item);
        item->setSizeHint(item_widget->sizeHint());
        m_operations_list->setItemWidget(item, item_widget);
        m_operations_widget.addWidget(operation);
        connect(operation, &AbstractOperation::write_started,
                this, &MainWindow::write_started);
        connect(operation, &AbstractOperation::progress_changed,
                this, &MainWindow::update_progress);
        connect(operation, &AbstractOperation::write_finished,
                this, &MainWindow::write_finished);
        connect(operation, &AbstractOperation::write_error,
                this, &MainWindow::write_error);
        connect(operation, &AbstractOperation::output_pages_count_changed,
                this, &MainWindow::update_output_pages_count);
        operation->update_multipage_profiles();
        connect(m_multipage_profiles_manager,
                &MultipageProfilesManager::close_signal,
                operation,
                &AbstractOperation::update_multipage_profiles);
        connect(operation,
                &AbstractOperation::trigger_new_profile,
                m_multipage_profiles_manager,
                &MultipageProfilesManager::new_profile_button_pressed);
        connect(m_multipage_profiles_manager,
                &MultipageProfilesManager::profile_created,
                operation,
                &AbstractOperation::profile_created);
    }

    connect(m_operations_list, &QListWidget::currentRowChanged,
            this, &MainWindow::operation_changed);

    m_active_operation = m_operations.first();
    m_active_operation->set_active(true);
    m_operations_list->setCurrentRow(0);
    m_operations_list->setFixedWidth(
                m_operations_list->sizeHintForColumn(0) + 64);
    m_operations_list->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

    // right side
    QWidget *right_side = new QWidget(this);
    v_layout = new QVBoxLayout();
    right_side->setLayout(v_layout);
    v_layout->addWidget(&m_open_pdf_widget);
    QGridLayout *open_pdf_layout = new QGridLayout();
    m_open_pdf_widget.setLayout(open_pdf_layout);

    QPushButton *open_button = new QPushButton(
                QIcon::fromTheme("document-open"), tr("Open PDF file…"), this);
    open_button->setShortcut(QKeySequence::Open);
    open_button->setToolTip(
                QString(TOOLTIP_STRING)
                .arg(
                    open_button->text(),
                    open_button->shortcut().toString()));
    open_pdf_layout->addWidget(open_button, 1, 1);

    connect(open_button, &QPushButton::pressed,
            this, &MainWindow::open_file_pressed);

    m_opened_file_label = new PdfInfoLabel(this);
    open_pdf_layout->addWidget(m_opened_file_label, 1, 2);
    open_pdf_layout->setColumnStretch(2, 1);

    m_view_opened_pdf_button = new QPushButton(
                QIcon::fromTheme("document-print-preview"), "", this);
    m_view_opened_pdf_button->setToolTip("View opened PDF file");
    open_pdf_layout->addWidget(m_view_opened_pdf_button, 1, 3);
    m_view_opened_pdf_button->setEnabled(false);
    connect(m_view_opened_pdf_button, &QPushButton::pressed,
            [=]() {
        QDesktopServices::openUrl(QString("file://") +
                                  m_opened_pdf_info.filename().c_str());});


    QFrame *separator = new QFrame(this);
    separator->setFrameStyle(QFrame::HLine | QFrame::Plain);
    separator->setMinimumHeight(20);
    open_pdf_layout->addWidget(separator, 2, 1, 1, 3);

    v_layout->addWidget(&m_operations_widget);

    separator = new QFrame(this);
    separator->setFrameStyle(QFrame::HLine | QFrame::Plain);
    separator->setContentsMargins(10, 0, 10, 0);
    v_layout->addWidget(separator);

    QStatusBar *m_status_bar = new QStatusBar(this);
    m_status_bar->addWidget(m_output_page_count);
    m_status_bar->addWidget(m_progress_bar, 100);
    m_status_bar->addWidget(new QWidget(this), 1);
    m_status_bar->addWidget(&m_saved_file);
    v_layout->addWidget(m_status_bar);

    // splitter: left + right
    splitter_layout->addWidget(left_side);
    splitter_layout->addWidget(right_side, 1);
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
        m_operations_list->setCurrentRow(1);
        this->update_opened_file_label(files[0]);
        m_view_opened_pdf_button->setEnabled(true);
    }
    else
    {
        static_cast<Merge *>(m_operations.at(0))->add_pdf_files(files);
    }
}

void MainWindow::operation_changed(int index)
{
//    for (int i{0}; i < m_operations_list->count(); ++i)
//        m_operations_list->itemWidget(m_operations_list->item(i))
//                ->findChildren<QLabel *>().at(1)
//                ->setForegroundRole(QPalette::Text);
//    m_operations_list->itemWidget(m_operations_list->item(index))
//            ->findChildren<QLabel *>().at(1)
//            ->setForegroundRole(QPalette::HighlightedText);

    m_active_operation->set_active(false);
    m_active_operation = m_operations.at(index);
    m_active_operation->set_active(true);

    if (m_active_operation->is_single_file_operation())
    {
        if (m_opened_pdf_info.filename() == "")
            m_operations_widget.setDisabled(true);
        else
            m_active_operation->set_pdf_info(m_opened_pdf_info);

        m_open_pdf_widget.show();
    }
    else
    {
        m_operations_widget.setDisabled(false);
        m_open_pdf_widget.hide();
    }

    update_output_pages_count(m_operations.at(index)->output_pages_count());

    m_operations_widget.setCurrentIndex(index);
}

void MainWindow::open_file_pressed()
{
    try
    {
        QString filename = QFileDialog::getOpenFileName(
                    this,
                    tr("Select a PDF file"),
                    settings->value("open_directory", "").toString(),
                    tr("PDF files") + " (*.pdf *.PDF)");

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

    m_operations_widget.setEnabled(true);

    m_opened_file_label->set_pdf_info(m_opened_pdf_info);

    if (m_active_operation->is_single_file_operation())
        m_active_operation->set_pdf_info(m_opened_pdf_info);
}

void MainWindow::update_output_pages_count(int count)
{
    if (count <= 0)
    {
        m_output_page_count->hide();
    }
    else
    {
        m_output_page_count->setText(tr("Output pages: %1").arg(count));
        m_output_page_count->show();
    }
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

void MainWindow::write_error(const QString &error)
{
    QMessageBox::critical(this, tr("Error generating the PDF"), error);

    m_progress_bar->hide();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    settings->setValue("main_window_geometry", this->saveGeometry());

    // Save custom multipage profiles
    settings->beginGroup("multipage_profiles");

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
