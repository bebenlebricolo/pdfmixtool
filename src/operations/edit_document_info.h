#ifndef EDITDOCUMENTINFO_H
#define EDITDOCUMENTINFO_H

#include <QLineEdit>
#include <QDateTimeEdit>
#include <QCheckBox>

#include "abstract_operation.h"

class EditDocumentInfo : public AbstractOperation
{
    Q_OBJECT
public:
    explicit EditDocumentInfo(QWidget *parent = nullptr);

    void set_pdf_info(const PdfInfo &pdf_info) override;

    int output_pages_count() override;

private:
    QLineEdit m_title;
    QLineEdit m_author;
    QLineEdit m_subject;
    QLineEdit m_keywords;
    QLineEdit m_creator;
    QLineEdit m_producer;
    QDateTimeEdit m_creation_date;
    QDateTimeEdit m_mod_date;
    QCheckBox m_creation_date_enabled;
    QCheckBox m_mod_date_enabled;

    void save(bool save_as);
};

#endif // EDITDOCUMENTINFO_H
