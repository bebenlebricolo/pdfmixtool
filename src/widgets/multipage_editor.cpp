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

#include "multipage_editor.h"

#include <cmath>
#include <QVBoxLayout>
#include <QRadioButton>

#include "../gui_utils.h"

#define H_SPACING 10

MultipageEditor::MultipageEditor(QWidget *parent) :
    QWidget(parent),
    m_multipage{}
{
    // prepare all widgets
    m_standard_custom.addButton(new QRadioButton(tr("Standard size"), this), 0);
    m_standard_custom.addButton(new QRadioButton(tr("Custom size"), this), 1);

    int i = 0;
    for (PaperSize size : paper_sizes)
    {
        m_page_size.addItem(QString::fromStdString(size.name));
        i++;
    }

    m_orientation.addItem(tr("Portrait"));
    m_orientation.addItem(tr("Landscape"));

    m_page_width.setSuffix(" cm");
    m_page_width.setDecimals(1);
    m_page_width.setSingleStep(0.1);
    m_page_width.setMinimum(1.0);
    m_page_width.setMaximum(1000.0);

    m_page_height.setSuffix(" cm");
    m_page_height.setDecimals(1);
    m_page_height.setSingleStep(0.1);
    m_page_height.setMinimum(1.0);
    m_page_height.setMaximum(1000.0);

    m_rows.setMinimum(1);
    m_rows.setMaximum(10);

    m_columns.setMinimum(1);
    m_columns.setMaximum(10);

    m_rtl.setText(tr("Right-to-left"));

    m_h_alignment.addItem(tr("Left"), Multipage::Left);
    m_h_alignment.addItem(tr("Center"), Multipage::Center);
    m_h_alignment.addItem(tr("Right"), Multipage::Right);

    m_v_alignment.addItem(tr("Top"), Multipage::Top);
    m_v_alignment.addItem(tr("Center"), Multipage::Center);
    m_v_alignment.addItem(tr("Bottom"), Multipage::Bottom);

    m_margin_left.setSuffix(" cm");
    m_margin_left.setDecimals(1);
    m_margin_left.setSingleStep(0.1);
    m_margin_left.setMaximum(1000.0);

    m_margin_right.setSuffix(" cm");
    m_margin_right.setDecimals(1);
    m_margin_right.setSingleStep(0.1);
    m_margin_right.setMaximum(1000.0);

    m_margin_top.setSuffix(" cm");
    m_margin_top.setDecimals(1);
    m_margin_top.setSingleStep(0.1);
    m_margin_top.setMaximum(1000.0);

    m_margin_bottom.setSuffix(" cm");
    m_margin_bottom.setDecimals(1);
    m_margin_bottom.setSingleStep(0.1);
    m_margin_bottom.setMaximum(1000.0);

    m_spacing.setSuffix(" cm");
    m_spacing.setDecimals(1);
    m_spacing.setSingleStep(0.1);
    m_spacing.setMaximum(1000.0);

    set_multipage(m_multipage);

    // add all widgets to the main widget
    QVBoxLayout *layout = new QVBoxLayout();
    this->setLayout(layout);

    QLabel *label = new QLabel(tr("Page size"), this);
    QFont bold = label->font();
    bold.setBold(true);
    label->setContentsMargins(0, 0, 0, 5);
    label->setFont(bold);
    layout->addWidget(label);

    QHBoxLayout *h_layout = new QHBoxLayout();
    layout->addLayout(h_layout);
    h_layout->addWidget(m_standard_custom.button(0));
    h_layout->addItem(new QSpacerItem(H_SPACING, 0));
    h_layout->addWidget(m_standard_custom.button(1));
    h_layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding));

    layout->addWidget(&m_standard_size_chooser);
    h_layout = new QHBoxLayout();
    m_standard_size_chooser.setLayout(h_layout);
    h_layout->addWidget(&m_page_size);
    h_layout->addItem(new QSpacerItem(H_SPACING, 0));
    h_layout->addWidget(new QLabel(tr("Orientation:"), this));
    h_layout->addWidget(&m_orientation);
    h_layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding));

    layout->addWidget(&m_custom_size_chooser);
    h_layout = new QHBoxLayout();
    m_custom_size_chooser.setLayout(h_layout);
    h_layout->addWidget(new QLabel(tr("Width:"), this));
    h_layout->addWidget(&m_page_width);
    h_layout->addItem(new QSpacerItem(H_SPACING, 0));
    h_layout->addWidget(new QLabel(tr("Height:"), this));
    h_layout->addWidget(&m_page_height);
    h_layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding));

    m_standard_custom.button(0)->setChecked(true);
    m_custom_size_chooser.hide();
    m_standard_size_chooser.show();
    connect(&m_standard_custom,
        #if QT_VERSION < 0x060000
            QOverload<int>::of(&QButtonGroup::buttonClicked),
        #else
            &QButtonGroup::idClicked,
        #endif
            [=](int id) {
        if (id == 0)
        {
            m_custom_size_chooser.hide();
            m_standard_size_chooser.show();
            update_custom_page_size();
        }
        else
        {
            m_standard_size_chooser.hide();
            m_custom_size_chooser.show();
        }
    });

    label = new QLabel(tr("Pages layout"), this);
    label->setFont(bold);
    label->setContentsMargins(0, 20, 0, 5);
    layout->addWidget(label);

    h_layout = new QHBoxLayout();
    layout->addLayout(h_layout);
    h_layout->addWidget(new QLabel(tr("Rows:"), this));
    h_layout->addWidget(&m_rows);
    h_layout->addItem(new QSpacerItem(H_SPACING, 0));
    h_layout->addWidget(new QLabel(tr("Columns:"), this));
    h_layout->addWidget(&m_columns);
    h_layout->addItem(new QSpacerItem(H_SPACING, 0));
    h_layout->addWidget(new QLabel(tr("Spacing:"), this));
    h_layout->addWidget(&m_spacing);
    h_layout->addItem(new QSpacerItem(H_SPACING, 0));
    h_layout->addWidget(&m_rtl);
    h_layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding));

    label = new QLabel(tr("Pages alignment"), this);
    label->setFont(bold);
    label->setContentsMargins(0, 20, 0, 5);
    layout->addWidget(label);

    h_layout = new QHBoxLayout();
    layout->addLayout(h_layout);
    h_layout->addWidget(new QLabel(tr("Horizontal:"), this));
    h_layout->addWidget(&m_h_alignment);
    h_layout->addItem(new QSpacerItem(H_SPACING, 0));
    h_layout->addWidget(new QLabel(tr("Vertical:"), this));
    h_layout->addWidget(&m_v_alignment);
    h_layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding));

    label = new QLabel(tr("Margins"), this);
    label->setFont(bold);
    label->setContentsMargins(0, 20, 0, 5);
    layout->addWidget(label);

    h_layout = new QHBoxLayout();
    layout->addLayout(h_layout);
    h_layout->addWidget(new QLabel(tr("Left") + ":", this));
    h_layout->addWidget(&m_margin_left);
    h_layout->addItem(new QSpacerItem(H_SPACING, 0));
    h_layout->addWidget(new QLabel(tr("Right") + ":", this));
    h_layout->addWidget(&m_margin_right);
    h_layout->addItem(new QSpacerItem(H_SPACING, 0));
    h_layout->addWidget(new QLabel(tr("Top") + ":", this));
    h_layout->addWidget(&m_margin_top);
    h_layout->addItem(new QSpacerItem(H_SPACING, 0));
    h_layout->addWidget(new QLabel(tr("Bottom") + ":", this));
    h_layout->addWidget(&m_margin_bottom);
    h_layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding));

    connect(&m_page_size, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MultipageEditor::update_custom_page_size);
    connect(&m_orientation, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MultipageEditor::update_custom_page_size);
    connect(&m_page_width, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MultipageEditor::on_page_width_changed);
    connect(&m_page_height, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MultipageEditor::on_page_height_changed);

    connect(&m_page_size, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MultipageEditor::update_multipage);
    connect(&m_orientation, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MultipageEditor::update_multipage);
    connect(&m_page_width, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MultipageEditor::update_multipage);
    connect(&m_page_height, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MultipageEditor::update_multipage);

    connect(&m_rows, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MultipageEditor::update_multipage);
    connect(&m_columns, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MultipageEditor::update_multipage);
    connect(&m_spacing, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MultipageEditor::update_multipage);
    connect(&m_rtl, &QCheckBox::clicked,
            this, &MultipageEditor::update_multipage);

    connect(&m_h_alignment, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MultipageEditor::update_multipage);
    connect(&m_v_alignment, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MultipageEditor::update_multipage);

    connect(&m_margin_left, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MultipageEditor::update_multipage);
    connect(&m_margin_right, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MultipageEditor::update_multipage);
    connect(&m_margin_top, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MultipageEditor::update_multipage);
    connect(&m_margin_bottom, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MultipageEditor::update_multipage);

    connect(&m_rows, QOverload<int>::of(&QSpinBox::valueChanged),
            [=](int rows) {emit subpages_number_changed(rows * m_columns.value());});

    connect(&m_columns, QOverload<int>::of(&QSpinBox::valueChanged),
            [=](int columns) {
        if (columns > 1)
            m_rtl.setEnabled(true);
        else
            m_rtl.setEnabled(false);
        emit subpages_number_changed(columns * m_rows.value());
    });
}

