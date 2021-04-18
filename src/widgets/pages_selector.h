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

#ifndef PAGESSELECTOR_H
#define PAGESSELECTOR_H

#include <QLineEdit>
#include <QButtonGroup>

class LineEdit : public QLineEdit
{
    Q_OBJECT
public:
    explicit LineEdit(QWidget *parent=nullptr);

signals:
    void focusIn();

protected:
    virtual void focusInEvent(QFocusEvent *e);
};


class PagesSelector : public QWidget
{
    Q_OBJECT
public:
    explicit PagesSelector(bool show_all_pages=true,
                           bool all_pages_first=false,
                           QWidget *parent=nullptr);

    // Return a null string if there is an error in the selection
    QString get_selection_as_text(int num_pages);

signals:

private:
    QButtonGroup m_type;
    LineEdit m_selection;
};

#endif // PAGESSELECTOR_H
