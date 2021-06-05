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

#ifndef ALTERNATEMIX_H
#define ALTERNATEMIX_H

#include "merge.h"

class AlternateMix : public Merge
{
    Q_OBJECT
public:
    explicit AlternateMix(QWidget *parent = nullptr);

    int output_pages_count() override;

public slots:
    void edit_menu_activated() override;

protected:
    void write(const QString &filename) override;
};

#endif // ALTERNATEMIX_H
