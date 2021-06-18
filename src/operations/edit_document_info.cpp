#include "edit_document_info.h"

#include <fstream>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>

#include "../pdf_edit_lib/pdf_editor.h"
#include "../gui_utils.h"

EditDocumentInfo::EditDocumentInfo(QWidget *parent) :
    AbstractOperation(parent)
{
    m_name = tr("Document information");
    m_icon = QIcon(":/icons/edit_document_info.svg");

    QVBoxLayout *v_layout = new QVBoxLayout();
    this->setLayout(v_layout);

    QGridLayout *grid_layout = new QGridLayout();
    v_layout->addLayout(grid_layout);

    int row = 1;
    grid_layout->addWidget(new QLabel(tr("Title:"), this), row, 1);
    grid_layout->addWidget(&m_title, row, 2);

    grid_layout->addWidget(new QLabel(tr("Author:"), this), ++row, 1);
    grid_layout->addWidget(&m_author, row, 2);

    grid_layout->addWidget(new QLabel(tr("Subject:"), this), ++row, 1);
    grid_layout->addWidget(&m_subject, row, 2);

    grid_layout->addWidget(new QLabel(tr("Keywords:"), this), ++row, 1);
    grid_layout->addWidget(&m_keywords, row, 2);

    grid_layout->addWidget(new QLabel(tr("Creator:"), this), ++row, 1);
    grid_layout->addWidget(&m_creator, row, 2);

    grid_layout->addWidget(new QLabel(tr("Producer:"), this), ++row, 1);
    grid_layout->addWidget(&m_producer, row, 2);

    grid_layout->addWidget(new QLabel(tr("Creation date:"), this), ++row, 1);
    grid_layout->addWidget(&m_creation_date, row, 2);
    grid_layout->addWidget(&m_creation_date_enabled, row, 3);

    grid_layout->addWidget(new QLabel(tr("Modification date:"), this), ++row, 1);
    grid_layout->addWidget(&m_mod_date, row, 2);
    grid_layout->addWidget(&m_mod_date_enabled, row, 3);

    m_title.setClearButtonEnabled(true);
    m_author.setClearButtonEnabled(true);
    m_subject.setClearButtonEnabled(true);
    m_keywords.setClearButtonEnabled(true);
    m_creator.setClearButtonEnabled(true);
    m_producer.setClearButtonEnabled(true);
    m_creation_date.setCalendarPopup(true);
    m_mod_date.setCalendarPopup(true);

    connect(&m_creation_date_enabled, &QCheckBox::toggled,
            &m_creation_date, &QDateTimeEdit::setEnabled);
    connect(&m_mod_date_enabled, &QCheckBox::toggled,
            &m_mod_date, &QDateTimeEdit::setEnabled);

    // spacer
    v_layout->addWidget(new QWidget(this), 1);

    QHBoxLayout *h_layout = new QHBoxLayout();
    v_layout->addLayout(h_layout);

    // spacer
    h_layout->addWidget(new QWidget(this), 1);

    QPushButton *save_as_button = new QPushButton(
                QIcon::fromTheme("document-save-as"),
                tr("Save as…"),
                this);
    save_as_button->setShortcut(QKeySequence::SaveAs);
    save_as_button->setToolTip(
                QString(TOOLTIP_STRING)
                .arg(
                    save_as_button->text(),
                    save_as_button->shortcut().toString()));

    h_layout->addWidget(&m_save_button);
    h_layout->addWidget(save_as_button);

    connect(&m_save_button, &QPushButton::pressed,
            [=]() {save(false);});
    connect(save_as_button, &QPushButton::pressed,
            [=]() {save(true);});
}

