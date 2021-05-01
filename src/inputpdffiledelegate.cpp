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

#include "inputpdffiledelegate.h"

#include <QPainter>
#include <QFileInfo>
#include <QDir>
#include <QLabel>
#include <QGridLayout>
#include <QCoreApplication>

#include "gui_utils.h"
#include "widgets/pdfinfolabel.h"
#include "widgets/output_preview.h"

InputPdfFileDelegate::InputPdfFileDelegate(
        MouseEventFilter *filter,
        QWidget *parent) :
    QStyledItemDelegate(parent),
    m_mouse_event_filter(filter),
    m_alternate_mix(false)
{

}

QWidget *InputPdfFileDelegate::build_widget(
        const QModelIndex &index,
        int width,
        int height) const
{
    QWidget *main_widget = new QWidget();
    main_widget->setFixedSize(width, height);
    main_widget->setLayout(new QHBoxLayout());
    main_widget->layout()->setContentsMargins(2, 2, 2, 2);

    OutputPreview *output_preview = new OutputPreview(main_widget);
    main_widget->layout()->addWidget(output_preview);

    QWidget *widget = new QWidget(main_widget);
    QGridLayout *grid_layout = new QGridLayout(widget);

    QString file_path = index.data(FILE_PATH_ROLE).toString();
    double page_width = index.data(PAGE_WIDTH_ROLE).toDouble();
    double page_height = index.data(PAGE_HEIGHT_ROLE).toDouble();
    QString paper_size = index.data(PAPER_SIZE_ROLE).toString();
    bool is_portrait = index.data(IS_PORTRAIT_ROLE).toBool();
    int n_pages = index.data(N_PAGES_ROLE).toInt();

    PdfInfoLabel *file_info_label = new PdfInfoLabel(widget);
    file_info_label->set_pdf_info(file_path,
                                  page_width,
                                  page_height,
                                  paper_size,
                                  is_portrait,
                                  n_pages);

    grid_layout->addWidget(file_info_label, 0, 1, 1, 3);

    main_widget->layout()->addWidget(widget);

    if (m_alternate_mix)
    {
        bool reverse_order = index.data(REVERSE_ORDER_ROLE).toBool();

        QString order_text = QCoreApplication::translate(
                    "InputPdfFileDelegate",
                    "Page order:") + ' ' + (reverse_order ?
                    QCoreApplication::translate("InputPdfFileDelegate",
                                                "reverse") :
                    QCoreApplication::translate("InputPdfFileDelegate",
                                                "forward"));

        QLabel *order_label = new QLabel(order_text, widget);

        grid_layout->addItem(new QSpacerItem(0, 0), 1, 1);
        grid_layout->addWidget(order_label, 1, 2);

        if (height > 0)
        {
            double size = height - 4;
            output_preview->setFixedSize(size, size);
            output_preview->set_page_size(page_width, page_height);
        }
    }
    else
    {
        QString output_pages = index.data(OUTPUT_PAGES_ROLE).toString();
        int mp_index = index.data(MULTIPAGE_ROLE).toInt();
        int rotation = index.data(ROTATION_ROLE).toInt();
        QString outline_entry = index.data(OUTLINE_ENTRY_ROLE).toString();

        if (output_pages.size() == 0)
            output_pages = QCoreApplication::translate("InputPdfFileDelegate",
                                                       "All");

        bool mp_enabled;
        Multipage mp;
        if (mp_index < 0)
            mp_enabled = false;
        else
        {
            mp_enabled = true;
            mp = multipages[mp_index];
        }

        QString pages = QCoreApplication::translate(
                    "InputPdfFileDelegate",
                    "Pages:") + ' ' + output_pages;
        QString multipage = QCoreApplication::translate(
                    "InputPdfFileDelegate",
                    "Multipage:") + ' ' + (
                    mp_index >= 0 ?
                        QString(" %1").arg(QString::fromStdString(mp.name)) :
                        QCoreApplication::translate(
                            "InputPdfFileDelegate",
                            "Disabled"));
        QString rotation_text = QCoreApplication::translate(
                    "InputPdfFileDelegate",
                    "Rotation:") + QString(" %1°").arg(rotation);
        QString outline_entry_text = QCoreApplication::translate(
                    "InputPdfFileDelegate",
                    "Outline entry:") + ' ' + outline_entry;

        QLabel *pages_label = new QLabel(pages, widget);
        QLabel *multipage_label = new QLabel(multipage, widget);
        QLabel *rotation_label = new QLabel(rotation_text, widget);
        QLabel *outline_label = new QLabel(outline_entry_text, widget);

        grid_layout->addItem(new QSpacerItem(0, 0), 1, 1);
        grid_layout->addWidget(pages_label, 1, 2);
        grid_layout->addWidget(multipage_label, 1, 3);
        grid_layout->addItem(new QSpacerItem(0, 0), 2, 1);
        grid_layout->addWidget(rotation_label, 2, 2);
        grid_layout->addWidget(outline_label, 2, 3);

        if (height > 0)
        {
            double size = height - 4;
            output_preview->setFixedSize(size, size);
            output_preview->set_page_size(page_width, page_height);
            output_preview->set_rotation(rotation);
            output_preview->set_multipage(mp);
            output_preview->set_multipage_enabled(mp_enabled);
        }
    }

    main_widget->setContentsMargins(0, 0, 0, 0);
    widget->setContentsMargins(10, 5, 10, 5);
    output_preview->setContentsMargins(0, 0, 0, 0);
    grid_layout->setColumnStretch(3, 1);
    grid_layout->setHorizontalSpacing(30);
    grid_layout->setVerticalSpacing(5);

    return main_widget;
}

