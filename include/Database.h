#pragma once
#include <string>
#include <sqlite_orm/sqlite_orm.h>
#include "NotificationCategory.h"
#include "Notification.h"

inline auto createStorage(const std::string& filename)
{
    using namespace sqlite_orm;

    return make_storage(
        filename,
        
        make_table(
            "NotificationCategory",
            
            make_column("id", &NotificationCategory::id, primary_key().autoincrement()),
            
            make_column("name", &NotificationCategory::name, not_null()),
            
            make_column("description", &NotificationCategory::description, not_null()),
            
            make_column("channel", &NotificationCategory::channel, not_null()),
            
            make_column("defaultPriority", &NotificationCategory::defaultPriority, not_null())
        ),
        
        make_table(
            "Notification",
            
            make_column("id", &Notification::id, primary_key().autoincrement()),
            
            make_column("categoryId", &Notification::categoryId, not_null()),
            
            make_column("title", &Notification::title, not_null()),
            
            make_column("message", &Notification::message, not_null()),
            
            make_column("createdAt", &Notification::createdAt, not_null()),
            
            make_column("priority", &Notification::priority, not_null()),

            foreign_key(&Notification::categoryId)
                .references(&NotificationCategory::id)
        )
    );
}