void MultipageEditor::set_multipage(const Multipage &multipage)
{
    int i = 0;
    int button = 1;
    for (const PaperSize &size : paper_sizes)
    {
        if (std::lround(multipage.page_width * 10) ==
                std::lround(size.width * 10)
                && std::lround(multipage.page_height * 10) ==
                std::lround(size.height * 10))
        {
            m_page_size.setCurrentIndex(i);
            m_orientation.setCurrentIndex(0);
            button = 0;
            break;
        }
        else if (std::lround(multipage.page_width * 10) ==
                std::lround(size.height * 10)
                && std::lround(multipage.page_height * 10) ==
                std::lround(size.width * 10))
        {
            m_page_size.setCurrentIndex(i);
            m_orientation.setCurrentIndex(1);
            button = 0;
            break;
        }
        i++;
    }

    m_page_width.setValue(multipage.page_width);
    m_page_height.setValue(multipage.page_height);

    m_standard_custom.button(button)->setChecked(true);
    if (button == 0)
    {
        m_custom_size_chooser.hide();
        m_standard_size_chooser.show();
        update_custom_page_size();
    }
    else
    {
        m_standard_size_chooser.hide();
        m_custom_size_chooser.show();
    }

    m_rows.setValue(multipage.rows);
    m_columns.setValue(multipage.columns);
    m_spacing.setValue(multipage.spacing);
    m_rtl.setChecked(multipage.rtl);

    if (multipage.columns > 1)
        m_rtl.setEnabled(true);
    else
        m_rtl.setEnabled(false);

    m_h_alignment.setCurrentIndex(
                m_h_alignment.findData(multipage.h_alignment));
    m_v_alignment.setCurrentIndex(
                m_v_alignment.findData(multipage.v_alignment));

    m_margin_left.setValue(multipage.margin_left);
    m_margin_right.setValue(multipage.margin_right);
    m_margin_top.setValue(multipage.margin_top);
    m_margin_bottom.setValue(multipage.margin_bottom);

    m_multipage = multipage;
}

