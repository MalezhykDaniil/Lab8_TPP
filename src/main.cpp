#include <iostream>
#include <stdexcept>
#include <string>
#include <optional>
#include <filesystem>
#include <sqlite3.h>
#include "NotificationCategory.h"
#include "Notification.h"
#include "Database.h"
#include "INotificationCategoryRepository.h"
#include "SQLiteNotificationCategoryRepository.h"
#include "TestData.h"

int insertNotificationCategory(
    sqlite3* db,
    const NotificationCategory& category)
{
    const char* sql = R"(
        INSERT INTO NotificationCategory (name, description, channel, defaultPriority)
        VALUES (?, ?, ?, ?);
)";

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(
        db,
        sql,
        -1,
        &stmt,
        nullptr
    );

    if (rc != SQLITE_OK)
    {
        throw std::runtime_error(sqlite3_errmsg(db));
    }

    rc = sqlite3_bind_text(stmt, 1, category.name.c_str(), -1, SQLITE_TRANSIENT);

    if (rc == SQLITE_OK)
    {
        rc = sqlite3_bind_text(stmt, 2, category.description.c_str(), -1, SQLITE_TRANSIENT);
    }

    if (rc == SQLITE_OK)
    {
        rc = sqlite3_bind_text(stmt, 3, category.channel.c_str(), -1, SQLITE_TRANSIENT);
    }

    if (rc == SQLITE_OK)
    {
        rc = sqlite3_bind_int(stmt, 4, category.defaultPriority);
    }
    
    if (rc != SQLITE_OK) 
    {
        std::string error = sqlite3_errmsg(db);
        sqlite3_finalize(stmt);
        throw std::runtime_error(error);
    }
    
    rc = sqlite3_step(stmt);
    
    if (rc != SQLITE_DONE) 
    {
        std::string error = sqlite3_errmsg(db);
        sqlite3_finalize(stmt);
        throw std::runtime_error(error);
    }
    
    int id = static_cast<int>(sqlite3_last_insert_rowid(db));
    sqlite3_finalize(stmt);
    
    return id;
}

std::optional<NotificationCategory> findNotificationCategoryById(
    sqlite3* db, 
    int id)
{
    const char* sql = R"(
        SELECT id, name, description, channel, defaultPriority
        FROM NotificationCategory 
        WHERE id = ?;
    )";
    
    sqlite3_stmt* stmt = nullptr;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    
    if (rc != SQLITE_OK)
    {
        throw std::runtime_error(sqlite3_errmsg(db));
    }

    rc = sqlite3_bind_int(stmt, 1, id);

    if (rc != SQLITE_OK)
    {
        std::string error = sqlite3_errmsg(db);
        sqlite3_finalize(stmt);
        throw std::runtime_error(error);
    }

    rc = sqlite3_step(stmt);
    
    if (rc == SQLITE_ROW) 
    {
        
        NotificationCategory category;
        
        category.id = sqlite3_column_int(stmt, 0);
        
        category.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        
        category.description = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        
        category.channel = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        
        category.defaultPriority = sqlite3_column_int(stmt, 4);
        
        sqlite3_finalize(stmt);
        
        return category;
    }
    
    if (rc != SQLITE_DONE) 
    {
        std::string error = sqlite3_errmsg(db);
        sqlite3_finalize(stmt);
        throw std::runtime_error(error);
    }
    
    sqlite3_finalize(stmt);
    
    return std::nullopt;
}

void runSqliteApiDemo()
{
    sqlite3* db = nullptr;
    if (sqlite3_open("notification.db", &db) != SQLITE_OK) 
    {
        std::string error = db ? sqlite3_errmsg(db) : "Не вдалося відкрити БД";
        if (db) sqlite3_close(db);
        throw std::runtime_error(error);
    }

    try {
        const char* createSql = R"(
            CREATE TABLE IF NOT EXISTS NotificationCategory (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                name TEXT NOT NULL,
                description TEXT NOT NULL,
                channel TEXT NOT NULL,
                defaultPriority INTEGER NOT NULL
            );
        )";
        
        char* errorMessage = nullptr;
        
        const int rc = sqlite3_exec(db, createSql, nullptr, nullptr, &errorMessage);
        
        if (rc != SQLITE_OK) 
        {
            std::string error = errorMessage ? errorMessage : sqlite3_errmsg(db);
            sqlite3_free(errorMessage);
            throw std::runtime_error(error);
        }

        NotificationCategory category{
            0, "Безпека", "Сповіщення про безпеку облікового запису", "email", 1
        };
        
        int id = insertNotificationCategory(db, category);
        auto found = findNotificationCategoryById(db, id);

        std::cout << "\n=== SQLite C API ===\n";
        if (found) 
        {
            std::cout << "Знайдено категорію: " << found->name
                      << " | канал: " << found->channel
                      << " | пріоритет: " << found->defaultPriority << '\n';
        }
    } catch (...) {
        sqlite3_close(db);
        throw;
    }
    sqlite3_close(db);
}

