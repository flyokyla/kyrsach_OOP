#include "equipmentdialog.h"
#include <QVBoxLayout>
#include <QMessageBox>
#include <QDate>

equipmentdialog::equipmentdialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi();
    setWindowTitle(tr("Add Equipment"));
}

equipmentdialog::equipmentdialog(const EquipmentRecord &record, QWidget *parent)
    : QDialog(parent)
{
    setupUi();
    setRecord(record);
    setWindowTitle(tr("Edit Equipment"));
}

void equipmentdialog::setupUi()
{
    setModal(true);
    resize(400, 500);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QFormLayout *formLayout = new QFormLayout();

    // ID
    m_idEdit = new QLineEdit();
    m_idEdit->setReadOnly(true);
    formLayout->addRow(tr("ID:"), m_idEdit);

    // Type
    m_typeCombo = new QComboBox();
    m_typeCombo->addItems({"Printer", "Monitor", "Desktop PC", "Laptop", "Plotter", "Scanner", 
                          "Projector", "Tablet", "UPS", "Router", "Phone", "Server", 
                          "Keyboard", "Mouse", "Webcam", "Headset", "External HDD", 
                          "Network Switch", "Label Printer", "Shredder"});
    m_typeCombo->setEditable(true);
    formLayout->addRow(tr("Type:"), m_typeCombo);

    // Model
    m_modelEdit = new QLineEdit();
    formLayout->addRow(tr("Model:"), m_modelEdit);

    // Serial Number
    m_serialEdit = new QLineEdit();
    formLayout->addRow(tr("Serial Number:"), m_serialEdit);

    // Purchase Date
    m_dateEdit = new QDateEdit();
    m_dateEdit->setDate(QDate::currentDate());
    m_dateEdit->setCalendarPopup(true);
    formLayout->addRow(tr("Purchase Date:"), m_dateEdit);

    // Price
    m_priceSpinBox = new QDoubleSpinBox();
    m_priceSpinBox->setRange(0.0, 999999.99);
    m_priceSpinBox->setDecimals(2);
    m_priceSpinBox->setSuffix(" $");
    formLayout->addRow(tr("Price:"), m_priceSpinBox);

    // Location
    m_locationEdit = new QLineEdit();
    formLayout->addRow(tr("Location:"), m_locationEdit);

    // Status
    m_statusCombo = new QComboBox();
    m_statusCombo->addItems({"Active", "Inactive", "Maintenance", "Retired"});
    formLayout->addRow(tr("Status:"), m_statusCombo);

    // Description
    m_descriptionEdit = new QTextEdit();
    m_descriptionEdit->setMaximumHeight(100);
    formLayout->addRow(tr("Description:"), m_descriptionEdit);

    mainLayout->addLayout(formLayout);

    // Buttons
    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &equipmentdialog::accept);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainLayout->addWidget(m_buttonBox);
}

void equipmentdialog::setRecord(const EquipmentRecord &record)
{
    m_idEdit->setText(record.id);
    m_typeCombo->setCurrentText(record.type);
    m_modelEdit->setText(record.model);
    m_serialEdit->setText(record.serial);
    m_dateEdit->setDate(record.purchase_date);
    m_priceSpinBox->setValue(record.price);
    m_locationEdit->setText(record.location);
    m_statusCombo->setCurrentText(record.status);
    m_descriptionEdit->setPlainText(record.notes);
}

EquipmentRecord equipmentdialog::getRecord() const
{
    EquipmentRecord record;
    record.id = m_idEdit->text();
    record.type = m_typeCombo->currentText();
    record.model = m_modelEdit->text();
    record.serial = m_serialEdit->text();
    record.purchase_date = m_dateEdit->date();
    record.price = m_priceSpinBox->value();
    record.location = m_locationEdit->text();
    record.status = m_statusCombo->currentText();
    record.notes = m_descriptionEdit->toPlainText();
    return record;
}

bool equipmentdialog::validateInput()
{
    if (m_typeCombo->currentText().isEmpty()) {
        QMessageBox::warning(this, tr("Validation Error"), tr("Type cannot be empty."));
        m_typeCombo->setFocus();
        return false;
    }

    if (m_modelEdit->text().isEmpty()) {
        QMessageBox::warning(this, tr("Validation Error"), tr("Model cannot be empty."));
        m_modelEdit->setFocus();
        return false;
    }

    if (m_serialEdit->text().isEmpty()) {
        QMessageBox::warning(this, tr("Validation Error"), tr("Serial number cannot be empty."));
        m_serialEdit->setFocus();
        return false;
    }

    if (m_locationEdit->text().isEmpty()) {
        QMessageBox::warning(this, tr("Validation Error"), tr("Location cannot be empty."));
        m_locationEdit->setFocus();
        return false;
    }

    return true;
}

void equipmentdialog::accept()
{
    if (validateInput()) {
        QDialog::accept();
    }
}
