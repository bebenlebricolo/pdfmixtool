/* Copyright (C) 2020-2021 Marco Scarpetta
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

#include <QFileInfo>
#include <QDir>
#include <QPainter>

#include "pdfinfolabel.h"

PdfInfoLabel::PdfInfoLabel(QWidget *parent) :
    QLabel(parent)
{

}

void PdfInfoLabel::set_pdf_info(const PdfInfo &pdfinfo)
{
    this->set_pdf_info(QString::fromStdString(pdfinfo.filename()),
                       pdfinfo.width(),
                       pdfinfo.height(),
                       QString::fromStdString(pdfinfo.paper_size()),
                       pdfinfo.is_portrait(),
                       pdfinfo.n_pages());
}

void PdfInfoLabel::set_pdf_info(const QString &filename,
                                double page_width,
                                double page_height,
                                QString paper_size,
                                bool is_portrait,
                                int n_pages)
{
    if (paper_size.size() > 0)
        paper_size = QString("(%1 %2) ").arg(
                    paper_size,
                    is_portrait ?
                        tr("portrait") :
                        tr("landscape"));

    QFileInfo fileinfo(filename);

    m_path = fileinfo.absolutePath();
    m_filename = fileinfo.fileName();
    m_info = QString(" − %1 cm \u00D7 %2 cm %3− %4").arg(
                QString::number(page_width),
                QString::number(page_height),
                paper_size,
                tr("%n page(s)", "", n_pages));

    this->setText("<b>" + m_filename + "</b>" + m_info);

    m_minimum_width = this->minimumSizeHint().width();
}

void PdfInfoLabel::paintEvent(QPaintEvent *event)
{
    QFrame::paintEvent(event);

    QPainter painter(this);

    QFontMetrics fm = painter.fontMetrics();

    int y = (painter.device()->height() + fm.capHeight()) / 2;

    int available_path_width = painter.device()->width() - m_minimum_width - 20;

    QString path;
    path = fm.elidedText(m_path, Qt::ElideLeft, available_path_width);
    if (!path.isEmpty())
        path += QDir::separator();

    painter.drawText(this->contentsMargins().left(),
                     this->contentsMargins().top() + y,
                     path);

    int x = fm.horizontalAdvance(path);

    QFont font = painter.font();
    font.setBold(true);
    painter.setFont(font);
    painter.drawText(this->contentsMargins().left() + x,
                     this->contentsMargins().top() + y,
                     m_filename);

    fm = painter.fontMetrics();
    x += fm.horizontalAdvance(m_filename);

    font.setBold(false);
    painter.setFont(font);
    painter.drawText(this->contentsMargins().left() + x,
                     this->contentsMargins().top() + y,
                     m_info);
}