void runOrmDemo()
{
    auto storage = createStorage("notification.db");
    
    storage.on_open = [](sqlite3* db) {
        if (sqlite3_exec(db, "PRAGMA foreign_keys = ON;", nullptr, nullptr, nullptr) != SQLITE_OK)
            throw std::runtime_error("Не вдалося увімкнути зовнішні ключі");
    };
    
    storage.sync_schema();

    NotificationCategory category{
        0, "Замовлення", "Статуси замовлень", "push", 2
    };
    
    int id = storage.insert(category);

    std::cout << "\nORM\n";
    std::cout << "Inserted notification category ID: "
        << id << '\n';

    auto result = storage.get_optional<NotificationCategory>(id);

    if (result)
    {
        std::cout << "Категорію сповіщень знайдено:\n";
        std::cout << "ID: " << result->id << '\n';
        std::cout << "Name: " << result->name << '\n';
        std::cout << "Description: " << result->description << '\n';
        std::cout << "Channel: " << result->channel << '\n';
        std::cout << "Priority: " << result->defaultPriority << '\n';
    }
    else
    {
        std::cout << "Категорію сповіщення не знайдено\n";
    }
}

void runRelationshipDemo()
{
    auto storage = createStorage("notification.db");
    
    storage.on_open = [](sqlite3* db) {
        if (sqlite3_exec(db, "PRAGMA foreign_keys = ON;", nullptr, nullptr, nullptr) != SQLITE_OK)
            throw std::runtime_error("Не вдалося увімкнути зовнішні ключі");
    };
    
    storage.sync_schema();

    NotificationCategory notification_category{
    0, 
    "Безпека", 
    "Попередження про безпеку облікового запису", 
    "email", 
    1
    };

    int categoryId = storage.insert(notification_category);

    std::cout << "\nДемонстрація зв'язків\n";
    std::cout << "Notification category ID: " << categoryId << '\n';

    Notification m1{
    0, 
    categoryId, 
    "Новий вхід", 
    "Виявлено вхід із нового пристрою", 
    "2026-11-01 08:15", 
    1
    };

    Notification m2{
    0, 
    categoryId, 
    "Зміна пароля", 
    "Пароль облікового запису змінено", 
    "2026-11-02 10:20", 
    1
    };

    int notificationId1 = storage.insert(m1);
    int notificationId2 = storage.insert(m2);
    std::cout << "Notifications inserted: "
        << notificationId1 << ", "
        << notificationId2 << '\n';

    using namespace sqlite_orm;

    auto notificationMessages = storage.get_all<Notification>(
        where(
            c(&Notification::categoryId) == categoryId
        )
    );

    std::cout << "\nІсторія сповіщень:\n";

    for (const auto& record : notificationMessages)
    {
        std::cout
            << "ID: " << record.id
            << ", Title: " << record.title
            << ", Message: " << record.message
            << ", Created at: " << record.createdAt
            << ", Priority: " << record.priority
            << '\n';
    }

    try
    {
        Notification invalidRecord{
        0,
        -1,
        "Підозріла активність",
        "Виявлено вхід із нового пристрою",
        "2026-11-02 10:20",
        1
        };

        storage.insert(invalidRecord);

        std::cout << "Неочікуваний успіх вставки\n";
    }
    catch (const std::exception& ex)
    {
        std::cout << "\nТест обмеження зовнішнього ключа:\n";
        std::cout << "Вставка відхилена: "
            << ex.what() << '\n';
    }
}

