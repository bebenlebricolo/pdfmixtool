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

#include "inputpdffilewidget.h"

#include <QGridLayout>
#include <QFocusEvent>
#include <QPainter>

double draw_preview_page(QPainter *painter,
                       int max_width, int max_height,
                       double page_width, double page_height,
                       Multipage::Alignment h_alignment,
                       Multipage::Alignment v_alignment,
                       const QString &text)
{
    double scale = std::min(max_width / page_width, max_height / page_height);

    int w = static_cast<int>(page_width * scale);
    int h = static_cast<int>(page_height * scale);
    int dx, dy;

    switch (h_alignment)
    {
    case Multipage::Left:
        dx = - max_width / 2;
        break;
    case Multipage::Center:
        dx = - w / 2;
        break;
    case Multipage::Right:
        dx = max_width / 2 - w;
        break;
    default:
        dx = - max_width / 2;
    }

    switch (v_alignment)
    {
    case Multipage::Top:
        dy = - max_height / 2;
        break;
    case Multipage::Center:
        dy = - h / 2;
        break;
    case Multipage::Bottom:
        dy = max_height / 2 - h;
        break;
    default:
        dy = - max_height / 2;
    }

    painter->drawRect(dx, dy, w, h);

    if (text.size() > 0)
    {
        QFont font = painter->font();
        font.setFamily("Serif");
        font.setPixelSize(h / 5 * 4);
        painter->setFont(font);

        int x = dx + w / 2 -
                painter->fontMetrics().boundingRect(text).width() / 2;
        int y = dy + h / 2 + font.pixelSize() / 2;

        painter->drawText(x, y, text);
    }

    return scale;
}

void draw_preview(QPainter *painter, const QRect &rect,
                  double source_width, double source_height,
                  int rotation,
                  bool multipage_enabled, const Multipage &multipage)
{
    painter->save();

    painter->fillRect(rect, painter->background());
    painter->setPen(QColor(10, 10, 10));

    painter->translate(rect.x() + rect.width() / 2,
                       rect.y() + rect.height() / 2);
    painter->rotate(rotation);

    if (!multipage_enabled)
        draw_preview_page(painter, rect.width() - 4, rect.height() - 4,
                          source_width, source_height,
                          Multipage::Center, Multipage::Center,
                          "1");
    else
    {
        double scale = draw_preview_page(painter,
                                         rect.width() - 4,
                                         rect.height() - 4,
                                         multipage.page_width,
                                         multipage.page_height,
                                         Multipage::Center,
                                         Multipage::Center,
                                         "");

        painter->rotate(multipage.rotation);

        int rows, columns, margin_left, margin_right, margin_top, margin_bottom,
                page_width, page_height;

        if (multipage.rotation == 90)
        {
            rows = multipage.columns;
            columns = multipage.rows;
            margin_left = static_cast<int>(multipage.margin_top * scale);
            margin_right = static_cast<int>(multipage.margin_bottom * scale);
            margin_top = static_cast<int>(multipage.margin_left  * scale);
            margin_bottom = static_cast<int>(multipage.margin_right  * scale);
            page_width = static_cast<int>(multipage.page_height * scale);
            page_height = static_cast<int>(multipage.page_width * scale);

        }
        else
        {
            rows = multipage.rows;
            columns = multipage.columns;
            margin_left = static_cast<int>(multipage.margin_left * scale);
            margin_right = static_cast<int>(multipage.margin_right * scale);
            margin_top = static_cast<int>(multipage.margin_top * scale);
            margin_bottom = static_cast<int>(multipage.margin_bottom * scale);
            page_width = static_cast<int>(multipage.page_width * scale);
            page_height = static_cast<int>(multipage.page_height * scale);
        }

        int spacing = static_cast<int>(multipage.spacing * scale);

        int subpage_width = (page_width - margin_left - margin_right -
                             (columns - 1) * spacing) / columns;
        int subpage_height = (page_height - margin_top - margin_bottom -
                              (rows - 1) * spacing) / rows;

        int page_number = 1;

        for (int i = 0; i < rows; i++)
        {
            for (int j = 0; j < columns; j++)
            {
                int dx = margin_left - page_width / 2 +
                        j * (spacing + subpage_width) + subpage_width / 2;
                int dy = margin_top - page_height / 2 +
                        i * (spacing + subpage_height) + subpage_height / 2;

                painter->translate(dx, dy);

                draw_preview_page(painter, subpage_width, subpage_height,
                                  source_width, source_height,
                                  multipage.h_alignment, multipage.v_alignment,
                                  QString::number(page_number));

                painter->translate(-dx, -dy);

                page_number++;
            }
        }

    }

    painter->restore();
}

