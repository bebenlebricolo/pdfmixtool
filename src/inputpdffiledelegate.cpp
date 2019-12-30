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

#include "inputpdffiledelegate.h"

#include <QPainter>
#include <QFileInfo>
#include <QDir>

InputPdfFileDelegate::InputPdfFileDelegate(
        MouseEventFilter *filter,
        const QMap<int, Multipage> &custom_multipages,
        MultipageProfilesManager *mp_manager,
        QWidget *parent) :
    QStyledItemDelegate(parent),
    m_mouse_event_filter(filter),
    m_multipages(custom_multipages),
    m_mp_manager(mp_manager),
    m_editing_enabled(true)
{

}

void InputPdfFileDelegate::paint(
        QPainter *painter,
        const QStyleOptionViewItem &option,
        const QModelIndex &index) const
{
    QString file_path = index.data(FILE_PATH_ROLE).toString();
    double page_width = index.data(PAGE_WIDTH_ROLE).toDouble();
    double page_height = index.data(PAGE_HEIGHT_ROLE).toDouble();
    QString paper_size = index.data(PAPER_SIZE_ROLE).toString();
    bool is_portrait = index.data(IS_PORTRAIT_ROLE).toBool();
    int n_pages = index.data(N_PAGES_ROLE).toInt();
    QString output_pages = index.data(OUTPUT_PAGES_ROLE).toString();
    int mp_index = index.data(MULTIPAGE_ROLE).toInt();
    int rotation = index.data(ROTATION_ROLE).toInt();
    QString outline_entry = index.data(OUTLINE_ENTRY_ROLE).toString();

    if (paper_size.size() > 0)
        paper_size = QString("(%1 %2) ").arg(
                    paper_size,
                    is_portrait ? tr("portrait") : tr("landscape"));

    bool mp_enabled;
    Multipage mp;
    if (mp_index < 0)
        mp_enabled = false;
    else
    {
        mp_enabled = true;
        mp = m_multipages[mp_index];
    }

    QColor text_color = option.palette.text().color();

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
    {
        painter->fillRect(border, option.palette.highlight());
        text_color = option.palette.highlightedText().color();
    }
    else if (option.state & QStyle::State_MouseOver)
        painter->fillRect(border, option.palette.midlight());

    // Draw preview
    draw_preview(painter,
                 QRect(option.rect.x() + 2,
                       option.rect.y() + 2,
                       option.rect.height() - 4,
                       option.rect.height() - 4),
                 page_width, page_height,
                 rotation,
                 mp_enabled, mp);

    // Draw text
    QFontMetrics fm = painter->fontMetrics();

    int font_height = fm.height();

    int line_height = static_cast<int>(1.2 * font_height);

    int x = option.rect.x() + option.rect.height() + 10;
    int y = option.rect.y() + line_height;

    // Define fonts
    painter->setPen(text_color);
    QFont font = painter->font();
    QFont bold = font;
    bold.setBold(true);

    // First row minimum width
    QString file_info = QString(" − %1 cm \u00D7 %2 cm %3− %5").arg(
                QString::number(page_width),
                QString::number(page_height),
                paper_size,
                tr("%n page(s)", "", n_pages)
                );

    int first_row = fm.boundingRect(file_info).width();

    QFileInfo fileinfo(file_path);
    QFontMetrics fmb(bold);
    first_row += fmb.boundingRect(fileinfo.fileName()).width();

    int path_available_width = option.rect.width() -
            (x - option.rect.x() + first_row + 30);
    QString path = fm.elidedText(fileinfo.absolutePath(),
                                 Qt::ElideLeft,
                                 path_available_width);
    if (path.size() != 0)
        path += QDir::separator();

    painter->drawText(x, y, path);
    x += fm.boundingRect(path).width();

    painter->setFont(bold);
    painter->drawText(x, y, fileinfo.fileName());
    x += fmb.boundingRect(fileinfo.fileName()).width();

    painter->setFont(font);
    painter->drawText(x, y, file_info);

    if (m_editing_enabled)
    {
        y += line_height;
        x = option.rect.x() + option.rect.height() + 10 + 30;

        if (output_pages.size() == 0)
            output_pages = tr("All");

        QString pages = tr("Pages:") + ' ' + output_pages;
        QString multipage = tr("Multipage:") + ' ' + (
                    mp_index > 0 ?
                        QString(" %1").arg(QString::fromStdString(mp.name)) :
                        tr("Disabled"));
        QString rotation_text = tr("Rotation:") + QString(" %1°").arg(rotation);
        QString outline_entry_text = tr("Outline entry:") + ' ' + outline_entry;

        painter->drawText(x, y, pages);
        y += line_height;

        painter->setPen(text_color);
        painter->drawText(x, y, rotation_text);

        y = option.rect.y() + 2 * line_height;
        x += std::max(fm.boundingRect(pages).width(),
                      fm.boundingRect(rotation).width()) + 40;
        painter->drawText(x, y, multipage);
        y += line_height;

        painter->drawText(x, y, outline_entry_text);
    }
}

