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

#ifndef PDFINFOLABEL_H
#define PDFINFOLABEL_H

#include <QLabel>

#include "../pdf_edit_lib/pdf_info.h"

class PdfInfoLabel : public QLabel
{
    Q_OBJECT
public:
    PdfInfoLabel(QWidget *parent=nullptr);

    void set_pdf_info(const PdfInfo &pdfinfo);

    void set_pdf_info(const QString &filename,
                      double page_width,
                      double page_height,
                      QString paper_size,
                      bool is_portrait,
                      int n_pages);

protected:
    virtual void paintEvent(QPaintEvent *event);

private:
    QString m_path;
    QString m_filename;
    QString m_info;
    int m_minimum_width;
};

#endif // PDFINFOLABEL_H
