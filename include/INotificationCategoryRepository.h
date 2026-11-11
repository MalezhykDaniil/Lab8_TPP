#pragma once
#include <optional>
#include <vector>
#include "NotificationCategory.h"

class INotificationCategoryRepository
{
public:
    virtual ~INotificationCategoryRepository() = default;
    
    //Створення
    virtual int add(const NotificationCategory& category) = 0;
    
    //Читання
    virtual std::optional<NotificationCategory> findById(int id) = 0;
    virtual std::vector<NotificationCategory> findAll() = 0;
    
    //Оновлення
    virtual bool update(const NotificationCategory& category) = 0;
    
    //Видалення
    virtual bool remove(int id) = 0;
};