void InputPdfFileDelegate::paint(
        QPainter *painter,
        const QStyleOptionViewItem &option,
        const QModelIndex &index) const
{
    // Draw border
    QPen pen;

    if (option.state & QStyle::State_MouseOver)
        pen.setBrush(option.palette.highlight());
    else
        pen.setBrush(option.palette.mid());

    painter->setPen(pen);

    QRect border = option.rect - QMargins(0, 0, 1, 1);
    painter->drawRect(border);

    // Draw background
    border -= QMargins(1, 1, 0, 0);

    if (option.state & QStyle::State_Selected)
        painter->fillRect(border, option.palette.highlight());
    else if (option.state & QStyle::State_MouseOver)
        painter->fillRect(border, option.palette.midlight());

    QWidget *widget = this->build_widget(index,
                                         option.rect.width(),
                                         option.rect.height());
    if (option.state & QStyle::State_Selected)
        widget->setBackgroundRole(QPalette::Highlight);

    widget->setFixedWidth(option.rect.width());

    widget->render(painter,
                   // QTBUG-26694
                   painter->deviceTransform().map(option.rect.topLeft()),
                   QRegion(),
                   QWidget::DrawChildren);

    delete widget;
}

QSize InputPdfFileDelegate::sizeHint(
        const QStyleOptionViewItem &option,
        const QModelIndex &index
        ) const
{
    Q_UNUSED(option)
    QWidget *widget = this->build_widget(index, 0, 0);
    QSize hint = widget->sizeHint();
    hint.setWidth(hint.width() + hint.height());
    delete widget;
    return hint;
}

QWidget *InputPdfFileDelegate::createEditor(
        QWidget *parent,
        const QStyleOptionViewItem &option,
        const QModelIndex &index
        ) const
{
    InputPdfFileWidget *editor = new InputPdfFileWidget(
                index,
                multipages,
                option.rect.height(),
                m_alternate_mix,
                parent);
    connect(m_mouse_event_filter, SIGNAL(mouse_button_pressed(QMouseEvent*)),
            editor, SLOT(mouse_button_pressed(QMouseEvent*)));
    connect(editor, SIGNAL(focus_out(QWidget*)),
            this, SLOT(end_editing(QWidget*)));
    connect(this, &InputPdfFileDelegate::profile_created,
            editor, &InputPdfFileWidget::profile_created);
    connect(editor, &InputPdfFileWidget::trigger_new_profile,
            this, &InputPdfFileDelegate::trigger_new_profile);
    return editor;
}

void InputPdfFileDelegate::setEditorData(QWidget *editor,
                                         const QModelIndex &index) const
{
    InputPdfFileWidget *w = static_cast<InputPdfFileWidget *>(editor);
    w->set_editor_data(index);
}

void InputPdfFileDelegate::setModelData(
        QWidget *editor,
        QAbstractItemModel *model,
        const QModelIndex &index
        ) const
{
    InputPdfFileWidget *w = static_cast<InputPdfFileWidget *>(editor);
    QStandardItemModel *std_model = static_cast<QStandardItemModel *>(model);
    w->set_model_data(std_model->itemFromIndex(index));

    emit data_edit();
}

void InputPdfFileDelegate::set_alternate_mix(bool enabled)
{
    m_alternate_mix = enabled;
}

void InputPdfFileDelegate::end_editing(QWidget *editor)
{
    emit commitData(editor);
    emit closeEditor(editor);
}
