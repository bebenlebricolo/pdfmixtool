/* Copyright (C) 2022 Marco Scarpetta
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

#include "extract_images.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QFileDialog>
#include <qpdf/QPDF.hh>
#include <Magick++/Blob.h>
#include <Magick++/Image.h>
#include <fstream>

#include "../gui_utils.h"

ExtractImages::ExtractImages(QWidget *parent) :
    AbstractOperation(parent)
{
    m_name = tr("Extract images");
    m_icon = ":/icons/extract_images.svg";

    QVBoxLayout *v_layout = new QVBoxLayout();
    this->setLayout(v_layout);

    QLabel *pages_selector_label = new QLabel(tr("Extract from:"), this);
    v_layout->addWidget(pages_selector_label);

    m_pages_selector = new PagesSelector(true, true, this);
    v_layout->addWidget(m_pages_selector);

    // spacer
    v_layout->addWidget(new QWidget(this), 1);

    QHBoxLayout *h_layout = new QHBoxLayout();
    v_layout->addLayout(h_layout);
    h_layout->addWidget(new QLabel(tr("Base name:"), this));
    h_layout->addWidget(&m_base_name, 1);
    m_base_name.setClearButtonEnabled(true);
    h_layout = new QHBoxLayout();
    v_layout->addLayout(h_layout);
    h_layout->addWidget(new QLabel(tr("Image file format:"), this));
    h_layout->addWidget(&m_image_format, 1);
    m_image_format.addItem("PNG", "png");
    m_image_format.addItem("JPEG", "jpg");
    m_image_format.addItem("GIF", "gif");
    m_image_format.addItem("TIFF", "tif");

    QPushButton *extract_button = new QPushButton(
                QIcon::fromTheme("document-save-as"),
                tr("Extract…"),
                this);
    connect(extract_button, &QPushButton::pressed,
            this, &ExtractImages::extract);

    h_layout = new QHBoxLayout();
    h_layout->addWidget(new QWidget(this), 1);
    h_layout->addWidget(extract_button);
    v_layout->addLayout(h_layout);
}

void ExtractImages::set_pdf_info(const PdfInfo &pdf_info)
{
    m_pages_selector->set_num_pages(pdf_info.n_pages());

    AbstractOperation::set_pdf_info(pdf_info);

    QFileInfo file_info(QString::fromStdString(m_pdf_info.filename()));

    m_base_name.setText(file_info.completeBaseName());
}

void ExtractImages::extract()
{
    if (m_pages_selector->has_error())
        return;

    std::vector<std::pair<int, int>> intervals =
            m_pages_selector->get_selected_intervals();

    int total_pages = m_pages_selector->get_number_of_selected_pages();

    QString dir_name = QFileDialog::getExistingDirectory(
                this,
                tr("Select save directory"),
                settings->value("save_directory",
                                  settings->value("open_directory", "")
                                  ).toString(),
                QFileDialog::ShowDirsOnly
                | QFileDialog::DontResolveSymlinks);

    if (!dir_name.isNull())
    {
        emit write_started();

        settings->setValue(
                    "save_directory",
                    QFileInfo(dir_name).dir().absolutePath());

        QDir dir(dir_name);
        QString base_name = m_base_name.text();

        try
        {
            QPDF pdf;
            pdf.processFile(m_pdf_info.filename().c_str());

            std::vector<QPDFObjectHandle> pages = pdf.getAllPages();

            std::set<QPDFObjGen> saved;
            int j{0};
            int k{0};
            std::vector<std::pair<int, int>>::iterator it;
            for (it = intervals.begin(); it != intervals.end(); ++it)
            {
                for (int i = it->first; i <= it->second; i++)
                {
                    ++j;

                    QPDFPageObjectHelper page(pages.at(i));

                    page.forEachImage(true,  [=, &k, &saved] (
                                      QPDFObjectHandle& obj,
                                      QPDFObjectHandle& xobj_dict,
                                      std::string const& key)
                    {
                        if (saved.find(obj.getObjGen()) != saved.end())
                            return;
                        saved.insert(obj.getObjGen());

                        QPDFObjectHandle obj_dict = obj.getDict();

                        int depth = obj_dict.getKey("/BitsPerComponent").getIntValue();

                        QPDFObjectHandle cs = obj_dict.getKey("/ColorSpace");
                        std::string map;
                        if (cs.getName() == "/DeviceGray")
                            map = "GRAY";
                        else if (cs.getName() == "/DeviceRGB")
                            map = "RGB";
                        else if (cs.getName() == "/DeviceCMYK")
                            map = "CMYK";
                        else
                            return;

                        ++k;

                        QString filename =
                                base_name + QString("_%1.%2").arg(
                                    QString::number(k),
                                    m_image_format.currentData().toString());

                        // this is to continue extracting images after an error
                        try
                        {
                            PointerHolder<Buffer> buffer = obj.getStreamData(qpdf_dl_all);

                            int w = obj_dict.getKey("/Width").getUIntValue();
                            int h = obj_dict.getKey("/Height").getUIntValue();

                            Magick::Blob blob{buffer->getBuffer(), buffer->getSize()};
                            Magick::Image img;
                            img.read(blob, Magick::Geometry(w, h), depth, map);
                            img.magick(m_image_format.currentText().toStdString());
                            img.write(dir.filePath(filename).toStdString());
                        }
                        catch (std::exception &e)
                        {
                            --k;
                        }
                    });

                    emit progress_changed(100. * j / total_pages);
                }
            }

            emit write_finished(dir_name);
        }
        catch (std::exception &e)
        {
            emit write_error(QString::fromStdString(e.what()));
        }
    }
}
