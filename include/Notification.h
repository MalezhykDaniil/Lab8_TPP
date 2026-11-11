#pragma once
#include <string>

struct Notification
{
    int id = 0;
    int categoryId = 0;
    std::string title;
    std::string message;
    std::string createdAt;
    int priority = 3;
};