void runRepositoryDemo()
{
    SQLiteNotificationCategoryRepository sqliteRepository("notification.db");
    
    INotificationCategoryRepository& repository = sqliteRepository;

    std::cout << "\nДемонстрація Repository\n";

    //1. Створення
    NotificationCategory category{
        0,
        "Платежі",
        "Повідомлення про платежі",
        "sms",
        1
    };
    
    int id = repository.add(category);

    std::cout << "\nСтворення\n";
    std::cout << "Вставлений ідентифікатор категорії сповіщень: " << id << '\n';

    // 2. Читання
    std::cout << "\nЧитання\n";

    auto found = repository.findById(id);

    if (found)
    {
        std::cout << "ID: " << found->id << '\n';
        std::cout << "Name: " << found->name << '\n';
        std::cout << "Description: " << found->description << '\n';
        std::cout << "Channel: " << found->channel << '\n';
        std::cout << "Priority: " << found->defaultPriority << '\n';
    }

    // 3. Оновлення
    std::cout << "\nОновлення\n";

    if (found)
    {
        found->description = "CHANGECHANGECHANGECHANGECHANGECHANGECHANGECHANGECHANGECHANGECHANGECHANGECHANGE";

        bool updated = repository.update(*found);

        std::cout << "Оновлено: "
            << std::boolalpha << updated << '\n';

        auto afterUpdate = repository.findById(id);

        if (afterUpdate)
        {
            std::cout << "Новий опис: "
                << afterUpdate->description << '\n';
        }
    }

    // 4. Читання всього
    std::cout << "\nЧитання всього\n";

    auto allNotificationCategory = repository.findAll();

    for (const auto& item : allNotificationCategory)
    {
        std::cout
            << item.id << " | "
            << item.name << " | "
            << item.description << " | "
            << item.channel << " | "
            << item.defaultPriority << '\n';

    }

    // 5. Видалення
    std::cout << "\nВидалення\n";

    bool removed = repository.remove(id);

    std::cout << "Видалено: "
        << std::boolalpha << removed << '\n';

    // 6. Перевірка після видалення
    auto afterDelete = repository.findById(id);

    std::cout << "Запис існує після видалення: "
        << std::boolalpha
        << afterDelete.has_value()
        << '\n';
}

void runQueryDemo()
{
    using namespace sqlite_orm;
    
    auto storage = createStorage("notification_queries.db");
    
    storage.on_open = [](sqlite3* db) {
        if (sqlite3_exec(db, "PRAGMA foreign_keys = ON;", nullptr, nullptr, nullptr) != SQLITE_OK)
            throw std::runtime_error("Не вдалося увімкнути зовнішні ключі");
    };
    
    storage.sync_schema();

    std::cout << "\n=== ORM ЗАПИТИ ===\n";

    auto urgent = storage.get_all<Notification>(
        where(c(&Notification::priority) <= 2)
    );
    std::cout << "1. Сповіщення з пріоритетом 1–2: " << urgent.size() << '\n';

    auto sorted = storage.get_all<Notification>(
        order_by(&Notification::createdAt).desc(), limit(5)
    );
    std::cout << "2. П'ять найновіших сповіщень:\n";
    for (const auto& item : sorted)
        std::cout << item.createdAt << " | " << item.title << '\n';

    const auto totalPriority = storage.sum(&Notification::priority);
    std::cout << "3. Сума пріоритетів усіх сповіщень: "
              << (totalPriority ? std::to_string(*totalPriority) : "0") << '\\n';

    auto joined = storage.select(
        columns(&NotificationCategory::name, &Notification::title,
                &Notification::priority),
        inner_join<Notification>(
            on(c(&NotificationCategory::id) == &Notification::categoryId)
        ),
        order_by(&Notification::createdAt).desc()
    );
    std::cout << "4. JOIN (категорія | заголовок | пріоритет):\n";
    for (const auto& [categoryName, title, priority] : joined)
        std::cout << categoryName << " | " << title << " | "
                  << priority << " | " << '\n';
}

void runTransactionDemo()
{
    auto storage = createStorage("notification_queries.db");
    storage.on_open = [](sqlite3* db) {
        if (sqlite3_exec(
            db,
            "PRAGMA foreign_keys = ON;",
            nullptr,
            nullptr,
            nullptr)
            != SQLITE_OK)
        {
            throw std::runtime_error("Не вдалося увімкнути зовнішні ключі");
        }
    };
    storage.sync_schema();

    const auto categoriesBefore = storage.count<NotificationCategory>();
    const auto notificationsBefore = storage.count<Notification>();

    std::cout << "\nПеред транзакцією:\n";
    std::cout << "Notification category: " << categoriesBefore << '\n';
    std::cout << "Notification: " << notificationsBefore << '\n';

    bool committed = storage.transaction([&]() 
        {
        NotificationCategory category
        {
            0, 
            "Безпека транзакції", 
            "Категорія, створена в транзакції", 
            "email", 
            1
        };

        int categoryId = storage.insert(category);

        Notification notification
        {
            0, 
            categoryId, 
            "Перевірка транзакції",
            "Категорію та сповіщення додано атомарно",
            "2026-11-11 14:10", 
            1
        };
        
        storage.insert(notification);

        std::cout << "\nЗаписи, вставлені всередині транзакції\n";
        std::cout << "Notification category ID: " << categoryId << '\n';

        return true;
    });

    std::cout << "\nТранзакцію здійснено: "
        << std::boolalpha << committed << '\n';

    std::cout << "\nПісля транзакції:\n";
    std::cout << "Notification category: "
        << storage.count<NotificationCategory>() << '\n';
    std::cout << "Notification: "
        << storage.count<Notification>() << '\n';

}

