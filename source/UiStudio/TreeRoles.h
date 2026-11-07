#pragma once

#include <Qt>

// Data stored on every item of the page structure tree.
constexpr int RolePath = Qt::UserRole;
constexpr int RoleRect = Qt::UserRole + 1;
constexpr int RoleFile = Qt::UserRole + 2;
constexpr int RoleLine = Qt::UserRole + 3;
constexpr int RoleCol = Qt::UserRole + 4;