QSize InputPdfFileDelegate::sizeHint(
        const QStyleOptionViewItem &option,
        const QModelIndex &index
        ) const
{
    QString file_path = index.data(FILE_PATH_ROLE).toString();
    double page_width = index.data(PAGE_WIDTH_ROLE).toDouble();
    double page_height = index.data(PAGE_HEIGHT_ROLE).toDouble();
    QString paper_size = index.data(PAPER_SIZE_ROLE).toString();
    bool is_portrait = index.data(IS_PORTRAIT_ROLE).toBool();
    int n_pages = index.data(N_PAGES_ROLE).toInt();
    QString output_pages = index.data(OUTPUT_PAGES_ROLE).toString();
    int mp_index = index.data(MULTIPAGE_ROLE).toInt();
    int rotation = index.data(ROTATION_ROLE).toInt();

    if (paper_size.size() > 0)
        paper_size = QString("(%1 %2) ").arg(
                    paper_size,
                    is_portrait ? tr("portrait") : tr("landscape"));

    Multipage mp;
    if (mp_index >= 0)
        mp = m_multipages[mp_index];

    QFontMetrics fm = option.fontMetrics;

    int font_height = fm.height();

    int line_height = static_cast<int>(1.2 * font_height);

    int h = line_height * 3 + 15;

    int w = h + 20;

    // Second row width
    if (output_pages.size() == 0)
        output_pages = tr("All");

    QString pages = tr("Pages:") + ' ' + output_pages;
    QString multipage = tr("Multipage:") + ' ' + (
                mp_index > 0 ?
                    QString(" %1").arg(QString::fromStdString(mp.name)) :
                    tr("Disabled"));
    QString rotation_text = tr("Rotation:") + QString(" %1°").arg(rotation);

    int second_row = std::max(
                fm.boundingRect(pages).width(),
                fm.boundingRect(rotation).width()) +
            40 + fm.boundingRect(multipage).width();

    // First row minimum width
    QString file_info = QString(" − %1 cm \u00D7 %2 cm %3− %5").arg(
                QString::number(page_width),
                QString::number(page_height),
                paper_size,
                tr("%n page(s)", "", n_pages)
                );

    int first_row = fm.boundingRect(file_info).width();

    QFileInfo fileinfo(file_path);
    QFont font = option.font;
    font.setBold(true);
    QFontMetrics fmb(font);
    first_row += fmb.boundingRect(fileinfo.fileName()).width() + 30;

    w += std::max(first_row, second_row);

    return QSize(w, h);
}

QWidget *InputPdfFileDelegate::createEditor(
        QWidget *parent,
        const QStyleOptionViewItem &option,
        const QModelIndex &index
        ) const
{
    InputPdfFileWidget *editor = new InputPdfFileWidget(
                index,
                m_multipages,
                m_mp_manager,
                option.rect.height(),
                parent);
    connect(m_mouse_event_filter, SIGNAL(mouse_button_pressed(QMouseEvent*)),
            editor, SLOT(mouse_button_pressed(QMouseEvent*)));
    connect(editor, SIGNAL(focus_out(QWidget*)),
            this, SLOT(end_editing(QWidget*)));
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

void InputPdfFileDelegate::set_editing_enabled(bool enabled)
{
    m_editing_enabled = enabled;
}

void InputPdfFileDelegate::end_editing(QWidget *editor)
{
    emit commitData(editor);
    emit closeEditor(editor);
}
