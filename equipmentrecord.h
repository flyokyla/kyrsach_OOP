/**
 * @file equipmentrecord.h
 * @brief Структура данных для записи офисного оборудования
 * @author Капытин Андрей Игоревич ИЦТМС 2-1
 * @date 2025
 */

#ifndef EQUIPMENTRECORD_H
#define EQUIPMENTRECORD_H

#include <QString>
#include <QDate>

/**
 * @struct EquipmentRecord
 * @brief Структура для хранения информации об единице офисного оборудования
 * 
 * Содержит полную информацию о единице оборудования включая:
 * идентификационные данные, характеристики, финансовую информацию,
 * местоположение и текущий статус.
 */
struct EquipmentRecord
{
    QString id;           ///< Уникальный идентификатор оборудования
    QString type;         ///< Тип оборудования (компьютер, принтер, мебель и т.д.)
    QString model;        ///< Модель оборудования
    QString serial;       ///< Серийный номер
    QDate purchase_date;  ///< Дата покупки
    double price;         ///< Цена приобретения
    QString location;     ///< Местоположение (офис, этаж, комната)
    QString status;       ///< Текущий статус (рабочее, на ремонте, списано)
    QString notes;        ///< Дополнительные заметки и комментарии

    /**
     * @brief Конструктор по умолчанию
     * 
     * Создает пустую запись с базовыми значениями:
     * - price = 0.0
     * - purchase_date = текущая дата
     * - status = "Working"
     */
    EquipmentRecord() : price(0.0), purchase_date(QDate::currentDate()), status("Working") {}

    /**
     * @brief Преобразует запись в строку для сохранения в файл
     * @return Строка в формате: id|type|model|serial|date|price|location|status|notes
     */
    QString toString() const;

    /**
     * @brief Создает запись из строки, загруженной из файла
     * @param str Строка в формате: id|type|model|serial|date|price|location|status|notes
     * @return Объект EquipmentRecord или пустой объект при ошибке парсинга
     */
    static EquipmentRecord fromString(const QString &str);

    /**
     * @brief Проверяет корректность заполнения всех обязательных полей
     * @return true если все обязательные поля заполнены корректно
     */
    bool isValid() const;

    /**
     * @brief Оператор сравнения записей
     * @param other Другая запись для сравнения
     * @return true если записи идентичны
     */
    bool operator==(const EquipmentRecord &other) const;
};

#endif // EQUIPMENTRECORD_H
