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

#ifndef OUTPUTPREVIEW_H
#define OUTPUTPREVIEW_H

#include <QWidget>

#include "../pdf_edit_lib/definitions.h"
#include "../pdf_edit_lib/pdf_info.h"
#include "../gui_utils.h"

class OutputPreview : public QWidget
{
    Q_OBJECT
public:
    explicit OutputPreview(QWidget *parent = nullptr);

    void set_page_size(double width, double height);

    void set_pdf_info(const PdfInfo &pdf_info);

    void set_multipage_enabled(bool enabled);

    void set_rotation(int rotation);

public slots:
    void set_multipage(const Multipage &multipage);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    bool m_has_page_size;
    double m_page_width;
    double m_page_height;
    bool m_multipage_enabled;
    Multipage m_multipage;
    int m_rotation;

    static double draw_preview_page(QPainter &painter,
                                    int max_width,
                                    int max_height,
                                    double page_width,
                                    double page_height,
                                    Multipage::Alignment h_alignment,
                                    Multipage::Alignment v_alignment,
                                    const QString &text);
};

#endif // OUTPUTPREVIEW_H
