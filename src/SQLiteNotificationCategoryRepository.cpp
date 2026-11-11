#include "SQLiteNotificationCategoryRepository.h"
#include <stdexcept>
#include <sqlite3.h>

SQLiteNotificationCategoryRepository::SQLiteNotificationCategoryRepository(
    const std::string& databaseFile
)
    : storage_(createStorage(databaseFile))
{
    storage_.on_open = [](sqlite3* db) {
        int rc = sqlite3_exec(db, 
            "PRAGMA foreign_keys = ON;",                     
            nullptr, 
            nullptr, 
            nullptr
        );
        
        if (rc != SQLITE_OK) {
            throw std::runtime_error(
                "Cannot enable foreign key constraints"
            );
        }
    };
    storage_.sync_schema();
}

int SQLiteNotificationCategoryRepository::add(const NotificationCategory& category)
{
    return storage_.insert(category);
}

std::optional<NotificationCategory>
SQLiteNotificationCategoryRepository::findById(int id)
{
    return storage_.get_optional<NotificationCategory>(id);
}

std::vector<NotificationCategory>
SQLiteNotificationCategoryRepository::findAll()
{
    return storage_.get_all<NotificationCategory>();
}

bool SQLiteNotificationCategoryRepository::update(const NotificationCategory& category)
{
    auto existing =
        storage_.get_optional<NotificationCategory>(category.id);

    if (!existing)
    {
        return false;
    }
    storage_.update(category);

    return true;
}

bool SQLiteNotificationCategoryRepository::remove(int id)
{
    auto existing =
        storage_.get_optional<NotificationCategory>(id);

    if (!existing)
    {
        return false;
    }

    storage_.remove<NotificationCategory>(id);
    return true;
}
