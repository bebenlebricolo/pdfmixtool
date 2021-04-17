/* Copyright (C) 2017-2021 Marco Scarpetta
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

#ifndef MULTIPAGEPROFILESMANAGER_H
#define MULTIPAGEPROFILESMANAGER_H

#include <QDialog>
#include <QSettings>
#include <QListView>
#include <QStandardItemModel>
#include <QPushButton>

#include "editmultipageprofiledialog.h"

class MultipageProfilesManager : public QDialog
{
    Q_OBJECT
public:
    explicit MultipageProfilesManager(QWidget *parent = nullptr);

public slots:
    void new_profile_button_pressed();

    void edit_profile_button_pressed();

    void delete_profile_button_pressed();

    void profile_double_clicked(const QModelIndex &index);

    void edit_dialog_accepted();

    void edit_dialog_closed();

    void closeEvent(QCloseEvent *event) override;

signals:
    void close_signal();

    void profile_created(int index);

private:
    bool profile_name_exists(const QString &name);

    EditMultipageProfileDialog *m_edit_dialog;

    QPushButton *m_new_profile_button;
    QPushButton *m_delete_profile_button;

    QListView *m_profiles_view;
    QStandardItemModel *m_profiles_model;
};

#endif // MULTIPAGEPROFILESMANAGER_H
