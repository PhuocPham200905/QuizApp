#pragma once

#include "../Common.h"

class FirebaseService {
private:
    string projectId = "quizapp-e586a";
    string baseUrl = "https://firestore.googleapis.com/v1/projects/quizapp-e586a/databases/(default)/documents";
    string lastError;

public:
    string getLastError() const {
        return lastError;
    }

    bool getCollection(string collectionName, json& output) {
        string response;
        string url = baseUrl + "/" + collectionName;
        if (!request("GET", url, "", response)) {
            return false;
        }

        try {
            output = json::parse(response);
            return true;
        } catch (const exception& error) {
            lastError = string("Không đọc được dữ liệu JSON: ") + error.what();
            return false;
        }
    }

    bool getDocument(string collectionName, string documentId, json& output) {
        string response;
        string url = baseUrl + "/" + collectionName + "/" + documentId;
        if (!request("GET", url, "", response)) {
            return false;
        }
        try {
            output = json::parse(response);
            return true;
        } catch (const exception& error) {
            lastError = string("Không đọc được document JSON: ") + error.what();
            return false;
        }
    }

    bool setDocument(string collectionName, string documentId, const json& fields) {
        string response;
        string url = baseUrl + "/" + collectionName + "/" + documentId;
        json body;
        body["fields"] = fields;
        return request("PATCH", url, body.dump(), response);
    }

    bool createDocument(string collectionName, string documentId, const json& fields) {
        string response;
        string url = baseUrl + "/" + collectionName + "?documentId=" + documentId;
        json body;
        body["fields"] = fields;
        return request("POST", url, body.dump(), response);
    }

    bool deleteDocument(string collectionName, string documentId) {
        string response;
        string url = baseUrl + "/" + collectionName + "/" + documentId;
        return request("DELETE", url, "", response);
    }

    bool getRawUrl(string url, string& output) {
        return request("GET", url, "", output);
    }

    string documentIdFromName(string name) const {
        size_t pos = name.find_last_of('/');
        if (pos == string::npos) {
            return name;
        }
        return name.substr(pos + 1);
    }

    string stringField(const json& fields, string key, string defaultValue = "") const {
        if (!fields.contains(key)) {
            return defaultValue;
        }
        const json& value = fields.at(key);
        if (value.contains("stringValue")) {
            return value.at("stringValue").get<string>();
        }
        if (value.contains("integerValue")) {
            return value.at("integerValue").get<string>();
        }
        if (value.contains("booleanValue")) {
            return value.at("booleanValue").get<bool>() ? "true" : "false";
        }
        return defaultValue;
    }

    int intField(const json& fields, string key, int defaultValue = 0) const {
        if (!fields.contains(key)) {
            return defaultValue;
        }
        const json& value = fields.at(key);
        if (value.contains("integerValue")) {
            return stoi(value.at("integerValue").get<string>());
        }
        if (value.contains("doubleValue")) {
            return (int)value.at("doubleValue").get<double>();
        }
        return defaultValue;
    }

    bool boolField(const json& fields, string key, bool defaultValue = false) const {
        if (!fields.contains(key)) {
            return defaultValue;
        }
        const json& value = fields.at(key);
        if (value.contains("booleanValue")) {
            return value.at("booleanValue").get<bool>();
        }
        return defaultValue;
    }

    vector<string> stringArrayField(const json& fields, string key) const {
        vector<string> values;
        if (!fields.contains(key)) {
            return values;
        }

        const json& field = fields.at(key);
        if (!field.contains("arrayValue")) {
            return values;
        }

        const json& arrayValue = field.at("arrayValue");
        if (!arrayValue.contains("values")) {
            return values;
        }

        for (const json& item : arrayValue.at("values")) {
            if (item.contains("stringValue")) {
                values.push_back(item.at("stringValue").get<string>());
            }
        }

        return values;
    }

    json stringValue(string value) const {
        return json{{"stringValue", value}};
    }

    json intValue(int value) const {
        return json{{"integerValue", to_string(value)}};
    }

    json boolValue(bool value) const {
        return json{{"booleanValue", value}};
    }

    json stringArrayValue(vector<string> values) const {
        json arrayValues = json::array();
        for (const string& value : values) {
            arrayValues.push_back(stringValue(value));
        }
        return json{{"arrayValue", {{"values", arrayValues}}}};
    }

    json answerMapValue(map<string, char> answers) const {
        json answerFields;
        for (const auto& item : answers) {
            answerFields[item.first] = stringValue(string(1, item.second));
        }
        return json{{"mapValue", {{"fields", answerFields}}}};
    }

private:
    static size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp) {
        size_t totalSize = size * nmemb;
        string* response = static_cast<string*>(userp);
        response->append(static_cast<char*>(contents), totalSize);
        return totalSize;
    }

    bool request(string method, string url, string body, string& response) {
        CURL* curl = curl_easy_init();
        if (curl == nullptr) {
            lastError = "Không khởi tạo được kết nối mạng.";
            return false;
        }

        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, 8000L);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 20000L);
        curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "QuizApp/1.0");
        curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);

        if (method == "PATCH") {
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PATCH");
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        } else if (method == "POST") {
            curl_easy_setopt(curl, CURLOPT_POST, 1L);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        } else if (method == "DELETE") {
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
        }

        CURLcode result = curl_easy_perform(curl);
        long statusCode = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &statusCode);

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);

        if (result != CURLE_OK) {
            lastError = string("Lỗi kết nối Firestore: ") + curl_easy_strerror(result);
            return false;
        }

        if (statusCode < 200 || statusCode >= 300) {
            lastError = "Firestore tra ve HTTP " + to_string(statusCode) + ": " + response;
            return false;
        }

        return true;
    }
};
