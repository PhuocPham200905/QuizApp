#include "FirebaseService.h"

string FirebaseService::getLastError() const {
    return lastError;
}

bool FirebaseService::getCollection(string collectionName, json& output) {
    string response;
    string url = baseUrl + "/" + collectionName;
    if (!request("GET", url, "", response)) {
        return false;
    }

    try {
        output = json::parse(response);
        return true;
    } catch (const exception& error) {
        lastError = string("Khong doc duoc du lieu JSON: ") + error.what();
        return false;
    }
}

bool FirebaseService::getDocument(string collectionName, string documentId, json& output) {
    string response;
    string url = baseUrl + "/" + collectionName + "/" + documentId;
    if (!request("GET", url, "", response)) {
        return false;
    }

    try {
        output = json::parse(response);
        return true;
    } catch (const exception& error) {
        lastError = string("Khong doc duoc document JSON: ") + error.what();
        return false;
    }
}

bool FirebaseService::setDocument(string collectionName, string documentId, const json& fields) {
    string response;
    string url = baseUrl + "/" + collectionName + "/" + documentId;
    json body;
    body["fields"] = fields;
    return request("PATCH", url, body.dump(), response);
}

bool FirebaseService::createDocument(string collectionName, string documentId, const json& fields) {
    string response;
    string url = baseUrl + "/" + collectionName + "?documentId=" + documentId;
    json body;
    body["fields"] = fields;
    return request("POST", url, body.dump(), response);
}

bool FirebaseService::deleteDocument(string collectionName, string documentId) {
    string response;
    string url = baseUrl + "/" + collectionName + "/" + documentId;
    return request("DELETE", url, "", response);
}

bool FirebaseService::getRawUrl(string url, string& output) {
    return request("GET", url, "", output);
}

string FirebaseService::documentIdFromName(string name) const {
    size_t position = name.find_last_of('/');
    if (position == string::npos) {
        return name;
    }
    return name.substr(position + 1);
}

string FirebaseService::stringField(const json& fields, string key, string defaultValue) const {
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

int FirebaseService::intField(const json& fields, string key, int defaultValue) const {
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

bool FirebaseService::boolField(const json& fields, string key, bool defaultValue) const {
    if (!fields.contains(key)) {
        return defaultValue;
    }

    const json& value = fields.at(key);
    if (value.contains("booleanValue")) {
        return value.at("booleanValue").get<bool>();
    }
    return defaultValue;
}

vector<string> FirebaseService::stringArrayField(const json& fields, string key) const {
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

json FirebaseService::stringValue(string value) const {
    return json{{"stringValue", value}};
}

json FirebaseService::intValue(int value) const {
    return json{{"integerValue", to_string(value)}};
}

json FirebaseService::boolValue(bool value) const {
    return json{{"booleanValue", value}};
}

json FirebaseService::stringArrayValue(vector<string> values) const {
    json arrayValues = json::array();
    for (const string& value : values) {
        arrayValues.push_back(stringValue(value));
    }
    return json{{"arrayValue", {{"values", arrayValues}}}};
}

json FirebaseService::answerMapValue(map<string, char> answers) const {
    json answerFields;
    for (const auto& item : answers) {
        answerFields[item.first] = stringValue(string(1, item.second));
    }
    return json{{"mapValue", {{"fields", answerFields}}}};
}

size_t FirebaseService::writeCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t totalSize = size * nmemb;
    string* response = static_cast<string*>(userp);
    response->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

bool FirebaseService::request(string method, string url, string body, string& response) {
    CURL* curl = curl_easy_init();
    if (curl == nullptr) {
        lastError = "Khong khoi tao duoc ket noi mang.";
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
        lastError = string("Loi ket noi Firestore: ") + curl_easy_strerror(result);
        return false;
    }

    if (statusCode < 200 || statusCode >= 300) {
        lastError = "Firestore tra ve HTTP " + to_string(statusCode) + ": " + response;
        return false;
    }

    return true;
}
