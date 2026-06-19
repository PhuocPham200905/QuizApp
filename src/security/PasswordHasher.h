#pragma once

#include "../Common.h"

class PasswordHasher {
public:
    static string hashPassword(const string& password);
    static bool verifyPassword(const string& password, const string& storedHash);
    static bool isSha256Hash(const string& value);
    static string normalizePassword(const string& value);
};
