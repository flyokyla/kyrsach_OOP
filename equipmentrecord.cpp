#include "equipmentrecord.h"
#include <QStringList>

EquipmentRecord EquipmentRecord::fromString(const QString& line) {
    EquipmentRecord record;
    QStringList parts = line.split(';'); // Используем точку с запятой как разделитель
    
    if (parts.size() >= 9) {
        record.id = parts[0];
        record.type = parts[1];
        record.model = parts[2];
        record.serial = parts[3];
        record.purchase_date = QDate::fromString(parts[4], "yyyy-MM-dd");
        record.price = parts[5].toDouble();
        record.location = parts[6];
        record.status = parts[7];
        record.notes = parts[8];
    }
    
    return record;
}

QString EquipmentRecord::toString() const {
    return QString("%1;%2;%3;%4;%5;%6;%7;%8;%9")
            .arg(id)
            .arg(type)
            .arg(model)
            .arg(serial)
            .arg(purchase_date.toString("yyyy-MM-dd"))
            .arg(price)
            .arg(location)
            .arg(status)
            .arg(notes);
}

bool EquipmentRecord::isValid() const {
    return !id.isEmpty() && !type.isEmpty() && !model.isEmpty() && 
           purchase_date.isValid() && price >= 0;
}

bool EquipmentRecord::operator==(const EquipmentRecord &other) const {
    return id == other.id && 
           type == other.type && 
           model == other.model && 
           serial == other.serial &&
           purchase_date == other.purchase_date && 
           price == other.price && 
           location == other.location && 
           status == other.status && 
           notes == other.notes;
}
