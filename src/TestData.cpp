#include "TestData.h"
#include "Database.h"
#include <sqlite3.h>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <vector>

void prepareTestData()
{
    const std::string databaseFile = "notification_queries.db";
    
    std::filesystem::remove(databaseFile);

    auto storage = createStorage(databaseFile);
    
    storage.on_open = [](sqlite3* db) {
        const int rc = sqlite3_exec(db, "PRAGMA foreign_keys = ON;",
                                    nullptr, nullptr, nullptr);
        if (rc != SQLITE_OK)
            throw std::runtime_error("Не вдалося увімкнути зовнішні ключі");
    };
    storage.sync_schema();

    const std::vector<NotificationCategory> categories = {
        {0, "Безпека", "Попередження про безпеку облікового запису", "email", 1},
        {0, "Замовлення", "Статуси та зміни замовлень", "push", 2},
        {0, "Платежі", "Інформація про оплату та повернення", "sms", 1},
        {0, "Новини", "Новини та інформаційні повідомлення", "in-app", 4},
        {0, "Система", "Технічні повідомлення системи", "email", 3}
    };

    std::vector<int> categoryIds;
    
    for (const auto& category : categories)
        categoryIds.push_back(storage.insert(category));

    const std::vector<Notification> notifications = {
        {0, categoryIds[0], "Новий вхід", "Виявлено вхід із нового пристрою", "2026-11-01 08:15", 1},
        {0, categoryIds[0], "Зміна пароля", "Пароль облікового запису змінено", "2026-11-02 10:20", 1},
        {0, categoryIds[0], "Підозріла активність", "Зафіксовано незвичну активність", "2026-11-03 21:05", 1},
        
        {0, categoryIds[1], "Замовлення прийнято", "Замовлення №104 прийнято", "2026-11-01 09:00", 2},
        {0, categoryIds[1], "Замовлення відправлено", "Замовлення №104 передано перевізнику", "2026-11-02 14:30", 2},
        {0, categoryIds[1], "Замовлення доставлено", "Замовлення №104 доставлено", "2026-11-04 12:10", 3},
        
        {0, categoryIds[2], "Оплату отримано", "Платіж за замовлення №104 успішний", "2026-11-01 09:05", 1},
        {0, categoryIds[2], "Повернення коштів", "Повернення коштів виконано", "2026-11-03 16:45", 2},
        {0, categoryIds[2], "Нагадування про оплату", "Очікується оплата замовлення №108", "2026-11-05 11:00", 2},
        
        {0, categoryIds[3], "Оновлення каталогу", "Додано нові товари до каталогу", "2026-11-01 13:00", 4},
        {0, categoryIds[3], "Листопадова пропозиція", "Доступна нова спеціальна пропозиція", "2026-11-02 08:30", 4},
        {0, categoryIds[3], "Опитування користувачів", "Поділіться враженнями про сервіс", "2026-11-06 17:20", 5},
        
        {0, categoryIds[4], "Технічні роботи", "Заплановано коротке обслуговування", "2026-11-03 19:00", 3},
        {0, categoryIds[4], "Оновлення системи", "Систему успішно оновлено", "2026-11-04 06:40", 3},
        {0, categoryIds[4], "Збій синхронізації", "Не вдалося синхронізувати дані", "2026-11-07 07:55", 2}
    };

    for (const auto& notification : notifications)
        storage.insert(notification);

    std::cout << "Тестові дані створено. Категорій: "
              << storage.count<NotificationCategory>()
              << ", сповіщень: " << storage.count<Notification>() << '\n';
}
