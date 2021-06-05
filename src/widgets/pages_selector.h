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

    using Intervals = std::vector<std::pair<int, int>>;

public:
    explicit PagesSelector(bool show_all_pages=true,
                           bool all_pages_first=false,
                           QWidget *parent=nullptr);

    void set_num_pages(int num_pages);

    bool has_error();

    Intervals get_selected_intervals();

    int get_number_of_selected_pages();

    int get_number_of_unique_selected_pages();

    // Return a null string if there is an error in the selection
    QString get_selection_as_text();

signals:
    void selection_changed();

private:
    int m_num_pages;
    QButtonGroup m_type;
    LineEdit m_selection;
};

#endif // PAGESSELECTOR_H
