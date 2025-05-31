#include "equipmentmodel.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QCoreApplication>
#include <QDate>

EquipmentModel::EquipmentModel(QObject *parent) : QAbstractTableModel(parent) {
    updateHeaders();
}

void EquipmentModel::updateHeaders() {
    m_headers.clear();
    m_headers << tr("ID") << tr("Type") << tr("Model") << tr("Serial") 
              << tr("Purchase Date") << tr("Price") << tr("Location") 
              << tr("Status") << tr("Notes");
    
    emit headerDataChanged(Qt::Horizontal, 0, columnCount() - 1);
}

int EquipmentModel::rowCount(const QModelIndex&) const {
    return m_records.size();
}

int EquipmentModel::columnCount(const QModelIndex&) const {
    return m_headers.size();
}

QVariant EquipmentModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= m_records.size())
        return QVariant();

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        const EquipmentRecord &record = m_records[index.row()];
        switch (index.column()) {
        case 0: return record.id;
        case 1: return record.type;
        case 2: return record.model;
        case 3: return record.serial;
        case 4: return record.purchase_date;
        case 5: return record.price;
        case 6: return record.location;
        case 7: return record.status;
        case 8: return record.notes;
        }
    }
    return QVariant();
}

QVariant EquipmentModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
        return m_headers[section];
    return QVariant();
}

bool EquipmentModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (role != Qt::EditRole || !index.isValid() || index.row() >= m_records.size())
        return false;

    EquipmentRecord &record = m_records[index.row()];
    switch (index.column()) {
    case 0: record.id = value.toString(); break;
    case 1: record.type = value.toString(); break;
    case 2: record.model = value.toString(); break;
    case 3: record.serial = value.toString(); break;
    case 4: {
        // Валидация даты в формате YYYY-MM-DD
        QString dateStr = value.toString();
        QDate date = QDate::fromString(dateStr, Qt::ISODate);
        if (date.isValid()) {
            record.purchase_date = date;
        } else {
            // Если дата невалидна, оставляем старое значение
            return false;
        }
        break;
    }
    case 5: record.price = value.toDouble(); break;
    case 6: record.location = value.toString(); break;
    case 7: record.status = value.toString(); break;
    case 8: record.notes = value.toString(); break;
    default: return false;
    }

    emit dataChanged(index, index);
    return true;
}

Qt::ItemFlags EquipmentModel::flags(const QModelIndex &index) const {
    Qt::ItemFlags flags = QAbstractTableModel::flags(index);
    if (index.isValid())
        flags |= Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
    return flags;
}

bool EquipmentModel::loadFromFile(const QString& filename) {
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Cannot open file:" << filename;
        return false;
    }

    QTextStream in(&file);
    if (in.readLine() != "HEADER:OfficeEquipmentDBv1") {
        qWarning() << "Invalid file format";
        return false;
    }

    beginResetModel();
    m_records.clear();
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (!line.isEmpty()) {
            EquipmentRecord record = EquipmentRecord::fromString(line);
            m_records.append(record);
        }
    }
    endResetModel();
    return true;
}

bool EquipmentModel::saveToFile(const QString& filename) {
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Cannot open file for writing:" << filename;
        return false;
    }

    QTextStream out(&file);
    out << "HEADER:OfficeEquipmentDBv1\n";
    for (const EquipmentRecord &record : m_records) {
        out << record.toString() << "\n";
    }
    return true;
}

void EquipmentModel::addRecord(const EquipmentRecord& record) {
    beginInsertRows(QModelIndex(), m_records.size(), m_records.size());
    m_records.append(record);
    endInsertRows();
}

bool EquipmentModel::updateRecord(int row, const EquipmentRecord& record) {
    if (row < 0 || row >= m_records.size())
        return false;

    m_records[row] = record;
    emit dataChanged(index(row, 0), index(row, columnCount()-1));
    return true;
}

bool EquipmentModel::removeRecord(int row) {
    if (row < 0 || row >= m_records.size())
        return false;

    beginRemoveRows(QModelIndex(), row, row);
    m_records.remove(row);
    endRemoveRows();
    return true;
}

EquipmentRecord EquipmentModel::getRecord(int row) const {
    if (row >= 0 && row < m_records.size())
        return m_records[row];
    return EquipmentRecord();
}

const QVector<EquipmentRecord>& EquipmentModel::getAllRecords() const {
    return m_records;
}

void EquipmentModel::clear() {
    beginResetModel();
    m_records.clear();
    endResetModel();
}

bool EquipmentModel::isIdUnique(const QString &id, int excludeRow) const {
    for (int i = 0; i < m_records.size(); ++i) {
        if (i != excludeRow && m_records[i].id == id) {
            return false;
        }
    }
    return true;
}
