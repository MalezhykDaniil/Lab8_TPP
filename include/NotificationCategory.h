#pragma once
#include <string>

struct NotificationCategory
{
    int id = 0;
    std::string name;
    std::string description;
    std::string channel;
    int defaultPriority = 3;
};
