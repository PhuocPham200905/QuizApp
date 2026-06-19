#pragma once

#include "../Common.h"
#include "../services/FirebaseService.h"

class QuizRepository {
private:
    FirebaseService firebase;

public:
    string getLastError() const;

    bool getCollection(string collectionName, json& output);
    bool getDocument(string collectionName, string documentId, json& output);
    bool setDocument(string collectionName, string documentId, const json& fields);
    bool createDocument(string collectionName, string documentId, const json& fields);
    bool deleteDocument(string collectionName, string documentId);
    bool getRawUrl(string url, string& output);

    string documentIdFromName(string name) const;
    string stringField(const json& fields, string key, string defaultValue = "") const;
    int intField(const json& fields, string key, int defaultValue = 0) const;
    bool boolField(const json& fields, string key, bool defaultValue = false) const;
    vector<string> stringArrayField(const json& fields, string key) const;

    json stringValue(string value) const;
    json intValue(int value) const;
    json boolValue(bool value) const;
    json stringArrayValue(vector<string> values) const;
    json answerMapValue(map<string, char> answers) const;
};
