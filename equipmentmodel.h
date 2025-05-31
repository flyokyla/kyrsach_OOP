/**
 * @file equipmentmodel.h
 * @brief Модель данных для управления записями офисного оборудования
 * @author Капытин Андрей Игоревич ИЦТМС 2-1
 * @date 2025
 */

#ifndef EQUIPMENTMODEL_H
#define EQUIPMENTMODEL_H

#include <QAbstractTableModel>
#include <QVector>
#include "equipmentrecord.h"

/**
 * @class EquipmentModel
 * @brief Модель данных оборудования, наследующая QAbstractTableModel
 * 
 * Реализует паттерн MVC для отображения и управления данными офисного оборудования.
 * Поддерживает операции загрузки/сохранения файлов, добавления, редактирования 
 * и удаления записей.
 */
class EquipmentModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    /**
     * @brief Конструктор модели
     * @param parent Родительский объект
     */
    explicit EquipmentModel(QObject *parent = nullptr);

    // QAbstractTableModel interface
    /**
     * @brief Возвращает количество строк в модели
     * @param parent Родительский индекс (не используется)
     * @return Количество записей оборудования
     */
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    
    /**
     * @brief Возвращает количество столбцов в модели
     * @param parent Родительский индекс (не используется)
     * @return Количество полей в записи (9)
     */
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    
    /**
     * @brief Возвращает данные для отображения
     * @param index Индекс ячейки
     * @param role Роль данных (отображение, редактирование и т.д.)
     * @return Данные для отображения или QVariant()
     */
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    
    /**
     * @brief Возвращает заголовки столбцов
     * @param section Номер секции (столбца)
     * @param orientation Ориентация заголовка
     * @param role Роль данных
     * @return Текст заголовка
     */
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    
    /**
     * @brief Проверяет возможность редактирования ячейки
     * @param index Индекс ячейки
     * @return Флаги элемента
     */
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    
    /**
     * @brief Устанавливает данные в ячейку
     * @param index Индекс ячейки
     * @param value Новое значение
     * @param role Роль данных
     * @return true если данные успешно установлены
     */
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    // Методы управления данными
    /**
     * @brief Загружает данные из файла
     * @param fileName Путь к файлу
     * @return true если загрузка успешна
     */
    bool loadFromFile(const QString &fileName);
    
    /**
     * @brief Сохраняет данные в файл
     * @param fileName Путь к файлу
     * @return true если сохранение успешно
     */
    bool saveToFile(const QString &fileName);
    
    /**
     * @brief Добавляет новую запись оборудования
     * @param record Запись для добавления
     */
    void addRecord(const EquipmentRecord &record);
    
    /**
     * @brief Удаляет запись по индексу
     * @param row Номер строки для удаления
     * @return true если удаление успешно
     */
    bool removeRecord(int row);
    
    /**
     * @brief Возвращает запись по индексу
     * @param row Номер строки
     * @return Запись оборудования
     */
    EquipmentRecord getRecord(int row) const;
    
    /**
     * @brief Обновляет запись по индексу
     * @param row Номер строки
     * @param record Новая запись
     * @return true если обновление успешно
     */
    bool updateRecord(int row, const EquipmentRecord &record);
    
    /**
     * @brief Очищает все данные модели
     */
    void clear();
    
    /**
     * @brief Проверяет уникальность ID
     * @param id ID для проверки
     * @param excludeRow Исключить из проверки указанную строку (-1 для проверки всех)
     * @return true если ID уникален
     */
    bool isIdUnique(const QString &id, int excludeRow = -1) const;

    /**
     * @brief Возвращает все записи оборудования
     * @return Ссылка на вектор записей
     */
    const QVector<EquipmentRecord>& getAllRecords() const;

    /**
     * @brief Обновляет заголовки столбцов для многоязычности
     */
    void updateHeaders();

private:
    QVector<EquipmentRecord> m_records;  ///< Контейнер для хранения записей оборудования
    QStringList m_headers;               ///< Заголовки столбцов таблицы
};

#endif // EQUIPMENTMODEL_H
