#pragma once
#include <string>
#include "INotificationCategoryRepository.h"
#include "Database.h"

class SQLiteNotificationCategoryRepository 
    : public INotificationCategoryRepository
{
public:
    explicit SQLiteNotificationCategoryRepository(const std::string& databaseFile);

    int add(const NotificationCategory& category) override;
    
    std::optional<NotificationCategory> findById(int id) override;
    
    std::vector<NotificationCategory> findAll() override;
    
    bool update(const NotificationCategory& category) override;
    
    bool remove(int id) override;

private:
    using Storage = decltype(createStorage(""));
    Storage storage_;
};
