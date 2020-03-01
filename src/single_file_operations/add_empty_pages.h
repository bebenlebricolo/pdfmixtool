/* Copyright (C) 2020 Marco Scarpetta
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

#ifndef ADDEMPTYPAGES_H
#define ADDEMPTYPAGES_H

#include <QWidget>
#include <QSpinBox>
#include <QButtonGroup>
#include <QComboBox>

class AddEmptyPages : public QWidget
{
    Q_OBJECT
public:
    explicit AddEmptyPages(QWidget *parent = nullptr);

    QSpinBox count;
    QButtonGroup page_size;
    QDoubleSpinBox page_width;
    QDoubleSpinBox page_height;
    QComboBox standard_page_size;
    QButtonGroup orientation;
    QButtonGroup before_after;
    QSpinBox page;

signals:
    void save_button_pressed();
    void save_as_button_pressed();

};

#endif // ADDEMPTYPAGES_H
