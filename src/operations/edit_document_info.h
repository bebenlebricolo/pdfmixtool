#ifndef EDITDOCUMENTINFO_H
#define EDITDOCUMENTINFO_H

#include <QLineEdit>
#include <QDateTimeEdit>

#include "abstract_operation.h"

class EditDocumentInfo : public AbstractOperation
{
    Q_OBJECT
public:
    explicit EditDocumentInfo(const PdfInfo &pdf_info,
                              QWidget *parent = nullptr);

public slots:
    void pdf_info_changed() override;

private:
    QLineEdit m_title;
    QLineEdit m_author;
    QLineEdit m_subject;
    QLineEdit m_keywords;
    QLineEdit m_creator;
    QLineEdit m_producer;
    QDateTimeEdit m_creation_date;
    QDateTimeEdit m_mod_date;

    void save(bool save_as);
};

#endif // EDITDOCUMENTINFO_H