void runRollbackDemo()
{
    auto storage = createStorage("notification_queries.db");

    storage.on_open = [](sqlite3* db)
        {
            if (sqlite3_exec(
                db,
                "PRAGMA foreign_keys = ON;",
                nullptr,
                nullptr,
                nullptr
            ) != SQLITE_OK)
            {
                throw std::runtime_error(
                    "Не вдалося увімкнути зовнішні ключі"
                );
            }
        };

    storage.sync_schema();

    std::cout << "\n=== Демонстрація відкату ===\n";

    auto notification_categoryBefore = storage.count<NotificationCategory>();
    auto notificationBefore = storage.count<Notification>();

    std::cout << "\nПеред транзакцією:\n";
    std::cout << "Notification category: " << notification_categoryBefore << '\n';
    std::cout << "Notification: " << notificationBefore << '\n';

    try
    {
        storage.transaction([&]()
            {
                // Перша операція – коректна
                NotificationCategory category
                {
                    0,
                    "Безпека транзакції",
                    "Категорія, створена в транзакції",
                    "email",
                    1
                };

                int categoryId = storage.insert(category);

                std::cout << "\nКатегорія сповіщень додана, ID: "
                    << categoryId << '\n';

                // Друга операція – навмисно некоректна
                Notification notification
                {
                    0,
                    -1,
                    "Перевірка транзакції",
                    "Категорію та сповіщення додано атомарно",
                    "2026-11-11 14:10",
                    1
                };

                storage.insert(notification);

                return true;
            });
    }
    catch (const std::exception& ex)
    {
        std::cout << "\nТранзакцію не вдалося: "
            << ex.what() << '\n';

        std::cout << "Зміни скасовано\n";
    }

    std::cout << "\nПісля транзакції:\n";
    std::cout << "Notification category: "
        << storage.count<NotificationCategory>() << '\n';
    std::cout << "Notification: "
        << storage.count<Notification>() << '\n';

    // Перевірка відсутності нової категорії
    using namespace sqlite_orm;

    auto transaction_check = storage.get_all<NotificationCategory>(
        where(c(&NotificationCategory::name) == "Сповіщення камери")
    );

    std::cout << "\nСповіщення камери існує: "
        << std::boolalpha
        << !transaction_check.empty() << '\n';
}

void runWithoutTransactionDemo()
{
    const std::string databaseFile =
        "notification_no_transaction.db";

    std::filesystem::remove(databaseFile);

    auto storage = createStorage(databaseFile);

    storage.on_open = [](sqlite3* db)
        {
            if (sqlite3_exec(
                db,
                "PRAGMA foreign_keys = ON;",
                nullptr,
                nullptr,
                nullptr
            ) != SQLITE_OK)
            {
                throw std::runtime_error(
                    "Не вдалося увімкнути зовнішні ключі"
                );
            }
        };

    storage.sync_schema();

    std::cout << "\n=== Без транзакції ===\n";

    try
    {
        NotificationCategory category
        {
            0,
            "Безпека транзакції",
            "Категорія, створена в транзакції",
            "email",
            1
        };

        int categoryId = storage.insert(category);

        std::cout << "\nКатегорію сповіщень додано, ID: "
            << categoryId << '\n';

        // Навмисно некоректний зовнішній ключ
        Notification notification
        {
            0,
            -1,
            "Перевірка транзакції",
            "Категорію та сповіщення додано атомарно",
            "2026-11-11 14:10",
            1
        };

        storage.insert(notification);
    }
    catch (const std::exception& ex)
    {
        std::cout << "\nОперація не вдалася: "
            << ex.what() << '\n';
    }

    std::cout << "\nПісля операції:\n";
    std::cout << "Notification category: "
        << storage.count<NotificationCategory>() << '\n';
    std::cout << "Notification: "
        << storage.count<Notification>() << '\n';
}

int main()
{
    try {
        runSqliteApiDemo();
        runOrmDemo();
        runRelationshipDemo();
        runRepositoryDemo();
        prepareTestData();
        runQueryDemo();
        runTransactionDemo();
		runRollbackDemo();
		runWithoutTransactionDemo();

    } catch (const std::exception& ex) {
        std::cerr << "Помилка застосунку: " << ex.what() << '\n';
        return 1;
    }
    return 0;
}
