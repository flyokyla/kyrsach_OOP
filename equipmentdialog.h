#ifndef EQUIPMENTDIALOG_H
#define EQUIPMENTDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QDateEdit>
#include <QDoubleSpinBox>
#include <QTextEdit>
#include <QFormLayout>
#include <QDialogButtonBox>
#include "equipmentmodel.h"

namespace Ui {
class equipmentdialog;
}

class equipmentdialog : public QDialog
{
    Q_OBJECT

public:
    explicit equipmentdialog(QWidget *parent = nullptr);
    explicit equipmentdialog(const EquipmentRecord &record, QWidget *parent = nullptr);

    EquipmentRecord getRecord() const;

private slots:
    void accept() override;

private:
    void setupUi();
    void setRecord(const EquipmentRecord &record);
    bool validateInput();

    QLineEdit *m_idEdit;
    QComboBox *m_typeCombo;
    QLineEdit *m_modelEdit;
    QLineEdit *m_serialEdit;
    QDateEdit *m_dateEdit;
    QDoubleSpinBox *m_priceSpinBox;
    QLineEdit *m_locationEdit;
    QComboBox *m_statusCombo;
    QTextEdit *m_descriptionEdit;
    QDialogButtonBox *m_buttonBox;
};

#endif // EQUIPMENTDIALOG_H
