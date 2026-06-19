#include "PasswordHasher.h"

#include <bcrypt.h>

#pragma comment(lib, "bcrypt.lib")

namespace {
    string bytesToHex(const vector<unsigned char>& bytes) {
        stringstream stream;
        stream << hex << setfill('0');
        for (unsigned char byte : bytes) {
            stream << setw(2) << static_cast<int>(byte);
        }
        return stream.str();
    }
}

string PasswordHasher::hashPassword(const string& password) {
    BCRYPT_ALG_HANDLE algorithmHandle = nullptr;
    BCRYPT_HASH_HANDLE hashHandle = nullptr;
    DWORD objectLength = 0;
    DWORD propertyLength = 0;
    DWORD hashLength = 0;
    vector<unsigned char> hashObject;
    vector<unsigned char> hashBytes;

    if (BCryptOpenAlgorithmProvider(&algorithmHandle, BCRYPT_SHA256_ALGORITHM, nullptr, 0) != 0) {
        return "";
    }

    if (BCryptGetProperty(algorithmHandle, BCRYPT_OBJECT_LENGTH,
                          reinterpret_cast<PUCHAR>(&objectLength), sizeof(objectLength),
                          &propertyLength, 0) != 0) {
        BCryptCloseAlgorithmProvider(algorithmHandle, 0);
        return "";
    }

    if (BCryptGetProperty(algorithmHandle, BCRYPT_HASH_LENGTH,
                          reinterpret_cast<PUCHAR>(&hashLength), sizeof(hashLength),
                          &propertyLength, 0) != 0) {
        BCryptCloseAlgorithmProvider(algorithmHandle, 0);
        return "";
    }

    hashObject.resize(objectLength);
    hashBytes.resize(hashLength);

    if (BCryptCreateHash(algorithmHandle, &hashHandle, hashObject.data(),
                         static_cast<ULONG>(hashObject.size()), nullptr, 0, 0) != 0) {
        BCryptCloseAlgorithmProvider(algorithmHandle, 0);
        return "";
    }

    if (BCryptHashData(hashHandle,
                       reinterpret_cast<PUCHAR>(const_cast<char*>(password.data())),
                       static_cast<ULONG>(password.size()), 0) != 0) {
        BCryptDestroyHash(hashHandle);
        BCryptCloseAlgorithmProvider(algorithmHandle, 0);
        return "";
    }

    if (BCryptFinishHash(hashHandle, hashBytes.data(), static_cast<ULONG>(hashBytes.size()), 0) != 0) {
        BCryptDestroyHash(hashHandle);
        BCryptCloseAlgorithmProvider(algorithmHandle, 0);
        return "";
    }

    BCryptDestroyHash(hashHandle);
    BCryptCloseAlgorithmProvider(algorithmHandle, 0);
    return bytesToHex(hashBytes);
}

bool PasswordHasher::verifyPassword(const string& password, const string& storedHash) {
    if (!isSha256Hash(storedHash)) {
        return storedHash == password;
    }

    return hashPassword(password) == storedHash;
}

bool PasswordHasher::isSha256Hash(const string& value) {
    if (value.size() != 64) {
        return false;
    }

    return all_of(value.begin(), value.end(), [](unsigned char character) {
        return isxdigit(character) != 0;
    });
}

string PasswordHasher::normalizePassword(const string& value) {
    if (value.empty()) {
        return "";
    }

    if (isSha256Hash(value)) {
        string normalized = value;
        transform(normalized.begin(), normalized.end(), normalized.begin(),
                  [](unsigned char character) { return static_cast<char>(tolower(character)); });
        return normalized;
    }

    return hashPassword(value);
}
