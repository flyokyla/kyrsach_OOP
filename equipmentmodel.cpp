#include "equipmentmodel.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QCoreApplication>
#include <QDate>
#include <QClipboard>
#include <QApplication>

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
    Qt::ItemFlags defaultFlags = QAbstractTableModel::flags(index);
    if (index.isValid())
        return defaultFlags | Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
    return defaultFlags | Qt::ItemIsDropEnabled;
}

Qt::DropActions EquipmentModel::supportedDropActions() const {
    return Qt::CopyAction | Qt::MoveAction;
}

Qt::DropActions EquipmentModel::supportedDragActions() const {
    return Qt::CopyAction | Qt::MoveAction;
}

QStringList EquipmentModel::mimeTypes() const {
    QStringList types;
    types << "application/vnd.text.list"
          << "text/csv"
          << "text/plain";
    return types;
}

QMimeData* EquipmentModel::mimeData(const QModelIndexList &indexes) const {
    QMimeData *mimeData = new QMimeData();
    QString csv = exportToCSV(indexes);
    mimeData->setData("text/csv", csv.toUtf8());
    mimeData->setData("text/plain", csv.toUtf8());
    
    // Сохраняем структурированные данные для внутреннего перетаскивания
    QByteArray encodedData;
    QDataStream stream(&encodedData, QIODevice::WriteOnly);
    
    // Запись количества строк и столбцов
    QMap<int, QMap<int, QVariant>> cellData;
    
    foreach (QModelIndex index, indexes) {
        if (index.isValid()) {
            cellData[index.row()][index.column()] = data(index, Qt::DisplayRole);
        }
    }
    
    stream << cellData;
    mimeData->setData("application/vnd.text.list", encodedData);
    
    return mimeData;
}

bool EquipmentModel::dropMimeData(const QMimeData *data, Qt::DropAction action,
                                 int row, int column, const QModelIndex &parent) {
    if (action == Qt::IgnoreAction)
        return true;
    
    // Если это внутреннее перетаскивание (между табами)
    if (data->hasFormat("application/vnd.text.list")) {
        QByteArray encodedData = data->data("application/vnd.text.list");
        QDataStream stream(&encodedData, QIODevice::ReadOnly);
        
        QMap<int, QMap<int, QVariant>> cellData;
        stream >> cellData;
        
        int targetRow = row;
        if (targetRow < 0) {
            if (parent.isValid())
                targetRow = parent.row();
            else
                targetRow = rowCount();
        }
        
        // Вставляем новые строки для данных, если нужно
        int requiredRows = targetRow + cellData.size();
        while (rowCount() < requiredRows) {
            EquipmentRecord newRecord;
            newRecord.id = generateUniqueId();
            addRecord(newRecord);
        }
        
        // Копируем данные
        if (!cellData.isEmpty()) {
            int firstRowKey = cellData.firstKey();
            
            for (auto rowIt = cellData.begin(); rowIt != cellData.end(); ++rowIt) {
                int currentRow = targetRow + (rowIt.key() - firstRowKey);
                QMap<int, QVariant> columnData = rowIt.value();
                
                if (!columnData.isEmpty()) {
                    int firstColKey = columnData.firstKey();
                    
                    for (auto colIt = columnData.begin(); colIt != columnData.end(); ++colIt) {
                        int targetCol = column >= 0 ? column + (colIt.key() - firstColKey) : colIt.key();
                        setData(index(currentRow, targetCol), colIt.value());
                    }
                }
            }
        }
        
        return true;
    }
    
    // Импорт из CSV
    if (data->hasFormat("text/csv") || data->hasFormat("text/plain")) {
        QString csvData = QString::fromUtf8(data->data(
            data->hasFormat("text/csv") ? "text/csv" : "text/plain"));
            
        int targetRow = row;
        if (targetRow < 0) {
            if (parent.isValid())
                targetRow = parent.row();
            else
                targetRow = rowCount();
        }
        
        int targetCol = column;
        if (targetCol < 0 && parent.isValid()) {
            targetCol = parent.column();
        }
        
        return importFromCSV(csvData, targetRow, targetCol);
    }
    
    return false;
}