void EditDocumentInfo::set_pdf_info(const PdfInfo &pdf_info)
{
    AbstractOperation::set_pdf_info(pdf_info);

    m_title.setText(QString::fromStdString(m_pdf_info.title()));
    m_author.setText(QString::fromStdString(m_pdf_info.author()));
    m_subject.setText(QString::fromStdString(m_pdf_info.subject()));
    m_keywords.setText(QString::fromStdString(m_pdf_info.keywords()));
    m_creator.setText(QString::fromStdString(m_pdf_info.creator()));
    m_producer.setText(QString::fromStdString(m_pdf_info.producer()));

    if (m_pdf_info.has_creation_date())
    {
        m_creation_date_enabled.setChecked(true);
        m_creation_date.setEnabled(true);
        std::tm creation_date = m_pdf_info.creation_date();
        QDateTime dt{};
        dt.setOffsetFromUtc(0);
        dt.setDate(QDate{creation_date.tm_year + 1900, creation_date.tm_mon + 1,
                         creation_date.tm_mday});
        dt.setTime(QTime{creation_date.tm_hour, creation_date.tm_min});
        m_creation_date.setDateTime(dt.toLocalTime());
    }
    else
    {
        m_creation_date_enabled.setChecked(false);
        m_creation_date.setEnabled(false);
        m_creation_date.setDateTime(QDateTime::currentDateTime());
    }

    if (m_pdf_info.has_mod_date())
    {
        m_mod_date_enabled.setChecked(true);
        m_mod_date.setEnabled(true);
        std::tm mod_date = m_pdf_info.mod_date();
        QDateTime dt{};
        dt.setOffsetFromUtc(0);
        dt.setDate(QDate{mod_date.tm_year + 1900, mod_date.tm_mon + 1,
                         mod_date.tm_mday});
        dt.setTime(QTime{mod_date.tm_hour, mod_date.tm_min});
        m_mod_date.setDateTime(dt.toLocalTime());
    }
    else
    {
        m_mod_date_enabled.setChecked(false);
        m_mod_date.setEnabled(false);
        m_mod_date.setDateTime(QDateTime::currentDateTime());
    }
}

int EditDocumentInfo::output_pages_count()
{
    return m_pdf_info.n_pages();
}

void EditDocumentInfo::save(bool save_as)
{
    if (save_as)
    {
        if (!show_save_as_dialog())
            return;
    }
    else
    {
        if (!show_overwrite_dialog())
            return;
    }

    emit write_started();

    try
    {
        std::locale old_locale{std::locale::global(std::locale::classic())};

        QPDF qpdf;
        qpdf.processFile(m_pdf_info.filename().c_str());

        using handle = QPDFObjectHandle;
        handle doc_info = qpdf.makeIndirectObject(handle::newDictionary());

        if (!m_title.text().isEmpty())
            doc_info.replaceKey("/Title",
                                handle::newUnicodeString(
                                    m_title.text().toStdString()));

        if (!m_author.text().isEmpty())
            doc_info.replaceKey("/Author",
                                handle::newUnicodeString(
                                    m_author.text().toStdString()));

        if (!m_subject.text().isEmpty())
            doc_info.replaceKey("/Subject",
                                handle::newUnicodeString(
                                    m_subject.text().toStdString()));

        if (!m_keywords.text().isEmpty())
            doc_info.replaceKey("/Keywords",
                                handle::newUnicodeString(
                                    m_keywords.text().toStdString()));

        if (!m_creator.text().isEmpty())
            doc_info.replaceKey("/Creator",
                                handle::newUnicodeString(
                                    m_creator.text().toStdString()));

        if (!m_producer.text().isEmpty())
            doc_info.replaceKey("/Producer",
                                handle::newUnicodeString(
                                    m_producer.text().toStdString()));

        if (m_creation_date_enabled.isChecked())
        {
            QDateTime creation_date = m_creation_date.dateTime().toUTC();
            QString str = creation_date.toString("D:yyyyMMddHHmmssZ00");
            doc_info.replaceKey("/CreationDate",
                                handle::newUnicodeString(str.toStdString()));
        }

        if (m_mod_date_enabled.isChecked())
        {
            QDateTime mod_date = m_mod_date.dateTime().toUTC();
            QString str = mod_date.toString("D:yyyyMMddHHmmssZ00");
            doc_info.replaceKey("/ModDate",
                                handle::newUnicodeString(str.toStdString()));
        }

        qpdf.getTrailer().replaceKey("/Info", doc_info);

        emit progress_changed(20);

        QPDFWriter writer(qpdf);
        writer.setOutputMemory();
        writer.write();
        Buffer *buffer = writer.getBuffer();

        const char *buf = reinterpret_cast<const char *>(buffer->getBuffer());

        emit progress_changed(70);

        // write the PDF file to disk
        std::ofstream output_file_stream;
        output_file_stream.open(m_save_filename.toStdString());
        output_file_stream.write(buf, buffer->getSize());
        output_file_stream.close();
        delete buffer;

        std::locale::global(old_locale);


        emit write_finished(m_save_filename);
    }
    catch (std::exception &e)
    {
        emit write_error(QString::fromStdString(e.what()));
    }

}