InputPdfFileWidget::InputPdfFileWidget(
        const QModelIndex &index,
        const QMap<int, Multipage> &custom_multipages,
        int preview_size,
        QWidget *parent) :
    QWidget(parent),
    m_multipages(custom_multipages),
    m_preview_size(preview_size),
    m_preview_label(new QLabel(this)),
    m_pages_filter_lineedit(new QLineEdit(this)),
    m_multipage_combobox(new QComboBox(this)),
    m_rotation_combobox(new QComboBox(this)),
    m_outline_entry_lineedit(new QLineEdit(this))
{
    m_page_width = index.data(PAGE_WIDTH_ROLE).toDouble();
    m_page_height = index.data(PAGE_HEIGHT_ROLE).toDouble();

    this->setBackgroundRole(QPalette::AlternateBase);
    this->setAutoFillBackground(true);

    QGridLayout *layout = new QGridLayout();
    this->setLayout(layout);

    m_pages_filter_lineedit->setClearButtonEnabled(true);
    m_outline_entry_lineedit->setClearButtonEnabled(true);

    m_multipage_combobox->addItem(tr("Disabled"), -1);
    QMap<int, Multipage>::const_iterator it;
    for (it = m_multipages.constBegin();
         it != m_multipages.constEnd();
         ++it)
        m_multipage_combobox->addItem(
                    QString::fromStdString(it.value().name),
                    it.key());

    m_rotation_combobox->addItem(tr("No rotation"), 0);
    m_rotation_combobox->addItem("90°", 90);
    m_rotation_combobox->addItem("180°", 180);
    m_rotation_combobox->addItem("270°", 270);

    layout->addWidget(m_preview_label, 1, 1, 3, 1);
    layout->addWidget(new QLabel(tr("Pages:"), this), 1, 2);
    layout->addWidget(m_pages_filter_lineedit, 1, 3);
    layout->addWidget(new QLabel(tr("Multipage:"), this), 1, 4);
    layout->addWidget(m_multipage_combobox, 1, 5);
    layout->addWidget(new QLabel(tr("Rotation:"), this), 2, 2);
    layout->addWidget(m_rotation_combobox, 2, 3);
    layout->addWidget(new QLabel(tr("Outline entry:"), this), 2, 4);
    layout->addWidget(m_outline_entry_lineedit, 2, 5);

    layout->addItem(new QSpacerItem(0, 0), 1, 6);
    layout->setColumnStretch(3, 10);
    layout->setColumnStretch(5, 10);
    layout->setColumnStretch(6, 90);

    connect(m_multipage_combobox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(update_preview()));
    connect(m_rotation_combobox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(update_preview()));

    update_preview();
}

void InputPdfFileWidget::set_editor_data(const QModelIndex &index)
{
    m_pages_filter_lineedit->setText(index.data(OUTPUT_PAGES_ROLE).toString());
    m_multipage_combobox->setCurrentIndex(
            m_multipage_combobox->findData(index.data(MULTIPAGE_ROLE).toInt()));
    m_rotation_combobox->setCurrentIndex(
            m_rotation_combobox->findData(index.data(ROTATION_ROLE).toInt()));
    m_outline_entry_lineedit->setText(
                index.data(OUTLINE_ENTRY_ROLE).toString());
}

void InputPdfFileWidget::set_model_data(QStandardItem *item)
{
    QString current_output_pages = m_pages_filter_lineedit->text();
    int current_mp_index = m_multipage_combobox->currentData().toInt();
    int current_rotation = m_rotation_combobox->currentData().toInt();
    QString current_outline_entry = m_outline_entry_lineedit->text();

    item->setData(current_output_pages, OUTPUT_PAGES_ROLE);
    item->setData(current_mp_index, MULTIPAGE_ROLE);
    item->setData(current_rotation, ROTATION_ROLE);
    item->setData(current_outline_entry, OUTLINE_ENTRY_ROLE);
}

void InputPdfFileWidget::mouse_button_pressed(QMouseEvent *event)
{
    QRect mp_rect = m_multipage_combobox->rect();
    mp_rect.setHeight((m_multipage_combobox->count() + 1) * mp_rect.height());
    mp_rect.setTop(mp_rect.top() - m_multipage_combobox->count() *
                   mp_rect.height());

    QRect rotation_rect = m_rotation_combobox->rect();
    rotation_rect.setHeight((m_rotation_combobox->count() + 1) *
                            rotation_rect.height());
    rotation_rect.setTop(rotation_rect.top() - m_rotation_combobox->count() *
                         rotation_rect.height());

    if (! this->rect().contains(
                this->mapFromGlobal(event->globalPos())) &&
            ! mp_rect.contains(
                m_multipage_combobox->mapFromGlobal(event->globalPos())) &&
            ! rotation_rect.contains(
                m_rotation_combobox->mapFromGlobal(event->globalPos()))
            )
        emit focus_out(this);
}

void InputPdfFileWidget::update_preview()
{
    QPixmap pixmap(m_preview_size - layout()->contentsMargins().top() * 2,
                   m_preview_size - layout()->contentsMargins().top() * 2);
    QPainter painter(&pixmap);

    bool ok;
    int mp_index = m_multipage_combobox->currentData().toInt(&ok);
    if (!ok)
        mp_index = -1;
    bool mp_enabled;
    Multipage mp;
    if (mp_index < 0)
        mp_enabled = false;
    else
    {
        mp_enabled = true;
        mp = m_multipages[mp_index];
    }

    draw_preview(&painter, pixmap.rect(),
                 m_page_width, m_page_height,
                 m_rotation_combobox->currentData().toInt(),
                 mp_enabled, mp);


    m_preview_label->setPixmap(pixmap);
}
