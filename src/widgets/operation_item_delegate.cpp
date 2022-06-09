/* Copyright (C) 2022 Marco Scarpetta
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

#include "operation_item_delegate.h"

#include <QPainter>
#include <QSvgRenderer>
#include <QtXml/QDomDocument>
#include <QFile>

#include "../gui_utils.h"

OperationItemDelegate::OperationItemDelegate(QObject *parent) :
    QStyledItemDelegate(parent),
    m_icon_width{128},
    m_icon_height{64}
{

}

void OperationItemDelegate::paint(QPainter *painter,
                   const QStyleOptionViewItem &option,
                   const QModelIndex &index) const
{
    // draw background
    if (option.state & QStyle::State_Selected)
        painter->fillRect(option.rect, option.palette.highlight());
    else if (option.state & QStyle::State_MouseOver)
        painter->fillRect(option.rect, option.palette.alternateBase());
    else
        painter->fillRect(option.rect, option.palette.base());

    // draw icon
    QFile file(index.data(Qt::UserRole).toString());
    file.open(QIODevice::ReadOnly);
    QDomDocument doc;
    doc.setContent(file.readAll());
    QDomNodeList path_elements = doc.elementsByTagName("path");
    for (int i = 0; i < path_elements.length(); ++i)
    {
        QDomElement element = path_elements.at(i).toElement();
        if (element.attribute("id") == "colorize")
        {
            QColor color = option.state & QStyle::State_Selected ?
                        option.palette.highlightedText().color() :
                        option.palette.text().color();
            element.setAttribute("style",
                                 QString("fill:%1;").arg(color.name()));
        }
    }
    QSvgRenderer svg_renderer(doc.toByteArray());
    int x = option.rect.x() + (option.rect.width() - m_icon_width) / 2;
    int y = option.rect.top() + MARGIN;
    svg_renderer.render(painter, QRectF(x, y, m_icon_width, m_icon_height));

    // draw text
    QPen pen;
    if (option.state & QStyle::State_Selected)
        pen.setBrush(option.palette.highlightedText());
    else
        pen.setBrush(option.palette.text());
    painter->setPen(pen);
    int text_width =
            option.fontMetrics.horizontalAdvance(index.data().toString());
    x = option.rect.x() + (option.rect.width() - text_width) / 2;
    y = option.rect.bottom() - MARGIN - option.fontMetrics.descent();
    painter->drawText(x, y, index.data().toString());
}

QSize OperationItemDelegate::sizeHint(const QStyleOptionViewItem &option,
               const QModelIndex &index) const
{
    int text_width =
            option.fontMetrics.horizontalAdvance(index.data().toString());
    int height = 3 * MARGIN + m_icon_height + option.fontMetrics.height();
    int width = 2 * MARGIN + std::max(m_icon_width, text_width);
    return QSize(width, height);
}