QString EquipmentModel::exportToCSV(const QModelIndexList &indexes) const {
    // Находим границы выделенной области
    int minRow = -1, maxRow = -1, minCol = -1, maxCol = -1;
    
    foreach (QModelIndex index, indexes) {
        if (!index.isValid())
            continue;
            
        if (minRow == -1 || index.row() < minRow)
            minRow = index.row();
            
        if (maxRow == -1 || index.row() > maxRow)
            maxRow = index.row();
            
        if (minCol == -1 || index.column() < minCol)
            minCol = index.column();
            
        if (maxCol == -1 || index.column() > maxCol)
            maxCol = index.column();
    }
    
    if (minRow == -1 || maxRow == -1 || minCol == -1 || maxCol == -1)
        return QString();
        
    // Создаем CSV из выделенных ячеек
    QString csv;
    QTextStream stream(&csv, QIODevice::WriteOnly);
    
    for (int row = minRow; row <= maxRow; ++row) {
        QStringList rowData;
        
        for (int col = minCol; col <= maxCol; ++col) {
            QModelIndex idx = index(row, col);
            QVariant value = data(idx, Qt::DisplayRole);
            
            // Экранируем двойные кавычки и обрамляем поле кавычками если есть запятые
            QString cellData = value.toString();
            if (cellData.contains('"'))
                cellData.replace("\"", "\"\"");
                
            if (cellData.contains(',') || cellData.contains('\n') || cellData.contains('"'))
                cellData = "\"" + cellData + "\"";
                
            rowData << cellData;
        }
        
        stream << rowData.join(",") << "\n";
    }
    
    return csv;
}

bool EquipmentModel::importFromCSV(const QString &csvData, int targetRow, int targetCol) {
    if (csvData.isEmpty())
        return false;
        
    QStringList rows = csvData.split('\n', Qt::SkipEmptyParts);
    if (rows.isEmpty())
        return false;
        
    // Проверяем, нужно ли добавить новые строки
    int requiredRows = targetRow + rows.size();
    while (rowCount() < requiredRows) {
        EquipmentRecord newRecord;
        newRecord.id = generateUniqueId();
        addRecord(newRecord);
    }
    
    // Парсим и вставляем данные из CSV
    int currentRow = targetRow;
    
    foreach (const QString &row, rows) {
        // Простое разделение по запятым (улучшенный парсер учтет кавычки и экранирование)
        QStringList fields = parseCSVRow(row);
        
        int currentCol = targetCol;
        foreach (const QString &field, fields) {
            if (currentCol < columnCount()) {
                setData(index(currentRow, currentCol), field);
                currentCol++;
            }
        }
        
        currentRow++;
        if (currentRow >= rowCount())
            break;
    }
    
    return true;
}

// Вспомогательный метод для правильного разбора строки CSV
QStringList EquipmentModel::parseCSVRow(const QString &row) const {
    QStringList fields;
    QString field;
    bool inQuotes = false;
    
    for (int i = 0; i < row.length(); i++) {
        QChar c = row[i];
        
        // Обработка кавычек
        if (c == '"') {
            if (i < row.length() - 1 && row[i+1] == '"') {
                // Экранированная кавычка
                field += '"';
                i++; // Пропускаем вторую кавычку
            } else {
                // Переключаем флаг кавычек
                inQuotes = !inQuotes;
            }
            continue;
        }
        
        // Обработка запятых
        if (c == ',' && !inQuotes) {
            fields.append(field);
            field.clear();
            continue;
        }
        
        // Добавляем символ в поле
        field += c;
    }
    
    // Добавляем последнее поле
    fields.append(field);
    
    return fields;
}

QString EquipmentModel::generateUniqueId() const {
    int maxId = 0;
    
    foreach (const EquipmentRecord &record, m_records) {
        bool ok;
        int id = record.id.toInt(&ok);
        if (ok && id > maxId) {
            maxId = id;
        }
    }
    
    return QString::number(maxId + 1);
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

bool EquipmentModel::hasEquipmentId(const QString &id) const {
    return !isIdUnique(id, -1);
}