Multipage MultipageEditor::get_multipage() const
{
    return m_multipage;
}

void MultipageEditor::update_multipage()
{
    m_multipage.page_width = m_page_width.value();
    m_multipage.page_height = m_page_height.value();

    m_multipage.rows = m_rows.value();
    m_multipage.columns = m_columns.value();
    m_multipage.rtl = m_rtl.isChecked();
    m_multipage.spacing = m_spacing.value();

    m_multipage.h_alignment = static_cast<Multipage::Alignment>(
                m_h_alignment.currentData().toInt());
    m_multipage.v_alignment = static_cast<Multipage::Alignment>(
                m_v_alignment.currentData().toInt());

    m_multipage.margin_left = m_margin_left.value();
    m_multipage.margin_right = m_margin_right.value();
    m_multipage.margin_top = m_margin_top.value();
    m_multipage.margin_bottom = m_margin_bottom.value();

    emit multipage_changed(m_multipage);
}

void MultipageEditor::update_custom_page_size()
{
    int index = m_page_size.currentIndex();
    if (m_orientation.currentIndex() == 0)
    {
        m_page_width.setValue(paper_sizes[index].width);
        m_page_height.setValue(paper_sizes[index].height);
    }
    else
    {
        m_page_width.setValue(paper_sizes[index].height);
        m_page_height.setValue(paper_sizes[index].width);
    }
}

void MultipageEditor::on_page_width_changed(double value)
{
    m_margin_left.setMaximum(value / 2);
    m_margin_right.setMaximum(value / 2);
    m_spacing.setMaximum(value / 4);
}

void MultipageEditor::on_page_height_changed(double value)
{
    m_margin_top.setMaximum(value / 2);
    m_margin_bottom.setMaximum(value / 2);
    m_spacing.setMaximum(value / 4);
}
