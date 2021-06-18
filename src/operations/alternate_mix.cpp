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

#include "alternate_mix.h"

#include "../gui_utils.h"

AlternateMix::AlternateMix(QWidget *parent) :
    Merge(parent)
{
    m_name = tr("Alternate mix");
    m_icon = QIcon(":/icons/alternate_mix.svg");
    m_delegate->set_alternate_mix(true);
}

int AlternateMix::output_pages_count()
{
    int pages_count = 0;

    for (int i = 0; i < m_files_list_model->rowCount(); i++)
    {
        QStandardItem *item = m_files_list_model->item(i);
        pages_count += item->data(N_PAGES_ROLE).toInt();
    }

    return pages_count;
}

void AlternateMix::edit_menu_activated()
{
    QModelIndexList indexes =
            m_files_list_view->selectionModel()->selectedIndexes();

    m_files_list_view->edit(indexes.first());
}

void AlternateMix::write(const QString &filename)
{
    std::vector<std::string> input_filenames;

    std::vector<int> num_pages{};
    std::vector<int> file_ids{};
    std::vector<bool> reverse_order;
    PdfEditor editor;

    // add files to editor
    for (int i = 0; i < m_files_list_model->rowCount(); i++)
    {
        QStandardItem *item = m_files_list_model->item(i);
        QString file_path = item->data(FILE_PATH_ROLE).toString();
        num_pages.push_back(item->data(N_PAGES_ROLE).toInt());
        file_ids.push_back(editor.add_file(file_path.toStdString()));
        reverse_order.push_back(
                    item->data(REVERSE_ORDER_ROLE).toBool());
    }

    int max_num_pages = *std::max_element(num_pages.begin(),
                                          num_pages.end());

    // alternatively add pages
    for (int i{0}; i < max_num_pages; ++i)
    {
        for (size_t j{0}; j < file_ids.size(); ++j)
        {
            if (i < num_pages[j])
            {
                int index = reverse_order[j] ? num_pages[j] - 1 - i : i;
                editor.add_pages(file_ids[j], 0, nullptr,
                                 {{index, index}});
            }
        }

        emit progress_changed(100. * (i + 1) / (max_num_pages + 1));
    }

    editor.write(filename.toStdString());
}
