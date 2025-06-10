# ОТЧЕТ О СООТВЕТСТВИИ ТЕХНИЧЕСКИМ ТРЕБОВАНИЯМ

**Проект:** Разработка программного обеспечения для учета офисного оборудования строительной организации  
**Автор:** Капытин Андрей Игоревич ИЦТМС 2-1  
**Дата:** 2025  
**Версия:** 1.0  

## СВОДКА СООТВЕТСТВИЯ

**Статус выполнения:**
- ✅ **Выполнено полностью:** 25 пунктов
- 🔄 **Частично выполнено:** 0 пунктов
- ❌ **Не выполнено:** 0 пунктов

**Общий процент выполнения:** 100%

## ДЕТАЛЬНЫЙ АНАЛИЗ ПО ПУНКТАМ

### 23. Drag-and-drop между документами
**✅ ВЫПОЛНЕНО**
- **Файлы:** `equipmentmodel.cpp/.h`, `mainwindow.cpp/.h`
- **Реализация:**
  - Выделение нескольких ячеек (ExtendedSelection)
  - Перетаскивание между вкладками (DragDrop)
  - Копирование/вставка данных в CSV формате
  - Импорт/экспорт данных из CSV файлов
- **Функциональность:**
  - Перетаскивание данных между таблицами
  - Копирование данных в буфер обмена (Ctrl+C)
  - Вставка данных из буфера обмена (Ctrl+V)
  - Экспорт выделенных ячеек в CSV файл
  - Импорт данных из CSV файла

# ?? **   **

## **  :**       

---

## ?? **   :**

### 1. -   Qt
**? **
- **:** `CMakeLists.txt`  1-25
- **:**    C++  Qt6
- **:**  Qt : `Qt6::Core`, `Qt6::Widgets`, `Qt6::Charts`, `Qt6::PrintSupport`

### 2.    ()
**? **
- **I.  :** `equipmentmodel.cpp`  85-105
- **II.  :** `equipmentmodel.cpp`  107-130
- **III.  :** `equipmentmodel.cpp`  48-83
- **IV.  :** `equipmentmodel.cpp`  230-240
- **V.  :** `equipmentmodel.cpp`  155-200
- **VI.  :** `equipmentmodel.cpp`  241-255

### 3.   (?20 , ?8 )
**? **
- **:** `equipmentrecord.h`  7-22
- ** (9 ):** id, type, model, serial, purchase_date, price, location, status, notes
- ** :** `sample_data.txt`  25+ 

### 4.    
**? **
- **:** `equipmentmodel.cpp`  85-105
- **:**   "HEADER:OfficeEquipmentDBv1"
- **:**     

### 5.   (Doxygen)
**? **
- **:** Doxygen   Doxygen  .

### 6.    
**? **
- ** :** `MainWindow` (`mainwindow.cpp/.h/.ui`)
- **:** 
  - `aboutdialog` - 
  - `equipmentdialog` -  
  - `chartwindow` -  

### 7.    
**? **
- **:** `mainwindow.ui`  57-102
- **:** File, Edit, View, Language, Help
- **:** `mainwindow.cpp`  68-91

### 8.  
**? **
- **:** `mainwindow.ui`, `chartwindow.ui`, `equipmentdialog.ui`
- **:**  QVBoxLayout QHBoxLayout
- **:**      

### 9.  
**? **
- **:** `aboutdialog.cpp`  16-25
- **:**     2-1
- ** :** , , ,   

### 10.  " " 
**? **
- **:** `mainwindow.ui`  179 (actionAbout)
- **:** `mainwindow.cpp`  86
- **:** `aboutdialog.cpp/.h/.ui` -  

### 11.   
**? **
- **:** `equipmentdialog.cpp`  50-85
- **:** 
  -   
  -  ID  
  -     
  -     

### 12.   tr() *.qm 
**? **
- ** :** `translations/office_equipment_en.ts`, `office_equipment_ru.ts`, `office_equipment_es.ts`
- ** :**      tr()
- **UI :**     

### 13. Item based    
**? **
- **:** `equipmentmodel.h/.cpp`
- **:** QAbstractTableModel   
- **:** QTableView  

### 14.  Qt/STL  Item based
**? **
- **:** `equipmentmodel.h`  21
- **:** `QVector<EquipmentRecord> m_records`
- **:**      

### 15.   MVC
**? **
- **Model:** `EquipmentModel` (`equipmentmodel.h/.cpp`)
- **View:** `QTableView`  
- **Controller:** `MainWindow`  
- **Proxy:** `QSortFilterProxyModel`   

### 16.   
**? **
- **:** , , 
- **:** `mainwindow.ui` Language -> English/Russian/Spanish
- **:** `mainwindow.cpp`  switchToEnglish/Russian/Spanish
- ** :**   

### 17.    
**? **
- **:** `mainwindow.cpp`  `onHeaderClicked`, `filterBySelectedColumn`
- **:** 
  -        
  -      
  -      ID ( 0)
  - Placeholder     
- **:**   QHeaderView::sectionClicked

### 18.    
**? **
- **:** `QSortFilterProxyModel`    
- **:** `mainwindow.cpp`   
- **:**      

### 19.   (  )
**? **
- **:** `mainwindow.cpp`  showContextMenu
- **:**   "Edit" "Remove"
- **:**  customContextMenuRequested  QTableView

### 20.   
**? NENO**
- **:** `settings.h/.cpp`
- **:**  , , 
- **:** saveWindowGeometry, saveWindowState, saveLanguage
- **:**   

### 21.    
**? **
- **:** `mainwindow.h`  74 (QTabWidget)
- **:**     
- **:** `TabData`     
- **:** , ,   

### 22.    
**? **
- **:** 
  - `QSortFilterProxyModel`   
  -         
  -  `setupColumnFilters`, `updateColumnFilter`, `onColumnFilterChanged`
- **:**    
- **:**     
- **:** `mainwindow.cpp`   

### 24.   
**? **
- **:** `chartwindow.h/.cpp/.ui`
- ** :** 
  -    
  -  
  -   
- **:** Qt6::Charts   

---

## ??? ** :**

### ** :**
1. **MainWindow** -    
2. **EquipmentModel** -   (MVC)
3. **EquipmentDialog** -  
4. **ChartWindow** -   
5. **AboutDialog** -  
6. **Settings** -  

### ** :**
```
pr-oftech-main/
??? main.cpp                    #  
??? mainwindow.h/.cpp/.ui       #  
??? equipmentmodel.h/.cpp       #  
??? equipmentrecord.h/.cpp      #  
??? equipmentdialog.h/.cpp/.ui  #  
??? chartwindow.h/.cpp/.ui      #  
??? aboutdialog.h/.cpp/.ui      #  " "
??? settings.h/.cpp             #  
??? translations/               #  
?   ??? office_equipment_en.ts
?   ??? office_equipment_ru.ts
?   ??? office_equipment_es.ts
??? CMakeLists.txt              #  
```

---

## ? ** :** 
**   !** 