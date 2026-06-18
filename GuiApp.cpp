#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <commdlg.h>
#include <shobjidl.h>
#include <shellapi.h>
#include <gdiplus.h>
#include <curl/curl.h>
#include <zlib.h>
#include <nlohmann/json.hpp>

#include <algorithm>
#include <array>
#include <cctype>
#include <chrono>
#include <cstdint>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

using namespace std;
using json = nlohmann::json;
using namespace Gdiplus;

class User {
protected:
    string userId;
    string name;
    string email;
    string password;
    string role;
    string className;

public:
    User(string userId, string name, string email, string password, string role, string className)
        : userId(userId), name(name), email(email), password(password), role(role), className(className) {}

    virtual ~User() {}

    string getUserId() const { return userId; }
    string getName() const { return name; }
    string getEmail() const { return email; }
    string getPassword() const { return password; }
    string getRole() const { return role; }
    string getClassName() const { return className; }

    void setName(string value) { name = value; }
    void setEmail(string value) { email = value; }
    void setPassword(string value) { password = value; }
    void setRole(string value) { role = value; }
    void setClassName(string value) { className = value; }

    bool checkPassword(string inputPassword) const {
        return password == inputPassword;
    }

    virtual string getInfoLine() const {
        return userId + " | " + role + " | " + name + " | " + email + " | " + className;
    }
};

class Student : public User {
public:
    Student(string userId, string name, string email, string password, string className)
        : User(userId, name, email, password, "student", className) {}
};

class Admin : public User {
public:
    Admin(string userId, string name, string email, string password)
        : User(userId, name, email, password, "admin", "Admin") {}
};

class Teacher : public User {
public:
    Teacher(string userId, string name, string email, string password, string className)
        : User(userId, name, email, password, "teacher", className) {}
};

class Question {
private:
    string questionId;
    string content;
    array<string, 4> options;
    char correctAnswer;
    string subject;
    string difficulty;
    string imagePath;
    string imageUrl;

public:
    Question(string questionId, string content, array<string, 4> options,
             char correctAnswer, string subject, string difficulty,
             string imagePath = "", string imageUrl = "")
        : questionId(questionId), content(content), options(options),
          correctAnswer((char)toupper(correctAnswer)), subject(subject), difficulty(difficulty),
          imagePath(imagePath), imageUrl(imageUrl) {}

    string getQuestionId() const { return questionId; }
    string getContent() const { return content; }
    array<string, 4> getOptions() const { return options; }
    char getCorrectAnswer() const { return correctAnswer; }
    string getSubject() const { return subject; }
    string getDifficulty() const { return difficulty; }
    string getImagePath() const { return imagePath; }
    string getImageUrl() const { return imageUrl; }
    bool hasImage() const { return !imagePath.empty() || !imageUrl.empty(); }

    void setContent(string value) { content = value; }
    void setOptions(array<string, 4> value) { options = value; }
    void setCorrectAnswer(char value) { correctAnswer = (char)toupper(value); }
    void setSubject(string value) { subject = value; }
    void setDifficulty(string value) { difficulty = value; }
    void setImagePath(string value) { imagePath = value; }
    void setImageUrl(string value) { imageUrl = value; }

    string toText(bool showAnswer) const {
        stringstream ss;
        ss << questionId << ". " << content << " [" << subject << "]\r\n";
        for (int i = 0; i < 4; i++) {
            ss << "   " << (char)('A' + i) << ". " << options[i] << "\r\n";
        }
        if (showAnswer) {
            ss << "   Đáp án đúng: " << correctAnswer << "\r\n";
        }
        if (hasImage()) {
            ss << "   Hình ảnh: Có\r\n";
        }
        ss << "\r\n";
        return ss.str();
    }
};

class Exam {
private:
    string examId;
    string title;
    int durationMinutes;
    vector<string> questionIds;
    string createdBy;
    string startAt;
    string closeAt;
    string examPassword;
    string attachmentPath;
    string attachmentUrl;
    vector<char> answerKey;
    int attemptLimit;
    bool open;

public:
    Exam(string examId, string title, int durationMinutes, vector<string> questionIds,
         string createdBy, string startAt, string closeAt, string examPassword,
         string attachmentPath, string attachmentUrl, vector<char> answerKey, int attemptLimit, bool open)
        : examId(examId), title(title), durationMinutes(durationMinutes),
          questionIds(questionIds), createdBy(createdBy), startAt(startAt), closeAt(closeAt),
          examPassword(examPassword), attachmentPath(attachmentPath), attachmentUrl(attachmentUrl),
          answerKey(answerKey), attemptLimit(attemptLimit), open(open) {}

    string getExamId() const { return examId; }
    string getTitle() const { return title; }
    int getDurationMinutes() const { return durationMinutes; }
    vector<string> getQuestionIds() const { return questionIds; }
    string getCreatedBy() const { return createdBy; }
    string getStartAt() const { return startAt; }
    string getCloseAt() const { return closeAt; }
    string getExamPassword() const { return examPassword; }
    string getAttachmentPath() const { return attachmentPath; }
    string getAttachmentUrl() const { return attachmentUrl; }
    vector<char> getAnswerKey() const { return answerKey; }
    int getAttemptLimit() const { return attemptLimit; }
    bool isOpen() const { return open; }
    bool hasPassword() const { return !examPassword.empty(); }
    bool hasAttachment() const { return !attachmentPath.empty() || !attachmentUrl.empty(); }
    bool isFileAnswerExam() const { return questionIds.empty() && !answerKey.empty(); }
    int totalQuestions() const { return isFileAnswerExam() ? (int)answerKey.size() : (int)questionIds.size(); }

    void setTitle(string value) { title = value; }
    void setDurationMinutes(int value) { durationMinutes = value; }
    void setQuestionIds(vector<string> value) { questionIds = value; }
    void setCreatedBy(string value) { createdBy = value; }
    void setStartAt(string value) { startAt = value; }
    void setCloseAt(string value) { closeAt = value; }
    void setExamPassword(string value) { examPassword = value; }
    void setAttachmentPath(string value) { attachmentPath = value; }
    void setAttachmentUrl(string value) { attachmentUrl = value; }
    void setAnswerKey(vector<char> value) { answerKey = value; }
    void setAttemptLimit(int value) { attemptLimit = value; }
    void setOpen(bool value) { open = value; }

    bool isBeforeStart(string currentTime) const {
        return !startAt.empty() && currentTime < startAt;
    }

    bool isClosedByTime(string currentTime) const {
        return !closeAt.empty() && currentTime > closeAt;
    }

    bool canTake(string currentTime) const {
        return open && !isBeforeStart(currentTime) && !isClosedByTime(currentTime);
    }

    string statusText(string currentTime) const {
        if (!open) {
            return "DONG";
        }
        if (isBeforeStart(currentTime)) {
            return "CHUA_MO";
        }
        if (isClosedByTime(currentTime)) {
            return "HET_HAN";
        }
        return "OPEN";
    }

    bool checkExamPassword(string inputPassword) const {
        return examPassword.empty() || examPassword == inputPassword;
    }
};

class AnswerSheet {
private:
    string sheetId;
    string studentId;
    string examId;
    map<string, char> answers;
    string submittedAt;
    int score;

public:
    AnswerSheet(string sheetId, string studentId, string examId,
                map<string, char> answers, string submittedAt, int score)
        : sheetId(sheetId), studentId(studentId), examId(examId),
          answers(answers), submittedAt(submittedAt), score(score) {}

    string getSheetId() const { return sheetId; }
    string getStudentId() const { return studentId; }
    string getExamId() const { return examId; }
    map<string, char> getAnswers() const { return answers; }
    string getSubmittedAt() const { return submittedAt; }
    int getScore() const { return score; }

    void setAnswers(map<string, char> value) { answers = value; }
    void setSubmittedAt(string value) { submittedAt = value; }
    void setScore(int value) { score = value; }
};

class Result {
private:
    string resultId;
    string studentId;
    string examId;
    int score;
    int totalQuestions;
    string submittedAt;

public:
    Result(string resultId, string studentId, string examId, int score,
           int totalQuestions, string submittedAt)
        : resultId(resultId), studentId(studentId), examId(examId),
          score(score), totalQuestions(totalQuestions), submittedAt(submittedAt) {}

    string getResultId() const { return resultId; }
    string getStudentId() const { return studentId; }
    string getExamId() const { return examId; }
    int getScore() const { return score; }
    int getTotalQuestions() const { return totalQuestions; }
    string getSubmittedAt() const { return submittedAt; }

    void setScore(int value) { score = value; }
    void setTotalQuestions(int value) { totalQuestions = value; }
    void setSubmittedAt(string value) { submittedAt = value; }
};

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

class QuizData {
private:
    vector<shared_ptr<User>> users;
    vector<Question> questions;
    vector<Exam> exams;
    vector<AnswerSheet> answerSheets;
    vector<Result> results;
    FirebaseService firebase;
    bool connectedToFirebase = false;
    string firebaseStatus = "Đang dùng dữ liệu mẫu trong RAM.";

    int nextAdminNumber = 1;
    int nextTeacherNumber = 1;
    int nextStudentNumber = 1;
    int nextQuestionNumber = 1;
    int nextExamNumber = 1;
    int nextSheetNumber = 1;
    int nextResultNumber = 1;

public:
    QuizData() {
        seedSampleData();
        loadFromFirebase();
    }

    void seedSampleData() {
        users.push_back(make_shared<Admin>(createAdminId(), "Admin Demo", "admin@quiz.local", "123456"));
        users.push_back(make_shared<Teacher>(createTeacherId(), "Teacher Demo", "teacher@quiz.local", "123456", "12A1"));
        users.push_back(make_shared<Student>(createStudentId(), "Nguyễn An", "an@quiz.local", "123456", "12A1"));

        questions.push_back(Question(createQuestionId(), "C++ là ngôn ngữ lập trình thuộc nhóm nào?",
                                     {"Hướng đối tượng", "Chỉ HTML", "Hệ điều hành", "Cơ sở dữ liệu"},
                                     'A', "C++", "Easy"));
        questions.push_back(Question(createQuestionId(), "Từ khóa nào dùng để tạo class trong C++?",
                                     {"structs", "class", "object", "newclass"},
                                     'B', "C++", "Easy"));
        questions.push_back(Question(createQuestionId(), "Tính đóng gói trong OOP giúp điều gì?",
                                     {"Ẩn chi tiết bên trong", "Tăng kích thước file", "Xóa biến", "Tắt chương trình"},
                                     'A', "OOP", "Medium"));
        questions.push_back(Question(createQuestionId(), "Vector trong C++ nằm trong thư viện nào?",
                                     {"<map>", "<array>", "<vector>", "<string>"},
                                     'C', "C++", "Easy"));
        questions.push_back(Question(createQuestionId(), "Kế thừa trong OOP được dùng để làm gì?",
                                     {"Tái sử dụng và mở rộng class", "Mã hóa mật khẩu", "Kết nối Wi-Fi", "Vẽ giao diện"},
                                     'A', "OOP", "Medium"));

        exams.push_back(Exam(createExamId(), "Đề mẫu C++ OOP", 30,
                             {"q1", "q2", "q3", "q4", "q5"}, "t1",
                             "2026-01-01 00:00", "2026-12-31 23:59", "oop123", "", "", {}, 1, true));
    }

    vector<shared_ptr<User>>& getUsers() { return users; }
    vector<Question>& getQuestions() { return questions; }
    vector<Exam>& getExams() { return exams; }
    vector<Result>& getResults() { return results; }
    bool isConnectedToFirebase() const { return connectedToFirebase; }
    string getFirebaseStatus() const { return firebaseStatus; }

    shared_ptr<User> findUserByEmail(string email) {
        for (shared_ptr<User> user : users) {
            if (user->getEmail() == email) {
                return user;
            }
        }
        return nullptr;
    }

    shared_ptr<User> findUserById(string userId) {
        for (shared_ptr<User> user : users) {
            if (user->getUserId() == userId) {
                return user;
            }
        }
        return nullptr;
    }

    string getUserNameById(string userId) {
        for (shared_ptr<User> user : users) {
            if (user->getUserId() == userId) {
                return user->getName();
            }
        }
        return userId;
    }

    Question* findQuestionById(string questionId) {
        for (Question& question : questions) {
            if (question.getQuestionId() == questionId) {
                return &question;
            }
        }
        return nullptr;
    }

    Exam* findExamById(string examId) {
        for (Exam& exam : exams) {
            if (exam.getExamId() == examId) {
                return &exam;
            }
        }
        return nullptr;
    }

    string getExamTitle(string examId) {
        for (Exam& exam : exams) {
            if (exam.getExamId() == examId) {
                return exam.getTitle();
            }
        }
        return examId;
    }

    bool addStudent(string name, string email, string password, string className) {
        string userId = createStudentIdFromEmail(email);
        if (!saveUserToFirebase(userId, name, email, password, "student", className, true)) {
            return false;
        }
        users.push_back(make_shared<Student>(userId, name, email, password, className));
        return true;
    }

    bool addTeacher(string name, string email, string password, string className) {
        string userId = createTeacherId();
        if (!saveUserToFirebase(userId, name, email, password, "teacher", className)) {
            return false;
        }
        users.push_back(make_shared<Teacher>(userId, name, email, password, className));
        return true;
    }

    bool updateUserClass(string userId, string expectedRole, string newClassName) {
        shared_ptr<User> user = findUserById(userId);
        if (user == nullptr || user->getRole() != expectedRole) {
            firebaseStatus = "Không tìm thấy tài khoản " + expectedRole + " co ID: " + userId;
            return false;
        }

        if (!saveUserToFirebase(user->getUserId(), user->getName(), user->getEmail(),
                                user->getPassword(), user->getRole(), newClassName)) {
            return false;
        }

        user->setClassName(newClassName);
        return true;
    }

    bool updateUserProfile(string userId, string name, string email, string password) {
        shared_ptr<User> user = findUserById(userId);
        if (user == nullptr) {
            firebaseStatus = "Không tìm thấy tài khoản co ID: " + userId;
            return false;
        }

        if (!saveUserToFirebase(user->getUserId(), name, email, password,
                                user->getRole(), user->getClassName())) {
            return false;
        }

        user->setName(name);
        user->setEmail(email);
        user->setPassword(password);
        return true;
    }

    bool resetUserPassword(string userId, string expectedRole, string newPassword) {
        shared_ptr<User> user = findUserById(userId);
        if (user == nullptr || user->getRole() != expectedRole) {
            firebaseStatus = "Không tìm thấy tài khoản " + expectedRole + " co ID: " + userId;
            return false;
        }

        if (!saveUserToFirebase(user->getUserId(), user->getName(), user->getEmail(),
                                newPassword, user->getRole(), user->getClassName())) {
            return false;
        }

        user->setPassword(newPassword);
        return true;
    }

    bool deleteUser(string userId, string expectedRole) {
        auto iterator = find_if(users.begin(), users.end(),
                                [userId, expectedRole](shared_ptr<User> user) {
                                    return user->getUserId() == userId && user->getRole() == expectedRole;
                                });

        if (iterator == users.end()) {
            firebaseStatus = "Không tìm thấy tài khoản " + expectedRole + " co ID: " + userId;
            return false;
        }

        string collectionName = collectionForRole(expectedRole);
        if (collectionName.empty()) {
            firebaseStatus = "Vai trò không hợp lệ: " + expectedRole;
            return false;
        }

        if (!firebase.deleteDocument(collectionName, userId)) {
            connectedToFirebase = false;
            firebaseStatus = "Không xóa được Firestore.\r\n" + firebase.getLastError();
            return false;
        }

        users.erase(iterator);
        connectedToFirebase = true;
        firebaseStatus = "Dữ liệu đã sẵn sàng.";
        return true;
    }

    void addQuestion(string content, array<string, 4> options, char correctAnswer,
                     string subject, string imageSourcePath = "", string imageUrl = "") {
        string questionId = createQuestionId();
        string imagePath = prepareQuestionImage(questionId, imageSourcePath);
        questions.push_back(Question(questionId, content, options, correctAnswer, subject, "", imagePath, imageUrl));
        saveQuestionToFirebase(questionId, content, options, correctAnswer, subject, imagePath, imageUrl);
    }

    bool downloadTextFromUrl(string url, string& output) {
        if (!firebase.getRawUrl(url, output)) {
            connectedToFirebase = false;
            firebaseStatus = "Không tải được dữ liệu từ URL.\r\n" + firebase.getLastError();
            return false;
        }

        connectedToFirebase = true;
        firebaseStatus = "Đã tải dữ liệu từ URL.";
        return true;
    }

    int importQuestionsFromCsv(string csvText, string subject, string& importMessage) {
        stringstream input(csvText);
        string line;
        int importedCount = 0;
        int skippedCount = 0;
        int lineNumber = 0;

        while (getline(input, line)) {
            lineNumber++;
            trimInPlace(line);
            if (line.empty()) {
                continue;
            }

            vector<string> columns = parseCsvLine(line);
            if (columns.size() < 6) {
                skippedCount++;
                continue;
            }

            if (lineNumber == 1 && lowerText(columns[0]) == "content") {
                continue;
            }

            for (string& column : columns) {
                trimInPlace(column);
            }

            string content = columns[0];
            array<string, 4> options = {columns[1], columns[2], columns[3], columns[4]};
            string answerText = columns[5];

            if (content.empty() || options[0].empty() || options[1].empty() ||
                options[2].empty() || options[3].empty() || answerText.empty() || subject.empty()) {
                skippedCount++;
                continue;
            }

            char answer = (char)toupper((unsigned char)answerText[0]);
            if (answer < 'A' || answer > 'D') {
                skippedCount++;
                continue;
            }

            addQuestion(content, options, answer, subject);
            importedCount++;
        }

        importMessage = "Đã nhập " + to_string(importedCount) + " câu hỏi.";
        if (skippedCount > 0) {
            importMessage += "\r\nBỏ qua " + to_string(skippedCount) + " dòng không hợp lệ.";
        }

        return importedCount;
    }

    string prepareExamAttachment(string examId, string sourcePath) {
        trimInPlace(sourcePath);
        if (sourcePath.empty()) {
            return "";
        }

        string extension = fileExtension(sourcePath);
        string fileName = examId + "_de_thi" + extension;
        string folder = appDirectory() + "\\exam_files";
        CreateDirectoryA(folder.c_str(), nullptr);

        string targetPath = folder + "\\" + fileName;
        if (!CopyFileA(sourcePath.c_str(), targetPath.c_str(), FALSE)) {
            firebaseStatus = "Không sao chép được file đề thi vào thư mục exam_files.";
            return sourcePath;
        }

        return targetPath;
    }

    string prepareQuestionImage(string questionId, string sourcePath) {
        trimInPlace(sourcePath);
        if (sourcePath.empty()) {
            return "";
        }

        string extension = fileExtension(sourcePath);
        string fileName = questionId + "_image" + extension;
        string folder = appDirectory() + "\\question_images";
        CreateDirectoryA(folder.c_str(), nullptr);

        string targetPath = folder + "\\" + fileName;
        if (!CopyFileA(sourcePath.c_str(), targetPath.c_str(), FALSE)) {
            firebaseStatus = "Không copy được hình ảnh câu hỏi vào thư mục question_images.";
            return sourcePath;
        }

        return targetPath;
    }

    string fileExtension(string path) {
        size_t slash = path.find_last_of("\\/");
        size_t dot = path.find_last_of('.');
        if (dot == string::npos || (slash != string::npos && dot < slash)) {
            return "";
        }
        return path.substr(dot);
    }

    vector<string> stringsFromChars(vector<char> values) {
        vector<string> strings;
        for (char value : values) {
            strings.push_back(string(1, value));
        }
        return strings;
    }

    vector<char> charArrayFromStrings(vector<string> values) {
        vector<char> chars;
        for (string value : values) {
            if (!value.empty()) {
                char answer = (char)toupper((unsigned char)value[0]);
                if (answer >= 'A' && answer <= 'D') {
                    chars.push_back(answer);
                }
            }
        }
        return chars;
    }

    string appDirectory() {
        char path[MAX_PATH] = "";
        GetModuleFileNameA(nullptr, path, MAX_PATH);
        string fullPath = path;
        size_t slash = fullPath.find_last_of("\\/");
        if (slash == string::npos) {
            return ".";
        }
        return fullPath.substr(0, slash);
    }

    bool deleteQuestion(string questionId) {
        int oldSize = (int)questions.size();
        questions.erase(remove_if(questions.begin(), questions.end(),
                                  [questionId](Question question) {
                                      return question.getQuestionId() == questionId;
                                  }),
                        questions.end());
        return (int)questions.size() < oldSize;
    }

    void addExam(string title, int duration, vector<string> questionIds, vector<char> answerKey,
                 string createdBy, string startAt, string closeAt, string examPassword,
                 string attachmentSourcePath, string attachmentUrl, int attemptLimit) {
        string examId = createExamId();
        string attachmentPath = prepareExamAttachment(examId, attachmentSourcePath);
        exams.push_back(Exam(examId, title, duration, questionIds,
                             createdBy, startAt, closeAt, examPassword,
                             attachmentPath, attachmentUrl, answerKey, attemptLimit, true));
        saveExamToFirebase(examId, title, duration, questionIds, createdBy, startAt, closeAt,
                           examPassword, attachmentPath, attachmentUrl, answerKey, attemptLimit, true);
    }

    bool updateExamAttemptLimit(string examId, int attemptLimit) {
        Exam* exam = findExamById(examId);
        if (exam == nullptr) {
            firebaseStatus = "Không tìm thấy đề thi.";
            return false;
        }
        if (attemptLimit <= 0) {
            firebaseStatus = "Số lượt làm phải lớn hơn 0.";
            return false;
        }

        exam->setAttemptLimit(attemptLimit);
        saveExamToFirebase(exam->getExamId(), exam->getTitle(), exam->getDurationMinutes(),
                           exam->getQuestionIds(), exam->getCreatedBy(), exam->getStartAt(),
                           exam->getCloseAt(), exam->getExamPassword(), exam->getAttachmentPath(),
                           exam->getAttachmentUrl(), exam->getAnswerKey(), exam->getAttemptLimit(),
                           exam->isOpen());
        return true;
    }

    int calculateScore(Exam& exam, map<string, char> answers) {
        int score = 0;
        if (exam.isFileAnswerExam()) {
            vector<char> answerKey = exam.getAnswerKey();
            for (int i = 0; i < (int)answerKey.size(); i++) {
                string answerId = "c" + to_string(i + 1);
                if (answers[answerId] == answerKey[i]) {
                    score++;
                }
            }
            return score;
        }

        for (string questionId : exam.getQuestionIds()) {
            Question* question = findQuestionById(questionId);
            if (question != nullptr && answers[questionId] == question->getCorrectAnswer()) {
                score++;
            }
        }
        return score;
    }

    void submitExam(shared_ptr<User> student, Exam& exam, map<string, char> answers) {
        int score = calculateScore(exam, answers);
        string submittedAt = currentTimeText();
        string sheetId = createSheetId();
        string resultId = createResultId();
        answerSheets.push_back(AnswerSheet(sheetId, student->getUserId(), exam.getExamId(),
                                           answers, submittedAt, score));
        results.push_back(Result(resultId, student->getUserId(), exam.getExamId(),
                                 score, exam.totalQuestions(), submittedAt));
        saveAnswerSheetToFirebase(sheetId, student->getUserId(), exam.getExamId(), answers, submittedAt, score);
        saveResultToFirebase(resultId, student->getUserId(), exam.getExamId(),
                             score, exam.totalQuestions(), submittedAt);
    }

    int countAttempts(string studentId, string examId) {
        int count = 0;
        for (const Result& result : results) {
            if (result.getStudentId() == studentId && result.getExamId() == examId) {
                count++;
            }
        }
        return count;
    }

    bool loadFromFirebase() {
        vector<shared_ptr<User>> loadedUsers;
        vector<Question> loadedQuestions;
        vector<Exam> loadedExams;
        vector<Result> loadedResults;

        json adminsJson;
        json teachersJson;
        json studentsJson;
        json questionsJson;
        json examsJson;
        json resultsJson;

        if (!firebase.getCollection("admins", adminsJson) ||
            !firebase.getCollection("teachers", teachersJson) ||
            !firebase.getCollection("students", studentsJson) ||
            !firebase.getCollection("questions", questionsJson) ||
            !firebase.getCollection("exams", examsJson) ||
            !firebase.getCollection("results", resultsJson)) {
            connectedToFirebase = false;
            firebaseStatus = "Không kết nối được cơ sở dữ liệu. Đang dùng dữ liệu trong bộ nhớ.\r\n" + firebase.getLastError();
            return false;
        }

        try {
            if (adminsJson.contains("documents")) {
                for (const json& doc : adminsJson["documents"]) {
                    string userId = firebase.documentIdFromName(doc.value("name", ""));
                    const json& fields = doc.at("fields");
                    loadedUsers.push_back(make_shared<Admin>(
                        userId,
                        firebase.stringField(fields, "name"),
                        firebase.stringField(fields, "email"),
                        firebase.stringField(fields, "passwordHash")
                    ));
                }
            }

            if (teachersJson.contains("documents")) {
                for (const json& doc : teachersJson["documents"]) {
                    string userId = firebase.documentIdFromName(doc.value("name", ""));
                    const json& fields = doc.at("fields");
                    loadedUsers.push_back(make_shared<Teacher>(
                        userId,
                        firebase.stringField(fields, "name"),
                        firebase.stringField(fields, "email"),
                        firebase.stringField(fields, "passwordHash"),
                        firebase.stringField(fields, "className")
                    ));
                }
            }

            if (studentsJson.contains("documents")) {
                for (const json& doc : studentsJson["documents"]) {
                    string userId = firebase.documentIdFromName(doc.value("name", ""));
                    const json& fields = doc.at("fields");
                    loadedUsers.push_back(make_shared<Student>(
                        userId,
                        firebase.stringField(fields, "name"),
                        firebase.stringField(fields, "email"),
                        firebase.stringField(fields, "passwordHash"),
                        firebase.stringField(fields, "className")
                    ));
                }
            }

            if (questionsJson.contains("documents")) {
                for (const json& doc : questionsJson["documents"]) {
                    string questionId = firebase.documentIdFromName(doc.value("name", ""));
                    const json& fields = doc.at("fields");
                    array<string, 4> options = {
                        firebase.stringField(fields, "optionA"),
                        firebase.stringField(fields, "optionB"),
                        firebase.stringField(fields, "optionC"),
                        firebase.stringField(fields, "optionD"),
                    };
                    string correctAnswer = firebase.stringField(fields, "correctAnswer", "A");
                    loadedQuestions.push_back(Question(
                        questionId,
                        firebase.stringField(fields, "content"),
                        options,
                        correctAnswer.empty() ? 'A' : correctAnswer[0],
                        firebase.stringField(fields, "subject"),
                        firebase.stringField(fields, "difficulty"),
                        firebase.stringField(fields, "imagePath"),
                        firebase.stringField(fields, "imageUrl")
                    ));
                }
            }

            if (examsJson.contains("documents")) {
                for (const json& doc : examsJson["documents"]) {
                    string examId = firebase.documentIdFromName(doc.value("name", ""));
                    const json& fields = doc.at("fields");
                    loadedExams.push_back(Exam(
                        examId,
                        firebase.stringField(fields, "title"),
                        firebase.intField(fields, "durationMinutes", 30),
                        firebase.stringArrayField(fields, "questionIds"),
                        firebase.stringField(fields, "createdBy"),
                        firebase.stringField(fields, "startAt"),
                        firebase.stringField(fields, "closeAt"),
                        firebase.stringField(fields, "examPassword"),
                        firebase.stringField(fields, "attachmentPath"),
                        firebase.stringField(fields, "attachmentUrl"),
                        charArrayFromStrings(firebase.stringArrayField(fields, "answerKey")),
                        firebase.intField(fields, "attemptLimit", 1),
                        firebase.boolField(fields, "isOpen", true)
                    ));
                }
            }

            if (resultsJson.contains("documents")) {
                for (const json& doc : resultsJson["documents"]) {
                    string resultId = firebase.documentIdFromName(doc.value("name", ""));
                    const json& fields = doc.at("fields");
                    loadedResults.push_back(Result(
                        resultId,
                        firebase.stringField(fields, "studentId"),
                        firebase.stringField(fields, "examId"),
                        firebase.intField(fields, "score", 0),
                        firebase.intField(fields, "totalQuestions", 0),
                        firebase.stringField(fields, "submittedAt")
                    ));
                }
            }

            if (!loadedUsers.empty()) users = loadedUsers;
            if (!loadedQuestions.empty()) questions = loadedQuestions;
            if (!loadedExams.empty()) exams = loadedExams;
            if (!loadedResults.empty()) results = loadedResults;

            resetUserCounters();
            nextQuestionNumber = (int)questions.size() + 1;
            nextExamNumber = (int)exams.size() + 1;

            connectedToFirebase = true;
            firebaseStatus = "Dữ liệu đã sẵn sàng.";
            return true;
        } catch (const exception& error) {
            connectedToFirebase = false;
            firebaseStatus = string("Đọc dữ liệu Firestore thất bại. Đang dùng RAM. Lỗi: ") + error.what();
            return false;
        }
    }

    bool loadUsersFromFirebase() {
        vector<shared_ptr<User>> loadedUsers;

        json adminsJson;
        json teachersJson;
        json studentsJson;

        if (!firebase.getCollection("admins", adminsJson) ||
            !firebase.getCollection("teachers", teachersJson) ||
            !firebase.getCollection("students", studentsJson)) {
            connectedToFirebase = false;
            firebaseStatus = "Không kết nối được Firestore. Đang dùng dữ liệu hiện có trong RAM.\r\n" + firebase.getLastError();
            return false;
        }

        try {
            if (adminsJson.contains("documents")) {
                for (const json& doc : adminsJson["documents"]) {
                    string userId = firebase.documentIdFromName(doc.value("name", ""));
                    const json& fields = doc.at("fields");
                    loadedUsers.push_back(make_shared<Admin>(
                        userId,
                        firebase.stringField(fields, "name"),
                        firebase.stringField(fields, "email"),
                        firebase.stringField(fields, "passwordHash")
                    ));
                }
            }

            if (teachersJson.contains("documents")) {
                for (const json& doc : teachersJson["documents"]) {
                    string userId = firebase.documentIdFromName(doc.value("name", ""));
                    const json& fields = doc.at("fields");
                    loadedUsers.push_back(make_shared<Teacher>(
                        userId,
                        firebase.stringField(fields, "name"),
                        firebase.stringField(fields, "email"),
                        firebase.stringField(fields, "passwordHash"),
                        firebase.stringField(fields, "className")
                    ));
                }
            }

            if (studentsJson.contains("documents")) {
                for (const json& doc : studentsJson["documents"]) {
                    string userId = firebase.documentIdFromName(doc.value("name", ""));
                    const json& fields = doc.at("fields");
                    loadedUsers.push_back(make_shared<Student>(
                        userId,
                        firebase.stringField(fields, "name"),
                        firebase.stringField(fields, "email"),
                        firebase.stringField(fields, "passwordHash"),
                        firebase.stringField(fields, "className")
                    ));
                }
            }

            if (!loadedUsers.empty()) {
                users = loadedUsers;
            }

            resetUserCounters();
            connectedToFirebase = true;
            firebaseStatus = "Dữ liệu đã sẵn sàng.";
            return true;
        } catch (const exception& error) {
            connectedToFirebase = false;
            firebaseStatus = string("Đọc tài khoản từ Firestore thất bại. Lỗi: ") + error.what();
            return false;
        }
    }

    bool saveUserToFirebase(string userId, string name, string email, string password, string role, string className, bool createOnly = false) {
        json fields;
        fields["name"] = firebase.stringValue(name);
        fields["email"] = firebase.stringValue(email);
        fields["passwordHash"] = firebase.stringValue(password);
        fields["role"] = firebase.stringValue(role);
        fields["className"] = firebase.stringValue(className);

        string collectionName = "students";
        if (role == "admin") {
            collectionName = "admins";
        } else if (role == "teacher") {
            collectionName = "teachers";
        }

        bool saved = createOnly
            ? firebase.createDocument(collectionName, userId, fields)
            : firebase.setDocument(collectionName, userId, fields);

        if (!saved) {
            connectedToFirebase = false;
            firebaseStatus = "Không ghi được Firestore.\r\n" + firebase.getLastError();
            return false;
        }

        connectedToFirebase = true;
        firebaseStatus = "Dữ liệu đã sẵn sàng.";
        return true;
    }

    string collectionForRole(string role) {
        if (role == "admin") {
            return "admins";
        }
        if (role == "teacher") {
            return "teachers";
        }
        if (role == "student") {
            return "students";
        }
        return "";
    }

    void saveQuestionToFirebase(string questionId, string content, array<string, 4> options,
                                char correctAnswer, string subject, string imagePath, string imageUrl) {
        json fields;
        fields["content"] = firebase.stringValue(content);
        fields["optionA"] = firebase.stringValue(options[0]);
        fields["optionB"] = firebase.stringValue(options[1]);
        fields["optionC"] = firebase.stringValue(options[2]);
        fields["optionD"] = firebase.stringValue(options[3]);
        fields["correctAnswer"] = firebase.stringValue(string(1, correctAnswer));
        fields["subject"] = firebase.stringValue(subject);
        fields["imagePath"] = firebase.stringValue(imagePath);
        fields["imageUrl"] = firebase.stringValue(imageUrl);
        firebase.setDocument("questions", questionId, fields);
    }

    void saveExamToFirebase(string examId, string title, int duration, vector<string> questionIds,
                            string createdBy, string startAt, string closeAt, string examPassword,
                            string attachmentPath, string attachmentUrl, vector<char> answerKey,
                            int attemptLimit, bool isOpen) {
        json fields;
        fields["title"] = firebase.stringValue(title);
        fields["durationMinutes"] = firebase.intValue(duration);
        fields["questionIds"] = firebase.stringArrayValue(questionIds);
        fields["createdBy"] = firebase.stringValue(createdBy);
        fields["startAt"] = firebase.stringValue(startAt);
        fields["closeAt"] = firebase.stringValue(closeAt);
        fields["examPassword"] = firebase.stringValue(examPassword);
        fields["attachmentPath"] = firebase.stringValue(attachmentPath);
        fields["attachmentUrl"] = firebase.stringValue(attachmentUrl);
        fields["answerKey"] = firebase.stringArrayValue(stringsFromChars(answerKey));
        fields["attemptLimit"] = firebase.intValue(attemptLimit);
        fields["isOpen"] = firebase.boolValue(isOpen);
        firebase.setDocument("exams", examId, fields);
    }

    void saveAnswerSheetToFirebase(string sheetId, string studentId, string examId,
                                   map<string, char> answers, string submittedAt, int score) {
        json fields;
        fields["studentId"] = firebase.stringValue(studentId);
        fields["examId"] = firebase.stringValue(examId);
        fields["answers"] = firebase.answerMapValue(answers);
        fields["submittedAt"] = firebase.stringValue(submittedAt);
        fields["score"] = firebase.intValue(score);
        firebase.setDocument("answerSheets", sheetId, fields);
    }

    void saveResultToFirebase(string resultId, string studentId, string examId,
                              int score, int totalQuestions, string submittedAt) {
        json fields;
        fields["studentId"] = firebase.stringValue(studentId);
        fields["examId"] = firebase.stringValue(examId);
        fields["score"] = firebase.intValue(score);
        fields["totalQuestions"] = firebase.intValue(totalQuestions);
        fields["submittedAt"] = firebase.stringValue(submittedAt);
        firebase.setDocument("results", resultId, fields);
    }

    void resetUserCounters() {
        nextAdminNumber = 1;
        nextTeacherNumber = 1;
        nextStudentNumber = 1;

        for (shared_ptr<User> user : users) {
            string id = user->getUserId();
            if (user->getRole() == "admin") {
                nextAdminNumber = max(nextAdminNumber, numberAfterPrefix(id, 'a') + 1);
            } else if (user->getRole() == "teacher") {
                nextTeacherNumber = max(nextTeacherNumber, numberAfterPrefix(id, 't') + 1);
            } else if (user->getRole() == "student") {
                nextStudentNumber = max(nextStudentNumber, numberAfterPrefix(id, 's') + 1);
            }
        }
    }

    int numberAfterPrefix(string id, char prefix) {
        if (id.size() < 2 || id[0] != prefix) {
            return 0;
        }
        return atoi(id.substr(1).c_str());
    }

    string createAdminId() { return createUserId('a', nextAdminNumber); }
    string createTeacherId() { return createUserId('t', nextTeacherNumber); }
    string createStudentId() { return createUserId('s', nextStudentNumber); }
    string createQuestionId() { return "q" + to_string(nextQuestionNumber++); }
    string createExamId() { return "e" + to_string(nextExamNumber++); }
    string createSheetId() { return "s" + to_string(nextSheetNumber++); }
    string createResultId() { return "r" + to_string(nextResultNumber++); }

    string createUserId(char prefix, int& nextNumber) {
        string userId;
        do {
            userId = string(1, prefix) + to_string(nextNumber++);
        } while (findUserById(userId) != nullptr);

        return userId;
    }

    string createStudentIdFromEmail(string email) {
        string userId = "s_";
        for (char ch : email) {
            unsigned char value = (unsigned char)ch;
            if (isalnum(value)) {
                userId += (char)tolower(value);
            } else {
                userId += "_";
            }
        }
        return userId;
    }

    string lowerText(string text) {
        transform(text.begin(), text.end(), text.begin(),
                  [](unsigned char ch) { return (char)tolower(ch); });
        return text;
    }

    void trimInPlace(string& text) {
        while (!text.empty() && isspace((unsigned char)text.front())) {
            text.erase(text.begin());
        }
        while (!text.empty() && isspace((unsigned char)text.back())) {
            text.pop_back();
        }
    }

    vector<string> parseCsvLine(string line) {
        vector<string> columns;
        string current;
        bool inQuotes = false;

        for (int i = 0; i < (int)line.size(); i++) {
            char ch = line[i];
            if (ch == '"') {
                if (inQuotes && i + 1 < (int)line.size() && line[i + 1] == '"') {
                    current += '"';
                    i++;
                } else {
                    inQuotes = !inQuotes;
                }
            } else if (ch == ',' && !inQuotes) {
                columns.push_back(current);
                current.clear();
            } else {
                current += ch;
            }
        }

        columns.push_back(current);
        return columns;
    }

    static string currentTimeText() {
        auto now = chrono::system_clock::now();
        time_t timeNow = chrono::system_clock::to_time_t(now);
        tm localTime;
        localtime_s(&localTime, &timeNow);

        stringstream ss;
        ss << put_time(&localTime, "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }

    static string currentMinuteText() {
        auto now = chrono::system_clock::now();
        time_t timeNow = chrono::system_clock::to_time_t(now);
        tm localTime;
        localtime_s(&localTime, &timeNow);

        stringstream ss;
        ss << put_time(&localTime, "%Y-%m-%d %H:%M");
        return ss.str();
    }
};

enum ControlId {
    ID_MAIN_REGISTER = 1001,
    ID_MAIN_LOGIN,
    ID_BACK_MAIN,
    ID_LOGIN_SUBMIT,
    ID_REGISTER_SUBMIT,
    ID_PROFILE,
    ID_PROFILE_SAVE,
    ID_HELP,
    ID_LOGOUT,
    ID_ADMIN_STUDENTS,
    ID_ADMIN_TEACHERS,
    ID_ADMIN_CREATE_TEACHER,
    ID_ADMIN_QUESTIONS,
    ID_ADMIN_EXAMS,
    ID_ADMIN_TOGGLE_EXAM,
    ID_ADMIN_RESULTS,
    ID_TEACHER_QUESTIONS,
    ID_TEACHER_ADD_QUESTION,
    ID_TEACHER_IMPORT_QUESTIONS,
    ID_TEACHER_DELETE_QUESTION,
    ID_TEACHER_EXAMS,
    ID_TEACHER_CREATE_EXAM,
    ID_TEACHER_TOGGLE_EXAM,
    ID_TEACHER_RESULTS,
    ID_STUDENT_OPEN_EXAMS,
    ID_STUDENT_TAKE_EXAM,
    ID_STUDENT_RESULTS,
    ID_CREATE_TEACHER_SUBMIT,
    ID_ADD_QUESTION_CHOOSE_IMAGE,
    ID_ADD_QUESTION_SUBMIT,
    ID_IMPORT_QUESTIONS_SUBMIT,
    ID_IMPORT_QUESTIONS_FROM_URL,
    ID_IMPORT_QUESTIONS_FROM_FILE,
    ID_IMPORT_QUESTIONS_TEMPLATE,
    ID_DELETE_QUESTION_SUBMIT,
    ID_CREATE_EXAM_CHOOSE_FILE,
    ID_CREATE_EXAM_SUBMIT,
    ID_TOGGLE_EXAM_SUBMIT,
    ID_UPDATE_EXAM_ATTEMPT_LIMIT,
    ID_TAKE_EXAM_OPEN_FILE,
    ID_TAKE_EXAM_SUBMIT,
    ID_STUDENT_DELETE,
    ID_STUDENT_CHANGE_CLASS,
    ID_STUDENT_SEARCH,
    ID_STUDENT_RESET_PASSWORD,
    ID_TEACHER_DELETE,
    ID_TEACHER_CHANGE_CLASS,
    ID_TEACHER_SEARCH,
    ID_TEACHER_RESET_PASSWORD,
    ID_DASHBOARD
};

enum AppScreen {
    SCREEN_MAIN,
    SCREEN_LOGIN,
    SCREEN_REGISTER,
    SCREEN_PROFILE,
    SCREEN_ADMIN_DASHBOARD,
    SCREEN_TEACHER_DASHBOARD,
    SCREEN_STUDENT_DASHBOARD,
    SCREEN_CREATE_TEACHER,
    SCREEN_ADD_QUESTION,
    SCREEN_IMPORT_QUESTIONS,
    SCREEN_DELETE_QUESTION,
    SCREEN_CREATE_EXAM,
    SCREEN_TOGGLE_EXAM,
    SCREEN_TAKE_EXAM_SELECT,
    SCREEN_TAKE_EXAM_FORM,
    SCREEN_VIEW
};

class GuiApp {
private:
    static constexpr COLORREF THEME_BACKGROUND = RGB(248, 250, 252);
    static constexpr COLORREF THEME_SURFACE = RGB(255, 255, 255);
    static constexpr COLORREF THEME_SIDEBAR = RGB(239, 246, 255);
    static constexpr COLORREF THEME_BORDER = RGB(203, 213, 225);
    static constexpr COLORREF THEME_TEXT = RGB(15, 23, 42);
    static constexpr COLORREF THEME_MUTED = RGB(71, 85, 105);
    static constexpr COLORREF THEME_PRIMARY = RGB(37, 99, 235);
    static constexpr COLORREF THEME_PRIMARY_HOVER = RGB(29, 78, 216);

    HINSTANCE instance;
    HWND window = nullptr;
    HFONT font = nullptr;
    HFONT titleFont = nullptr;
    HFONT brandFont = nullptr;
    HFONT smallFont = nullptr;
    HBRUSH backgroundBrush = nullptr;
    HBRUSH surfaceBrush = nullptr;
    HBRUSH sidebarBrush = nullptr;
    HBRUSH inputBrush = nullptr;
    HBRUSH loginBackgroundBrush = nullptr;
    QuizData data;
    shared_ptr<User> currentUser = nullptr;
    vector<HWND> controls;
    vector<HBITMAP> imageBitmaps;
    vector<HIMAGELIST> listImageLists;
    vector<IPreviewHandler*> previewHandlers;
    map<HWND, IPreviewHandler*> previewHandlerByHost;
    map<HWND, bool> primaryButtons;
    map<HWND, bool> answerChoiceButtons;
    map<HWND, COLORREF> controlTextColors;
    map<HWND, bool> surfaceLabels;
    vector<array<HWND, 4>> answerOptions;
    vector<HWND> answerCombos;
    vector<string> activeQuestionIds;
    string activeExamId;
    HWND examTimerLabel = nullptr;
    chrono::system_clock::time_point activeExamEndTime;
    bool examTimerActive = false;
    AppScreen currentScreen = SCREEN_MAIN;
    ULONG_PTR gdiplusToken = 0;
    bool comInitialized = false;
    static constexpr UINT_PTR EXAM_TIMER_ID = 9001;

public:
    GuiApp(HINSTANCE instance) : instance(instance) {}

    int run() {
        SetProcessDPIAware();
        HRESULT comResult = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
        comInitialized = SUCCEEDED(comResult);

        GdiplusStartupInput gdiplusStartupInput;
        GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);

        backgroundBrush = CreateSolidBrush(THEME_BACKGROUND);
        surfaceBrush = CreateSolidBrush(THEME_SURFACE);
        sidebarBrush = CreateSolidBrush(THEME_SIDEBAR);
        inputBrush = CreateSolidBrush(THEME_SURFACE);
        loginBackgroundBrush = CreateSolidBrush(RGB(243, 244, 246));

        WNDCLASSW wc = {};
        wc.lpfnWndProc = GuiApp::windowProc;
        wc.hInstance = instance;
        wc.lpszClassName = L"QuizAppGuiWindow";
        wc.hbrBackground = backgroundBrush;
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

        RegisterClassW(&wc);

        WNDCLASSW previewWc = {};
        previewWc.lpfnWndProc = GuiApp::previewHostProc;
        previewWc.hInstance = instance;
        previewWc.lpszClassName = L"QuizPreviewHostWindow";
        previewWc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        previewWc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        RegisterClassW(&previewWc);

        WNDCLASSW cardWc = {};
        cardWc.lpfnWndProc = GuiApp::cardPanelProc;
        cardWc.hInstance = instance;
        cardWc.lpszClassName = L"QuizCardPanel";
        cardWc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        RegisterClassW(&cardWc);

        font = CreateFontW(19, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                           DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                           CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
        titleFont = CreateFontW(30, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
                                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
        brandFont = CreateFontW(22, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
                                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
        smallFont = CreateFontW(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");

        INITCOMMONCONTROLSEX controlsInfo = {};
        controlsInfo.dwSize = sizeof(controlsInfo);
        controlsInfo.dwICC = ICC_LISTVIEW_CLASSES;
        InitCommonControlsEx(&controlsInfo);

        int windowWidth = 1280;
        int windowHeight = 840;
        int windowX = max(0, (GetSystemMetrics(SM_CXSCREEN) - windowWidth) / 2);
        int windowY = max(0, (GetSystemMetrics(SM_CYSCREEN) - windowHeight) / 2);
        window = CreateWindowW(L"QuizAppGuiWindow", L"HCMUTE Quiz App",
                               WS_OVERLAPPEDWINDOW,
                               windowX, windowY, windowWidth, windowHeight,
                               nullptr, nullptr, instance, this);
        DragAcceptFiles(window, TRUE);

        ShowWindow(window, SW_SHOW);
        UpdateWindow(window);
        showLoginScreen();

        MSG msg = {};
        while (GetMessage(&msg, nullptr, 0, 0)) {
            if (handleShortcut(msg)) {
                continue;
            }

            if (translatePreviewMessage(msg)) {
                continue;
            }

            if (!IsDialogMessageW(window, &msg)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }

        if (gdiplusToken != 0) {
            GdiplusShutdown(gdiplusToken);
            gdiplusToken = 0;
        }
        if (comInitialized) {
            CoUninitialize();
            comInitialized = false;
        }

        return 0;
    }

    LRESULT handleMessage(UINT message, WPARAM wParam, LPARAM lParam) {
        if (message == WM_ERASEBKGND) {
            paintAppBackground((HDC)wParam);
            return 1;
        }

        if (message == WM_CTLCOLORSTATIC) {
            HDC hdc = (HDC)wParam;
            HWND control = (HWND)lParam;
            auto color = controlTextColors.find(control);
            SetTextColor(hdc, color != controlTextColors.end() ? color->second : THEME_TEXT);
            SetBkMode(hdc, TRANSPARENT);
            if (surfaceLabels.find(control) != surfaceLabels.end()) {
                return (LRESULT)surfaceBrush;
            }
            if (currentScreen == SCREEN_LOGIN) {
                return (LRESULT)loginBackgroundBrush;
            }
            return (LRESULT)backgroundBrush;
        }

        if (message == WM_CTLCOLOREDIT || message == WM_CTLCOLORLISTBOX) {
            HDC hdc = (HDC)wParam;
            SetTextColor(hdc, THEME_TEXT);
            SetBkColor(hdc, THEME_SURFACE);
            return (LRESULT)inputBrush;
        }

        if (message == WM_DRAWITEM) {
            DRAWITEMSTRUCT* drawInfo = (DRAWITEMSTRUCT*)lParam;
            if (drawInfo != nullptr && drawInfo->CtlType == ODT_BUTTON) {
                if (answerChoiceButtons.find(drawInfo->hwndItem) != answerChoiceButtons.end()) {
                    drawAnswerChoiceButton(drawInfo);
                } else {
                    drawModernButton(drawInfo);
                }
                return TRUE;
            }
        }

        if (message == WM_COMMAND) {
            handleCommand(LOWORD(wParam), HIWORD(wParam));
            return 0;
        }

        if (message == WM_TIMER && wParam == EXAM_TIMER_ID) {
            updateExamTimer();
            return 0;
        }

        if (message == WM_DROPFILES) {
            loadDroppedCsv((HDROP)wParam);
            return 0;
        }

        if (message == WM_MOUSEWHEEL) {
            HWND previewHost = previewHostFromPoint(lParam);
            if (previewHost != nullptr) {
                focusPreviewHandler(previewHost);
                POINT point = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
                HWND target = WindowFromPoint(point);
                if (target != nullptr && target != window && target != previewHost) {
                    SendMessageW(target, message, wParam, lParam);
                    return 0;
                }
            }
        }

        if (message == WM_DESTROY) {
            stopExamTimer();
            releaseThemeObjects();
            PostQuitMessage(0);
            return 0;
        }

        return DefWindowProcW(window, message, wParam, lParam);
    }

private:
    static LRESULT CALLBACK windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
        GuiApp* app = nullptr;

        if (message == WM_NCCREATE) {
            CREATESTRUCTA* create = (CREATESTRUCTA*)lParam;
            app = (GuiApp*)create->lpCreateParams;
            SetWindowLongPtrA(hwnd, GWLP_USERDATA, (LONG_PTR)app);
            app->window = hwnd;
        } else {
            app = (GuiApp*)GetWindowLongPtrA(hwnd, GWLP_USERDATA);
        }

        if (app != nullptr) {
            return app->handleMessage(message, wParam, lParam);
        }

        return DefWindowProcW(hwnd, message, wParam, lParam);
    }

    static LRESULT CALLBACK previewHostProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
        GuiApp* app = nullptr;

        if (message == WM_NCCREATE) {
            CREATESTRUCTW* create = (CREATESTRUCTW*)lParam;
            app = (GuiApp*)create->lpCreateParams;
            SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)app);
        } else {
            app = (GuiApp*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
        }

        if (app != nullptr) {
            LRESULT result = 0;
            if (app->handlePreviewHostMessage(hwnd, message, wParam, lParam, result)) {
                return result;
            }
        }

        return DefWindowProcW(hwnd, message, wParam, lParam);
    }

    static LRESULT CALLBACK cardPanelProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
        if (message == WM_ERASEBKGND) {
            return 1;
        }

        if (message == WM_PAINT) {
            PAINTSTRUCT ps = {};
            HDC hdc = BeginPaint(hwnd, &ps);
            RECT rect = {};
            GetClientRect(hwnd, &rect);

            HBRUSH fillBrush = CreateSolidBrush(THEME_SURFACE);
            HPEN borderPen = CreatePen(PS_SOLID, 1, THEME_BORDER);
            HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, fillBrush);
            HPEN oldPen = (HPEN)SelectObject(hdc, borderPen);

            RoundRect(hdc, rect.left, rect.top, rect.right - 1, rect.bottom - 1, 18, 18);

            SelectObject(hdc, oldPen);
            SelectObject(hdc, oldBrush);
            DeleteObject(borderPen);
            DeleteObject(fillBrush);
            EndPaint(hwnd, &ps);
            return 0;
        }

        return DefWindowProcW(hwnd, message, wParam, lParam);
    }

    bool handlePreviewHostMessage(HWND host, UINT message, WPARAM wParam, LPARAM lParam, LRESULT& result) {
        if (message == WM_MOUSEACTIVATE) {
            focusPreviewHandler(host);
            result = MA_ACTIVATE;
            return true;
        }

        if (message == WM_SETFOCUS) {
            focusPreviewHandler(host);
            result = 0;
            return true;
        }

        if (message == WM_MOUSEWHEEL ||
            message == WM_LBUTTONDOWN || message == WM_LBUTTONUP ||
            message == WM_LBUTTONDBLCLK || message == WM_MOUSEMOVE ||
            message == WM_RBUTTONDOWN || message == WM_RBUTTONUP) {
            focusPreviewHandler(host);
            if (forwardPreviewMouseMessage(host, message, wParam, lParam)) {
                result = 0;
                return true;
            }
        }

        return false;
    }

    void paintAppBackground(HDC hdc) {
        RECT client = {};
        GetClientRect(window, &client);

        if (currentScreen == SCREEN_LOGIN) {
            FillRect(hdc, &client, loginBackgroundBrush);

            int centerX = (int)(client.right - client.left) / 2;
            RECT card = { centerX - 270, 232, centerX + 270, 752 };
            HBRUSH cardBrush = CreateSolidBrush(THEME_SURFACE);
            HPEN cardPen = CreatePen(PS_SOLID, 1, THEME_BORDER);
            HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, cardBrush);
            HPEN oldPen = (HPEN)SelectObject(hdc, cardPen);
            RoundRect(hdc, card.left, card.top, card.right, card.bottom, 12, 12);
            SelectObject(hdc, oldPen);
            SelectObject(hdc, oldBrush);
            DeleteObject(cardPen);
            DeleteObject(cardBrush);
            return;
        }

        FillRect(hdc, &client, backgroundBrush);

        RECT sidebar = { 0, 0, 240, client.bottom };
        FillRect(hdc, &sidebar, sidebarBrush);

        HPEN borderPen = CreatePen(PS_SOLID, 1, THEME_BORDER);
        HPEN oldPen = (HPEN)SelectObject(hdc, borderPen);
        MoveToEx(hdc, 240, 0, nullptr);
        LineTo(hdc, 240, client.bottom);
        SelectObject(hdc, oldPen);
        DeleteObject(borderPen);
    }

    void drawModernButton(DRAWITEMSTRUCT* drawInfo) {
        HWND button = drawInfo->hwndItem;
        bool primary = primaryButtons.find(button) != primaryButtons.end() && primaryButtons[button];
        bool pressed = (drawInfo->itemState & ODS_SELECTED) != 0;
        bool focused = (drawInfo->itemState & ODS_FOCUS) != 0;
        bool disabled = (drawInfo->itemState & ODS_DISABLED) != 0;

        RECT rect = drawInfo->rcItem;
        HDC hdc = drawInfo->hDC;

        COLORREF fillColor = primary ? THEME_PRIMARY : THEME_SURFACE;
        COLORREF borderColor = primary ? THEME_PRIMARY : THEME_BORDER;
        COLORREF textColor = primary ? RGB(255, 255, 255) : THEME_TEXT;

        if (pressed && primary) {
            fillColor = THEME_PRIMARY_HOVER;
            borderColor = THEME_PRIMARY_HOVER;
        } else if (pressed) {
            fillColor = RGB(226, 232, 240);
        }
        if (disabled) {
            fillColor = RGB(226, 232, 240);
            borderColor = RGB(203, 213, 225);
            textColor = RGB(100, 116, 139);
        }

        HBRUSH fillBrush = CreateSolidBrush(fillColor);
        HPEN borderPen = CreatePen(PS_SOLID, focused ? 2 : 1, borderColor);
        HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, fillBrush);
        HPEN oldPen = (HPEN)SelectObject(hdc, borderPen);

        int offset = pressed ? 1 : 0;
        RECT buttonRect = rect;
        InflateRect(&buttonRect, -1, -1);
        RoundRect(hdc, buttonRect.left, buttonRect.top, buttonRect.right, buttonRect.bottom, 8, 8);

        SelectObject(hdc, oldBrush);
        SelectObject(hdc, oldPen);
        DeleteObject(fillBrush);
        DeleteObject(borderPen);

        wchar_t text[256] = L"";
        GetWindowTextW(button, text, 256);
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, textColor);
        HFONT oldFont = (HFONT)SelectObject(hdc, font);
        RECT textRect = rect;
        OffsetRect(&textRect, offset, offset);
        DrawTextW(hdc, text, -1, &textRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
        SelectObject(hdc, oldFont);
    }

    void drawAnswerChoiceButton(DRAWITEMSTRUCT* drawInfo) {
        HWND button = drawInfo->hwndItem;
        auto selectedIterator = answerChoiceButtons.find(button);
        bool checked = selectedIterator != answerChoiceButtons.end() && selectedIterator->second;
        bool pressed = (drawInfo->itemState & ODS_SELECTED) != 0;
        bool focused = (drawInfo->itemState & ODS_FOCUS) != 0;

        RECT rect = drawInfo->rcItem;
        HDC hdc = drawInfo->hDC;

        wchar_t text[512] = L"";
        GetWindowTextW(button, text, 512);
        bool compactBubble = wcslen(text) <= 2;

        COLORREF fillColor = checked ? THEME_PRIMARY : THEME_SURFACE;
        COLORREF borderColor = checked ? THEME_PRIMARY : RGB(148, 163, 184);
        COLORREF textColor = checked ? RGB(255, 255, 255) : THEME_TEXT;

        if (checked && pressed) {
            fillColor = THEME_PRIMARY_HOVER;
            borderColor = THEME_PRIMARY_HOVER;
        } else if (!checked && pressed) {
            fillColor = RGB(219, 234, 254);
            borderColor = THEME_PRIMARY;
        } else if (!checked && focused) {
            borderColor = THEME_PRIMARY;
        }

        HBRUSH fillBrush = CreateSolidBrush(fillColor);
        HPEN borderPen = CreatePen(PS_SOLID, checked || focused ? 2 : 1, borderColor);
        HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, fillBrush);
        HPEN oldPen = (HPEN)SelectObject(hdc, borderPen);

        RECT buttonRect = rect;
        InflateRect(&buttonRect, -1, -1);
        if (compactBubble) {
            int diameter = min(buttonRect.right - buttonRect.left - 4, buttonRect.bottom - buttonRect.top - 4);
            int centerX = (buttonRect.left + buttonRect.right) / 2;
            int centerY = (buttonRect.top + buttonRect.bottom) / 2;
            RECT circle = {
                centerX - diameter / 2,
                centerY - diameter / 2,
                centerX + diameter / 2,
                centerY + diameter / 2
            };
            Ellipse(hdc, circle.left, circle.top, circle.right, circle.bottom);
        } else {
            RoundRect(hdc, buttonRect.left, buttonRect.top, buttonRect.right, buttonRect.bottom, 9, 9);
        }

        SelectObject(hdc, oldBrush);
        SelectObject(hdc, oldPen);
        DeleteObject(fillBrush);
        DeleteObject(borderPen);

        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, textColor);

        HFONT oldFont = (HFONT)SelectObject(hdc, smallFont);
        RECT textRect = rect;
        if (compactBubble) {
            DrawTextW(hdc, text, -1, &textRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        } else {
            textRect.left += 14;
            textRect.right -= 10;
            DrawTextW(hdc, text, -1, &textRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
        }
        SelectObject(hdc, oldFont);
    }

    void releaseThemeObjects() {
        if (font != nullptr) {
            DeleteObject(font);
            font = nullptr;
        }
        if (titleFont != nullptr) {
            DeleteObject(titleFont);
            titleFont = nullptr;
        }
        if (brandFont != nullptr) {
            DeleteObject(brandFont);
            brandFont = nullptr;
        }
        if (smallFont != nullptr) {
            DeleteObject(smallFont);
            smallFont = nullptr;
        }
        if (backgroundBrush != nullptr) {
            DeleteObject(backgroundBrush);
            backgroundBrush = nullptr;
        }
        if (surfaceBrush != nullptr) {
            DeleteObject(surfaceBrush);
            surfaceBrush = nullptr;
        }
        if (sidebarBrush != nullptr) {
            DeleteObject(sidebarBrush);
            sidebarBrush = nullptr;
        }
        if (inputBrush != nullptr) {
            DeleteObject(inputBrush);
            inputBrush = nullptr;
        }
        if (loginBackgroundBrush != nullptr) {
            DeleteObject(loginBackgroundBrush);
            loginBackgroundBrush = nullptr;
        }
    }

    void handleCommand(int id, int notifyCode) {
        if (notifyCode == BN_CLICKED && id >= 5300 && id < 6200 && selectAnswerChoice(id)) {
            return;
        }

        if (id == ID_MAIN_REGISTER) showRegisterScreen();
        else if (id == ID_MAIN_LOGIN) showLoginScreen();
        else if (id == ID_BACK_MAIN) showMainScreen();
        else if (id == ID_LOGIN_SUBMIT) submitLogin();
        else if (id == ID_REGISTER_SUBMIT) submitRegisterStudent();
        else if (id == ID_PROFILE) showProfileScreen();
        else if (id == ID_PROFILE_SAVE) submitProfileUpdate();
        else if (id == ID_HELP) showUserGuide();
        else if (id == ID_LOGOUT) { currentUser = nullptr; showMainScreen(); }
        else if (id == ID_ADMIN_STUDENTS) showStudents();
        else if (id == ID_ADMIN_TEACHERS) showTeachers();
        else if (id == ID_ADMIN_CREATE_TEACHER) showCreateTeacher();
        else if (id == ID_ADMIN_QUESTIONS) showQuestions(true);
        else if (id == ID_ADMIN_EXAMS) showExams();
        else if (id == ID_ADMIN_TOGGLE_EXAM) showToggleExam();
        else if (id == ID_ADMIN_RESULTS) showAllResults();
        else if (id == ID_TEACHER_QUESTIONS) showQuestions(true);
        else if (id == ID_TEACHER_ADD_QUESTION) showAddQuestion();
        else if (id == ID_TEACHER_IMPORT_QUESTIONS) showImportQuestions();
        else if (id == ID_TEACHER_DELETE_QUESTION) showDeleteQuestion();
        else if (id == ID_TEACHER_EXAMS) showExams();
        else if (id == ID_TEACHER_CREATE_EXAM) showCreateExam();
        else if (id == ID_TEACHER_TOGGLE_EXAM) showToggleExam();
        else if (id == ID_TEACHER_RESULTS) showAllResults();
        else if (id == ID_STUDENT_OPEN_EXAMS) showOpenExams();
        else if (id == ID_STUDENT_TAKE_EXAM) showTakeExamSelect();
        else if (id == ID_STUDENT_RESULTS) showMyResults();
        else if (id == ID_CREATE_TEACHER_SUBMIT) submitCreateTeacher();
        else if (id == ID_ADD_QUESTION_CHOOSE_IMAGE) chooseQuestionImage();
        else if (id == ID_ADD_QUESTION_SUBMIT) submitAddQuestion();
        else if (id == ID_IMPORT_QUESTIONS_SUBMIT) submitImportQuestions();
        else if (id == ID_IMPORT_QUESTIONS_FROM_URL) loadImportQuestionsFromUrl();
        else if (id == ID_IMPORT_QUESTIONS_FROM_FILE) loadImportQuestionsFromFile();
        else if (id == ID_IMPORT_QUESTIONS_TEMPLATE) showCsvTemplate();
        else if (id == ID_DELETE_QUESTION_SUBMIT) submitDeleteQuestion();
        else if (id == ID_CREATE_EXAM_CHOOSE_FILE) chooseExamFile();
        else if (id == ID_CREATE_EXAM_SUBMIT) submitCreateExam();
        else if (id == ID_TOGGLE_EXAM_SUBMIT) submitToggleExam();
        else if (id == ID_UPDATE_EXAM_ATTEMPT_LIMIT) submitUpdateExamAttemptLimit();
        else if (id == ID_TAKE_EXAM_OPEN_FILE) openActiveExamAttachment();
        else if (id == ID_TAKE_EXAM_SUBMIT) submitTakeExam();
        else if (id == ID_STUDENT_DELETE) submitDeleteManagedUser("student");
        else if (id == ID_STUDENT_CHANGE_CLASS) submitChangeManagedUserClass("student");
        else if (id == ID_STUDENT_SEARCH) showStudents(getText(6103));
        else if (id == ID_STUDENT_RESET_PASSWORD) submitResetManagedUserPassword("student");
        else if (id == ID_TEACHER_DELETE) submitDeleteManagedUser("teacher");
        else if (id == ID_TEACHER_CHANGE_CLASS) submitChangeManagedUserClass("teacher");
        else if (id == ID_TEACHER_SEARCH) showTeachers(getText(6203));
        else if (id == ID_TEACHER_RESET_PASSWORD) submitResetManagedUserPassword("teacher");
        else if (id == ID_DASHBOARD) showCurrentDashboard();
        else if (id == IDCANCEL) DestroyWindow(window);
    }

    void refreshAnswerChoiceButtons() {
        for (array<HWND, 4>& row : answerOptions) {
            for (HWND button : row) {
                if (button != nullptr &&
                    answerChoiceButtons.find(button) != answerChoiceButtons.end()) {
                    InvalidateRect(button, nullptr, TRUE);
                    UpdateWindow(button);
                }
            }
        }
    }

    bool selectAnswerChoice(int id) {
        HWND selectedButton = GetDlgItem(window, id);
        if (selectedButton == nullptr) {
            return false;
        }

        for (array<HWND, 4>& row : answerOptions) {
            bool foundInRow = false;
            for (HWND button : row) {
                if (button == selectedButton) {
                    foundInRow = true;
                    break;
                }
            }

            if (!foundInRow) {
                continue;
            }

            for (HWND button : row) {
                bool selected = button == selectedButton;
                answerChoiceButtons[button] = selected;
                SendMessageW(button, BM_SETCHECK, selected ? BST_CHECKED : BST_UNCHECKED, 0);
            }
            refreshAnswerChoiceButtons();
            SetFocus(selectedButton);
            return true;
        }

        return false;
    }

    wstring utf8ToWide(string text) {
        if (text.empty()) {
            return L"";
        }

        int size = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, nullptr, 0);
        if (size <= 0) {
            size = MultiByteToWideChar(CP_ACP, 0, text.c_str(), -1, nullptr, 0);
            if (size <= 0) {
                return L"";
            }
            wstring wide(size, L'\0');
            MultiByteToWideChar(CP_ACP, 0, text.c_str(), -1, wide.data(), size);
            wide.resize(size - 1);
            return wide;
        }

        wstring wide(size, L'\0');
        MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, wide.data(), size);
        wide.resize(size - 1);
        return wide;
    }

    string wideToUtf8(wstring text) {
        if (text.empty()) {
            return "";
        }

        int size = WideCharToMultiByte(CP_UTF8, 0, text.c_str(), -1, nullptr, 0, nullptr, nullptr);
        if (size <= 0) {
            return "";
        }

        string utf8(size, '\0');
        WideCharToMultiByte(CP_UTF8, 0, text.c_str(), -1, utf8.data(), size, nullptr, nullptr);
        utf8.resize(size - 1);
        return utf8;
    }

    void setControlText(HWND control, string text) {
        wstring wide = utf8ToWide(text);
        SetWindowTextW(control, wide.c_str());
    }

    bool handleShortcut(MSG& msg) {
        if (msg.message != WM_KEYDOWN) {
            return false;
        }

        bool ctrlDown = (GetKeyState(VK_CONTROL) & 0x8000) != 0;

        if (msg.wParam == VK_F1) {
            showShortcutHelp();
            return true;
        }

        if (ctrlDown && msg.wParam == 'Q') {
            DestroyWindow(window);
            return true;
        }

        if (ctrlDown && msg.wParam == 'L') {
            currentUser = nullptr;
            showMainScreen();
            return true;
        }

        if (msg.wParam == VK_ESCAPE) {
            goBackByShortcut();
            return true;
        }

        if (msg.wParam == VK_DELETE && currentScreen == SCREEN_DELETE_QUESTION) {
            submitDeleteQuestion();
            return true;
        }

        if (msg.wParam == VK_RETURN) {
            return submitDefaultByShortcut();
        }

        return false;
    }

    void clearControls() {
        stopExamTimer();
        answerOptions.clear();
        answerCombos.clear();
        activeQuestionIds.clear();
        activeExamId.clear();
        examTimerLabel = nullptr;
        primaryButtons.clear();
        answerChoiceButtons.clear();
        controlTextColors.clear();
        surfaceLabels.clear();
        for (IPreviewHandler* handler : previewHandlers) {
            if (handler != nullptr) {
                handler->Unload();
                handler->Release();
            }
        }
        previewHandlers.clear();
        previewHandlerByHost.clear();
        for (HWND control : controls) {
            DestroyWindow(control);
        }
        controls.clear();
        for (HBITMAP bitmap : imageBitmaps) {
            DeleteObject(bitmap);
        }
        imageBitmaps.clear();
        for (HIMAGELIST imageList : listImageLists) {
            ImageList_Destroy(imageList);
        }
        listImageLists.clear();
    }

    HWND addControl(string cls, string text, DWORD style, int x, int y, int w, int h, int id = 0, bool tabStop = false) {
        DWORD baseStyle = WS_CHILD | WS_VISIBLE;
        if (tabStop) {
            baseStyle |= WS_TABSTOP;
        }

        wstring wideClass = utf8ToWide(cls);
        wstring wideText = utf8ToWide(text);
        DWORD exStyle = 0;
        if (cls == "EDIT") {
            exStyle = WS_EX_CLIENTEDGE;
        }

        HWND control = CreateWindowExW(exStyle, wideClass.c_str(), wideText.c_str(),
                                      baseStyle | style,
                                      x, y, w, h, window, (HMENU)(INT_PTR)id, instance, nullptr);
        SendMessageW(control, WM_SETFONT, (WPARAM)font, TRUE);
        controls.push_back(control);
        return control;
    }

    HWND label(string text, int x, int y, int w, int h) {
        return addControl("STATIC", text, 0, x, y, w, h, 0, false);
    }

    HWND centeredLabel(string text, int x, int y, int w, int h, HFONT labelFont = nullptr,
                       COLORREF textColor = THEME_TEXT) {
        HWND control = addControl("STATIC", text, SS_CENTER | SS_CENTERIMAGE, x, y, w, h, 0, false);
        if (labelFont != nullptr) {
            SendMessageW(control, WM_SETFONT, (WPARAM)labelFont, TRUE);
        }
        controlTextColors[control] = textColor;
        return control;
    }

    HWND cardPanel(int x, int y, int w, int h) {
        HWND control = CreateWindowExW(0, L"QuizCardPanel", L"",
                                      WS_CHILD | WS_VISIBLE,
                                      x, y, w, h, window, nullptr, instance, nullptr);
        SetWindowPos(control, HWND_BOTTOM, x, y, w, h, SWP_NOACTIVATE);
        controls.push_back(control);
        return control;
    }

    HWND button(string text, int x, int y, int w, int h, int id) {
        HWND control = addControl("BUTTON", text, BS_OWNERDRAW, x, y, w, h, id, true);
        primaryButtons[control] = false;
        return control;
    }

    HWND defaultButton(string text, int x, int y, int w, int h, int id) {
        HWND control = addControl("BUTTON", text, BS_OWNERDRAW, x, y, w, h, id, true);
        primaryButtons[control] = true;
        return control;
    }

    HWND radioButton(string text, int x, int y, int w, int h, int id, bool startGroup) {
        DWORD style = BS_AUTORADIOBUTTON;
        if (startGroup) {
            style |= WS_GROUP;
        }
        return addControl("BUTTON", text, style, x, y, w, h, id, true);
    }

    HWND answerChoiceButton(string text, int x, int y, int w, int h, int id, bool startGroup) {
        DWORD style = BS_OWNERDRAW | BS_CENTER | BS_VCENTER;
        if (startGroup) {
            style |= WS_GROUP;
        }

        HWND control = addControl("BUTTON", text, style, x, y, w, h, id, true);
        SendMessageW(control, WM_SETFONT, (WPARAM)smallFont, TRUE);
        answerChoiceButtons[control] = false;
        return control;
    }

    HWND comboBox(vector<string> items, int x, int y, int w, int h, int id) {
        HWND combo = CreateWindowExW(0, L"COMBOBOX", L"",
                                    WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST,
                                    x, y, w, h, window, (HMENU)(INT_PTR)id, instance, nullptr);
        SendMessageW(combo, WM_SETFONT, (WPARAM)font, TRUE);
        for (const string& item : items) {
            wstring wideItem = utf8ToWide(item);
            SendMessageW(combo, CB_ADDSTRING, 0, (LPARAM)wideItem.c_str());
        }
        SendMessageW(combo, CB_SETCURSEL, (WPARAM)-1, 0);
        controls.push_back(combo);
        return combo;
    }

    HWND edit(string text, int x, int y, int w, int h, int id, bool password = false, bool multi = false, bool readOnly = false) {
        DWORD style = ES_AUTOHSCROLL;
        if (password) style |= ES_PASSWORD;
        if (multi) style = ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL;
        if (readOnly) style |= ES_READONLY;
        return addControl("EDIT", text, style, x, y, w, h, id, !readOnly);
    }

    HWND imageBox(string path, int x, int y, int maxW, int maxH) {
        if (path.empty()) {
            return nullptr;
        }

        wstring widePath = utf8ToWide(path);
        Image image(widePath.c_str());
        if (image.GetLastStatus() != Ok || image.GetWidth() == 0 || image.GetHeight() == 0) {
            return nullptr;
        }

        int originalW = (int)image.GetWidth();
        int originalH = (int)image.GetHeight();
        double scale = min((double)maxW / originalW, (double)maxH / originalH);
        if (scale > 1.0) {
            scale = 1.0;
        }

        int targetW = max(1, (int)(originalW * scale));
        int targetH = max(1, (int)(originalH * scale));

        Bitmap canvas(targetW, targetH, PixelFormat32bppARGB);
        Graphics graphics(&canvas);
        graphics.SetInterpolationMode(InterpolationModeHighQualityBicubic);
        graphics.Clear(Color(255, 245, 247, 250));
        graphics.DrawImage(&image, 0, 0, targetW, targetH);

        HBITMAP bitmap = nullptr;
        canvas.GetHBITMAP(Color(255, 245, 247, 250), &bitmap);
        if (bitmap == nullptr) {
            return nullptr;
        }

        HWND control = addControl("STATIC", "", SS_BITMAP | SS_CENTERIMAGE, x, y, targetW, targetH, 0, false);
        SendMessageW(control, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)bitmap);
        imageBitmaps.push_back(bitmap);
        return control;
    }

    HWND shellPreviewBox(string path, int x, int y, int maxW, int maxH) {
        if (path.empty()) {
            return nullptr;
        }

        wstring widePath = utf8ToWide(path);
        IShellItemImageFactory* imageFactory = nullptr;
        HRESULT result = SHCreateItemFromParsingName(
            widePath.c_str(),
            nullptr,
            IID_PPV_ARGS(&imageFactory)
        );
        if (FAILED(result) || imageFactory == nullptr) {
            return nullptr;
        }

        SIZE size = { maxW, maxH };
        HBITMAP bitmap = nullptr;
        result = imageFactory->GetImage(size, SIIGBF_RESIZETOFIT, &bitmap);
        imageFactory->Release();
        if (FAILED(result) || bitmap == nullptr) {
            return nullptr;
        }

        HWND control = addControl("STATIC", "", SS_BITMAP | SS_CENTERIMAGE, x, y, maxW, maxH, 0, false);
        SendMessageW(control, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)bitmap);
        imageBitmaps.push_back(bitmap);
        return control;
    }

    HWND previewHandlerBox(string path, int x, int y, int w, int h) {
        if (path.empty()) {
            return nullptr;
        }

        wstring widePath = utf8ToWide(path);
        CLSID handlerClsid;
        if (!findPreviewHandlerClsid(widePath, handlerClsid)) {
            return nullptr;
        }

        IPreviewHandler* handler = nullptr;
        HRESULT result = CoCreateInstance(
            handlerClsid,
            nullptr,
            CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER,
            IID_PPV_ARGS(&handler)
        );
        if (FAILED(result) || handler == nullptr) {
            return nullptr;
        }

        IInitializeWithFile* initializer = nullptr;
        result = handler->QueryInterface(IID_PPV_ARGS(&initializer));
        if (SUCCEEDED(result) && initializer != nullptr) {
            result = initializer->Initialize(widePath.c_str(), STGM_READ);
            initializer->Release();
        }
        if (FAILED(result)) {
            handler->Release();
            return nullptr;
        }

        HWND host = CreateWindowExW(
            WS_EX_CLIENTEDGE,
            L"QuizPreviewHostWindow",
            L"",
            WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_TABSTOP,
            x, y, w, h,
            window,
            nullptr,
            instance,
            this
        );
        if (host == nullptr) {
            handler->Release();
            return nullptr;
        }
        controls.push_back(host);
        RECT rect = { 0, 0, w, h };
        result = handler->SetWindow(host, &rect);
        if (SUCCEEDED(result)) {
            result = handler->DoPreview();
        }
        if (FAILED(result)) {
            handler->Unload();
            handler->Release();
            controls.erase(remove(controls.begin(), controls.end(), host), controls.end());
            DestroyWindow(host);
            return nullptr;
        }

        handler->SetRect(&rect);
        previewHandlers.push_back(handler);
        previewHandlerByHost[host] = handler;
        return host;
    }

    bool forwardPreviewMouseMessage(HWND host, UINT message, WPARAM wParam, LPARAM lParam) {
        HWND target = nullptr;

        if (message == WM_MOUSEWHEEL) {
            POINT screenPoint = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            target = WindowFromPoint(screenPoint);
        } else {
            POINT hostPoint = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            POINT screenPoint = hostPoint;
            ClientToScreen(host, &screenPoint);
            target = WindowFromPoint(screenPoint);
        }

        if (target == nullptr || target == host || !IsChild(host, target)) {
            target = deepestPreviewChild(host);
        }

        if (target == nullptr || target == host) {
            return false;
        }

        if (message == WM_MOUSEWHEEL) {
            SendMessageW(target, message, wParam, lParam);
            return true;
        }

        POINT targetPoint = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        ClientToScreen(host, &targetPoint);
        ScreenToClient(target, &targetPoint);
        LPARAM targetLParam = MAKELPARAM(targetPoint.x, targetPoint.y);
        SendMessageW(target, message, wParam, targetLParam);
        return true;
    }

    HWND deepestPreviewChild(HWND host) {
        HWND child = GetWindow(host, GW_CHILD);
        if (child == nullptr) {
            return nullptr;
        }

        HWND current = child;
        while (true) {
            HWND nested = GetWindow(current, GW_CHILD);
            if (nested == nullptr) {
                return current;
            }
            current = nested;
        }
    }

    HWND previewHostFromPoint(LPARAM lParam) {
        POINT point = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        for (const auto& item : previewHandlerByHost) {
            HWND host = item.first;
            RECT rect = {};
            if (IsWindow(host) && GetWindowRect(host, &rect) && PtInRect(&rect, point)) {
                return host;
            }
        }

        return nullptr;
    }

    void focusPreviewHandler(HWND host) {
        auto iterator = previewHandlerByHost.find(host);
        if (iterator != previewHandlerByHost.end() && iterator->second != nullptr) {
            iterator->second->SetFocus();
            return;
        }

        HWND previewChild = GetWindow(host, GW_CHILD);
        if (previewChild != nullptr) {
            SetFocus(previewChild);
        }
    }

    bool translatePreviewMessage(MSG& msg) {
        if (previewHandlerByHost.empty()) {
            return false;
        }

        if (msg.message != WM_KEYDOWN && msg.message != WM_KEYUP) {
            return false;
        }

        HWND previewHost = nullptr;
        if (msg.hwnd != nullptr) {
            previewHost = previewHostFromWindow(msg.hwnd);
        }

        if (previewHost == nullptr) {
            return false;
        }

        auto iterator = previewHandlerByHost.find(previewHost);
        if (iterator == previewHandlerByHost.end() || iterator->second == nullptr) {
            return false;
        }

        if (msg.message == WM_KEYDOWN) {
            focusPreviewHandler(previewHost);
        }

        return iterator->second->TranslateAccelerator(&msg) == S_OK;
    }

    HWND previewHostFromWindow(HWND hwnd) {
        HWND current = hwnd;
        while (current != nullptr) {
            if (previewHandlerByHost.find(current) != previewHandlerByHost.end()) {
                return current;
            }
            current = GetParent(current);
        }

        return nullptr;
    }

    bool findPreviewHandlerClsid(const wstring& filePath, CLSID& clsid) {
        wstring extension = fileExtension(filePath);
        if (extension.empty()) {
            return false;
        }

        const wstring previewHandlerKey = L"\\shellex\\{8895b1c6-b41f-4c1c-a562-0d564250836f}";
        vector<wstring> candidateKeys;
        candidateKeys.push_back(extension + previewHandlerKey);

        wstring progId;
        if (readRegistryDefault(HKEY_CLASSES_ROOT, extension, progId) && !progId.empty()) {
            candidateKeys.push_back(progId + previewHandlerKey);
        }

        candidateKeys.push_back(L"SystemFileAssociations\\" + extension + previewHandlerKey);

        for (const wstring& key : candidateKeys) {
            wstring clsidText;
            if (readRegistryDefault(HKEY_CLASSES_ROOT, key, clsidText) && !clsidText.empty()) {
                if (SUCCEEDED(CLSIDFromString(clsidText.c_str(), &clsid))) {
                    return true;
                }
            }
        }

        return false;
    }

    wstring fileExtension(const wstring& path) {
        size_t slash = path.find_last_of(L"\\/");
        size_t dot = path.find_last_of(L'.');
        if (dot == wstring::npos || (slash != wstring::npos && dot < slash)) {
            return L"";
        }

        wstring extension = path.substr(dot);
        transform(extension.begin(), extension.end(), extension.begin(),
                  [](wchar_t ch) { return (wchar_t)towlower(ch); });
        return extension;
    }

    bool readRegistryDefault(HKEY root, const wstring& subKey, wstring& value) {
        HKEY key = nullptr;
        if (RegOpenKeyExW(root, subKey.c_str(), 0, KEY_READ, &key) != ERROR_SUCCESS) {
            return false;
        }

        DWORD type = 0;
        DWORD size = 0;
        LONG result = RegQueryValueExW(key, nullptr, nullptr, &type, nullptr, &size);
        if (result != ERROR_SUCCESS || (type != REG_SZ && type != REG_EXPAND_SZ) || size == 0) {
            RegCloseKey(key);
            return false;
        }

        vector<wchar_t> buffer(size / sizeof(wchar_t) + 1, L'\0');
        result = RegQueryValueExW(key, nullptr, nullptr, &type, (LPBYTE)buffer.data(), &size);
        RegCloseKey(key);
        if (result != ERROR_SUCCESS) {
            return false;
        }

        value.assign(buffer.data());
        return !value.empty();
    }

    HWND examListView(int x, int y, int w, int h, bool onlyOpen) {
        w = max(w, 1040);
        HWND list = CreateWindowExW(WS_EX_CLIENTEDGE, WC_LISTVIEWW, L"",
                                   WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
                                   x, y, w, h, window, nullptr, instance, nullptr);
        styleModernListView(list, 34);
        controls.push_back(list);

        addListColumn(list, 0, "ID", 58);
        addListColumn(list, 1, "Tên đề", 180);
        addListColumn(list, 2, "Làm bài", 92);
        addListColumn(list, 3, "Mở lúc", 150);
        addListColumn(list, 4, "Đóng lúc", 150);
        addListColumn(list, 5, "Số câu", 76);
        addListColumn(list, 6, "Lượt", 64);
        addListColumn(list, 7, "Giáo viên", 130);
        addListColumn(list, 8, "MK", 52);
        addListColumn(list, 9, "File", 56);
        addListColumn(list, 10, "Trạng thái", 112);

        int row = 0;
        for (Exam& exam : data.getExams()) {
            string now = QuizData::currentMinuteText();
            if (onlyOpen && !exam.canTake(now)) {
                continue;
            }

            string status = exam.statusText(now);
            vector<string> values = {
                exam.getExamId(),
                exam.getTitle(),
                to_string(exam.getDurationMinutes()) + " phut",
                exam.getStartAt().empty() ? "-" : exam.getStartAt(),
                exam.getCloseAt(),
                to_string(exam.totalQuestions()),
                to_string(exam.getAttemptLimit()),
                data.getUserNameById(exam.getCreatedBy()),
                exam.hasPassword() ? "Có" : "Không",
                exam.hasAttachment() ? "Có" : "Không",
                status
            };

            wstring wideValue = utf8ToWide(values[0]);
            LVITEMW item = {};
            item.mask = LVIF_TEXT;
            item.iItem = row;
            item.iSubItem = 0;
            item.pszText = const_cast<wchar_t*>(wideValue.c_str());
            SendMessageW(list, LVM_INSERTITEMW, 0, (LPARAM)&item);

            for (int col = 1; col < (int)values.size(); col++) {
                setListText(list, row, col, values[col]);
            }
            row++;
        }

        return list;
    }

    HWND userListView(int x, int y, int w, int h, string role, string keyword = "") {
        w = max(w, 920);
        h = max(h, 360);
        HWND list = CreateWindowExW(WS_EX_CLIENTEDGE, WC_LISTVIEWW, L"",
                                   WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
                                   x, y, w, h, window, nullptr, instance, nullptr);
        styleModernListView(list, 34);
        controls.push_back(list);

        addListColumn(list, 0, "ID", 125);
        addListColumn(list, 1, "Họ tên", 220);
        addListColumn(list, 2, "Email", 320);
        addListColumn(list, 3, role == "student" ? "Lớp" : "Lớp/môn phụ trách", 180);

        string normalizedKeyword = lowerText(keyword);
        vector<shared_ptr<User>> rows;
        for (shared_ptr<User> user : data.getUsers()) {
            string searchable = lowerText(user->getUserId() + " " + user->getName() + " " +
                                          user->getEmail() + " " + user->getClassName());
            if (user->getRole() == role &&
                (normalizedKeyword.empty() || searchable.find(normalizedKeyword) != string::npos)) {
                rows.push_back(user);
            }
        }

        sort(rows.begin(), rows.end(), [](shared_ptr<User> leftUser, shared_ptr<User> rightUser) {
            if (leftUser->getClassName() != rightUser->getClassName()) {
                return leftUser->getClassName() < rightUser->getClassName();
            }
            return leftUser->getName() < rightUser->getName();
        });

        for (int row = 0; row < (int)rows.size(); row++) {
            vector<string> values = {
                rows[row]->getUserId(),
                rows[row]->getName(),
                rows[row]->getEmail(),
                rows[row]->getClassName(),
            };

            wstring wideValue = utf8ToWide(values[0]);
            LVITEMW item = {};
            item.mask = LVIF_TEXT;
            item.iItem = row;
            item.iSubItem = 0;
            item.pszText = const_cast<wchar_t*>(wideValue.c_str());
            SendMessageW(list, LVM_INSERTITEMW, 0, (LPARAM)&item);

            for (int col = 1; col < (int)values.size(); col++) {
                setListText(list, row, col, values[col]);
            }
        }

        return list;
    }

    void addListColumn(HWND list, int index, string title, int width, int format = LVCFMT_LEFT) {
        wstring wideTitle = utf8ToWide(title);
        LVCOLUMNW column = {};
        column.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM | LVCF_FMT;
        column.pszText = const_cast<wchar_t*>(wideTitle.c_str());
        column.cx = width;
        column.iSubItem = index;
        column.fmt = format;
        SendMessageW(list, LVM_INSERTCOLUMNW, index, (LPARAM)&column);
    }

    void setListText(HWND list, int row, int col, string text) {
        wstring wideText = utf8ToWide(text);
        LVITEMW item = {};
        item.iSubItem = col;
        item.pszText = const_cast<wchar_t*>(wideText.c_str());
        SendMessageW(list, LVM_SETITEMTEXTW, row, (LPARAM)&item);
    }

    void styleModernListView(HWND list, int rowHeight) {
        SendMessageW(list, WM_SETFONT, (WPARAM)font, TRUE);
        ListView_SetExtendedListViewStyle(
            list,
            LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER | LVS_EX_INFOTIP | LVS_EX_GRIDLINES
        );
        ListView_SetBkColor(list, THEME_SURFACE);
        ListView_SetTextBkColor(list, THEME_SURFACE);
        ListView_SetTextColor(list, THEME_TEXT);

        HIMAGELIST imageList = ImageList_Create(1, rowHeight, ILC_COLOR32, 1, 1);
        if (imageList != nullptr) {
            ListView_SetImageList(list, imageList, LVSIL_SMALL);
            listImageLists.push_back(imageList);
        }

        HWND header = ListView_GetHeader(list);
        if (header != nullptr) {
            SendMessageW(header, WM_SETFONT, (WPARAM)smallFont, TRUE);
        }
    }

    string getText(int id) {
        HWND control = GetDlgItem(window, id);
        if (control == nullptr) {
            return "";
        }
        int len = GetWindowTextLengthW(control);
        wstring text(len + 1, L'\0');
        GetWindowTextW(control, text.data(), len + 1);
        text.resize(len);
        return wideToUtf8(text);
    }

    void setFocusTo(int id) {
        HWND control = GetDlgItem(window, id);
        if (control != nullptr) {
            SetFocus(control);
        }
    }

    void message(string text, string title = "Quiz App") {
        MessageBoxW(window, utf8ToWide(text).c_str(), utf8ToWide(title).c_str(), MB_OK);
    }

    void error(string text) {
        MessageBoxW(window, utf8ToWide(text).c_str(), L"Thông báo", MB_OK | MB_SETFOREGROUND);
    }

    void showShortcutHelp() {
        string text =
            "Phím tắt trong app:\r\n\r\n"
            "Tab: Chuyển sang ô/nút tiếp theo\r\n"
            "Shift + Tab: Quay lại ô/nút trước\r\n"
            "Enter: Thực hiện nút chính của màn hình hiện tại\r\n"
            "Esc: Quay lại dashboard hoặc màn hình chính\r\n"
            "Delete: Xóa câu hỏi khi đang ở màn hình Xóa câu hỏi\r\n"
            "Ctrl + L: Đăng xuất nhanh\r\n"
            "Ctrl + Q: Thoát app\r\n"
            "F1: Xem bảng phím tắt này";
        MessageBoxW(window, utf8ToWide(text).c_str(), L"Phím tắt", MB_OK | MB_ICONINFORMATION);
    }

    void goBackByShortcut() {
        if (currentScreen == SCREEN_MAIN) {
            DestroyWindow(window);
        } else if (currentUser == nullptr ||
                   currentScreen == SCREEN_LOGIN ||
                   currentScreen == SCREEN_REGISTER) {
            showMainScreen();
        } else {
            showCurrentDashboard();
        }
    }

    bool submitDefaultByShortcut() {
        switch (currentScreen) {
            case SCREEN_LOGIN:
                submitLogin();
                return true;
            case SCREEN_REGISTER:
                submitRegisterStudent();
                return true;
            case SCREEN_PROFILE:
                submitProfileUpdate();
                return true;
            case SCREEN_CREATE_TEACHER:
                submitCreateTeacher();
                return true;
            case SCREEN_ADD_QUESTION:
                submitAddQuestion();
                return true;
            case SCREEN_IMPORT_QUESTIONS:
                submitImportQuestions();
                return true;
            case SCREEN_DELETE_QUESTION:
                submitDeleteQuestion();
                return true;
            case SCREEN_CREATE_EXAM:
                submitCreateExam();
                return true;
            case SCREEN_TOGGLE_EXAM:
                submitToggleExam();
                return true;
            case SCREEN_TAKE_EXAM_SELECT:
            case SCREEN_TAKE_EXAM_FORM:
                submitTakeExam();
                return true;
            default:
                return false;
        }
    }

    void title(string text, string subtitle = "") {
        RedrawWindow(
            window,
            nullptr,
            nullptr,
            RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN | RDW_UPDATENOW
        );

        HWND brand = label("QUIZ APP", 24, 22, 190, 30);
        SendMessageW(brand, WM_SETFONT, (WPARAM)brandFont, TRUE);
        HWND titleLabel = label(text, 280, 24, 820, 38);
        SendMessageW(titleLabel, WM_SETFONT, (WPARAM)titleFont, TRUE);
        if (!subtitle.empty()) {
            HWND subtitleLabel = label(subtitle, 282, 66, 820, 24);
            SendMessageW(subtitleLabel, WM_SETFONT, (WPARAM)smallFont, TRUE);
        }
    }

    void showMainScreen() {
        showLoginScreen();
    }

    void showSampleAccounts() {
        clearControls();
        title("Tài khoản mẫu", "Dùng để kiểm tra nhanh dữ liệu trong bộ nhớ.");
        string text =
            "Admin:   admin@quiz.local / 123456\r\n"
            "Teacher: teacher@quiz.local / 123456\r\n"
            "Student: an@quiz.local / 123456\r\n\r\n"
            "Học sinh được đăng ký ở màn hình chính.\r\n"
            "Admin có sẵn trong hệ thống.\r\n"
            "Giáo viên do admin tạo.";
        edit(text, 280, 120, 520, 220, 0, false, true, true);
        button("Quay lại", 280, 370, 150, 40, ID_BACK_MAIN);
    }

    void showLoginScreen() {
        clearControls();
        currentScreen = SCREEN_LOGIN;
        InvalidateRect(window, nullptr, TRUE);
        UpdateWindow(window);

        RECT client = {};
        GetClientRect(window, &client);
        int centerX = (int)(client.right - client.left) / 2;
        int cardX = centerX - 270;
        int fieldX = cardX + 35;

        string logoPath = appDirectory() + "\\hcmute-logo.png";
        imageBox(logoPath, centerX - 55, 18, 110, 140);

        centeredLabel(
            "TRƯỜNG ĐẠI HỌC CÔNG NGHỆ KỸ THUẬT TP.HCM",
            centerX - 330, 164, 660, 54, brandFont, RGB(8, 47, 89)
        );

        HWND formTitle = label("ĐĂNG NHẬP", fieldX, 270, 470, 42);
        SendMessageW(formTitle, WM_SETFONT, (WPARAM)titleFont, TRUE);
        controlTextColors[formTitle] = RGB(30, 83, 148);
        surfaceLabels[formTitle] = true;

        HWND subtitle = label("Hệ thống quản lý và thi trắc nghiệm", fieldX + 2, 316, 470, 26);
        SendMessageW(subtitle, WM_SETFONT, (WPARAM)smallFont, TRUE);
        controlTextColors[subtitle] = THEME_MUTED;
        surfaceLabels[subtitle] = true;

        HWND emailLabel = label("Tài khoản hoặc email", fieldX + 2, 363, 470, 24);
        SendMessageW(emailLabel, WM_SETFONT, (WPARAM)smallFont, TRUE);
        controlTextColors[emailLabel] = THEME_MUTED;
        surfaceLabels[emailLabel] = true;
        edit("", fieldX, 390, 470, 48, 2001);

        HWND passwordLabel = label("Mật khẩu", fieldX + 2, 453, 470, 24);
        SendMessageW(passwordLabel, WM_SETFONT, (WPARAM)smallFont, TRUE);
        controlTextColors[passwordLabel] = THEME_MUTED;
        surfaceLabels[passwordLabel] = true;
        edit("", fieldX, 480, 470, 48, 2002, true);

        defaultButton("Đăng nhập", fieldX, 554, 470, 48, ID_LOGIN_SUBMIT);
        button("Đăng ký tài khoản học sinh", fieldX, 618, 470, 46, ID_MAIN_REGISTER);

        HWND passwordHelp = centeredLabel(
            "Quên mật khẩu? Vui lòng liên hệ giáo viên hoặc quản trị viên.",
            fieldX, 684, 470, 26, smallFont, THEME_MUTED
        );
        surfaceLabels[passwordHelp] = true;

        centeredLabel(
            "HCMUTE Quiz App • Học tập, kiểm tra và quản lý lớp học",
            centerX - 330, 765, 660, 26, smallFont, THEME_MUTED
        );

        RedrawWindow(
            window,
            nullptr,
            nullptr,
            RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN | RDW_UPDATENOW
        );
        setFocusTo(2001);
    }

    void showRegisterScreen() {
        clearControls();
        currentScreen = SCREEN_REGISTER;
        title("Đăng ký học sinh", "Chỉ học sinh được tự đăng ký.");
        label("Họ tên", 300, 130, 130, 26);
        edit("", 450, 126, 280, 30, 2101);
        label("Email", 300, 175, 130, 26);
        edit("", 450, 171, 280, 30, 2102);
        label("Mật khẩu", 300, 220, 130, 26);
        edit("", 450, 216, 280, 30, 2103, true);
        label("Lớp", 300, 265, 130, 26);
        edit("", 450, 261, 280, 30, 2104);
        defaultButton("Đăng ký", 450, 320, 125, 38, ID_REGISTER_SUBMIT);
        button("Quay lại", 585, 320, 125, 38, ID_BACK_MAIN);
        setFocusTo(2101);
    }

    void submitLogin() {
        string email = getText(2001);
        string password = getText(2002);
        shared_ptr<User> user = data.findUserByEmail(email);
        if (user == nullptr || !user->checkPassword(password)) {
            error("Sai email hoặc mật khẩu.");
            return;
        }

        currentUser = user;
        if (user->getRole() == "admin") showAdminDashboard();
        else if (user->getRole() == "teacher") showTeacherDashboard();
        else showStudentDashboard();
    }

    void submitRegisterStudent() {
        string name = getText(2101);
        string email = getText(2102);
        string password = getText(2103);
        string className = normalizeClassName(getText(2104));

        if (name.empty() || email.empty() || password.empty() || className.empty()) {
            error("Vui lòng nhập đầy đủ thông tin.");
            return;
        }
        if (containsRoleKeyword(name) || containsRoleKeyword(password)) {
            error("Tên và mật khẩu không được chứa từ khóa: admin, teacher, student.");
            return;
        }

        if (data.findUserByEmail(email) != nullptr) {
            error("Email này đã tồn tại.");
            return;
        }

        if (!data.addStudent(name, email, password, className)) {
            error("Đăng ký thất bại. Không thể lưu tài khoản.\r\n"
                  "Vui lòng kiểm tra kết nối Internet rồi thử lại.");
            return;
        }

        message("Đăng ký thành công. Bạn có thể đăng nhập.");
        showLoginScreen();
    }

    void addNav(string roleTitle) {
        HWND role = label(roleTitle, 24, 68, 190, 28);
        SendMessageW(role, WM_SETFONT, (WPARAM)brandFont, TRUE);
        button("Hướng dẫn", 24, 590, 190, 40, ID_HELP);
        button("Thông tin", 24, 645, 190, 40, ID_PROFILE);
        button("Đăng xuất", 24, 700, 190, 40, ID_LOGOUT);
    }

    void showUserGuide() {
        clearControls();
        currentScreen = SCREEN_VIEW;

        if (currentUser == nullptr) {
            showMainScreen();
            return;
        }

        string roleTitle = "Học sinh";
        if (currentUser->getRole() == "admin") roleTitle = "Quản trị viên";
        else if (currentUser->getRole() == "teacher") roleTitle = "Giáo viên";

        title("Hướng dẫn sử dụng", "Hướng dẫn nhanh cho " + roleTitle + ".");
        addNav(roleTitle);

        edit(guideTextForRole(currentUser->getRole()), 260, 95, 660, 430, 0, false, true, true);
    }

    string guideTextForRole(string role) {
        if (role == "admin") {
            return
                "HƯỚNG DẪN CHO QUẢN TRỊ VIÊN\r\n\r\n"
                "1. Quản lý học sinh\r\n"
                "- Chọn Học sinh để xem danh sách học sinh theo bảng.\r\n"
                "- Nhập ID học sinh và Lớp mới, sau đó bấm Chuyển lớp.\r\n"
                "- Nhập ID học sinh và Mật khẩu mới để đặt lại mật khẩu.\r\n"
                "- Nhập ID học sinh và bấm Xóa học sinh để xóa tài khoản.\r\n\r\n"
                "2. Quản lý giáo viên\r\n"
                "- Chọn Giáo viên để xem danh sách giáo viên.\r\n"
                "- Chọn Tạo giáo viên để tạo tài khoản mới.\r\n"
                "- Nhập ID và Phụ trách mới để đổi lớp/môn phụ trách.\r\n"
                "- Nhập ID và Mật khẩu mới để đặt lại mật khẩu.\r\n\r\n"
                "3. Quản lý đề thi và kết quả\r\n"
                "- Chọn Đề thi để xem danh sách đề.\r\n"
                "- Chọn Mở/đóng đề để thay đổi trạng thái đề thi.\r\n"
                "- Chọn Kết quả để xem điểm của tất cả học sinh.\r\n\r\n"
                "4. Thông tin cá nhân\r\n"
                "- Chọn Thông tin để đổi tên, email hoặc mật khẩu.\r\n"
                "- Sau khi lưu, app sẽ đăng xuất và yêu cầu đăng nhập lại.";
        }

        if (role == "teacher") {
            return
                "HƯỚNG DẪN CHO GIÁO VIÊN\r\n\r\n"
                "1. Quản lý câu hỏi\r\n"
                "- Chọn Câu hỏi để xem ngân hàng câu hỏi.\r\n"
                "- Chọn Thêm câu hỏi để thêm từng câu hỏi.\r\n"
                "- Chọn Nhập câu hỏi để nhập nhiều câu hỏi từ CSV.\r\n"
                "- Khi nhập CSV, chỉ cần nhập Môn học/Chủ đề một lần ở bên ngoài.\r\n"
                "- File CSV cần các cột: content, optionA, optionB, optionC, optionD, correctAnswer.\r\n"
                "- Có thể chọn file từ máy, tải URL hoặc mở Mẫu CSV.\r\n\r\n"
                "2. Tạo đề thi\r\n"
                "- Chọn Tải đề thi.\r\n"
                "- Nhập tên đề, thời gian làm bài, thời gian mở và đóng đề.\r\n"
                "- Với ngân hàng câu hỏi, nhập mã câu hỏi, ví dụ: q1 q2 q3.\r\n"
                "- Với file gốc, để trống mã câu hỏi và nhập đáp án, ví dụ: A C B B A.\r\n"
                "- Chọn file .docx/.pdf/.jpg/.png hoặc dán liên kết công khai.\r\n"
                "- Có thể bỏ trống mật khẩu nếu đề không cần bảo vệ.\r\n\r\n"
                "3. Theo dõi đề và kết quả\r\n"
                "- Chọn Đề thi để xem danh sách đề.\r\n"
                "- Chọn Mở/đóng đề để thay đổi trạng thái.\r\n"
                "- Chọn Kết quả để xem điểm học sinh.\r\n\r\n"
                "4. Thông tin cá nhân\r\n"
                "- Chọn Thông tin để đổi tên, email hoặc mật khẩu.\r\n"
                "- Sau khi lưu, app sẽ đăng xuất và yêu cầu đăng nhập lại.";
        }

        return
            "HƯỚNG DẪN CHO HỌC SINH\r\n\r\n"
            "1. Đăng ký và đăng nhập\r\n"
            "- Học sinh có thể tự đăng ký tại màn hình đăng nhập.\r\n"
            "- Sau khi đăng ký thành công, đăng nhập bằng email và mật khẩu đã tạo.\r\n\r\n"
            "2. Xem và làm bài thi\r\n"
            "- Chọn Đề đang mở để xem các đề có thể làm.\r\n"
            "- Chọn Làm bài, nhập mã đề và mật khẩu nếu có.\r\n"
            "- Chỉ có thể bắt đầu khi đã tới thời gian mở đề.\r\n"
            "- Khi vào bài, chọn đáp án A/B/C/D bằng chuột.\r\n"
            "- Nếu đề có file gốc, mở file để xem nội dung rồi chọn đáp án trong app.\r\n"
            "- App hiển thị thời gian còn lại và tự động nộp bài khi hết giờ.\r\n\r\n"
            "3. Xem kết quả\r\n"
            "- Chọn Lịch sử điểm để xem điểm các bài đã nộp.\r\n\r\n"
            "4. Thông tin cá nhân\r\n"
            "- Chọn Thông tin để đổi tên, email hoặc mật khẩu.\r\n"
            "- Sau khi lưu, app sẽ đăng xuất và yêu cầu đăng nhập lại.";
    }

    void showProfileScreen() {
        if (currentUser == nullptr) {
            showMainScreen();
            return;
        }

        clearControls();
        currentScreen = SCREEN_PROFILE;
        title("Thông tin cá nhân", "Sau khi cập nhật, bạn cần đăng nhập lại.");

        string roleTitle = "Học sinh";
        if (currentUser->getRole() == "admin") roleTitle = "Quản trị viên";
        else if (currentUser->getRole() == "teacher") roleTitle = "Giáo viên";
        addNav(roleTitle);

        label("Họ tên", 300, 130, 170, 26);
        edit(currentUser->getName(), 500, 126, 360, 34, 7001);
        label("Email/Gmail", 300, 180, 170, 26);
        edit(currentUser->getEmail(), 500, 176, 360, 34, 7002);
        label("Mật khẩu hiện tại", 300, 230, 170, 26);
        edit("", 500, 226, 360, 34, 7003, true);
        label("Mật khẩu mới", 300, 280, 170, 26);
        edit("", 500, 276, 360, 34, 7004, true);
        label("Xác nhận mật khẩu mới", 300, 330, 190, 26);
        edit("", 500, 326, 360, 34, 7005, true);
        label("Lớp/phụ trách", 300, 380, 170, 26);
        edit(currentUser->getClassName(), 500, 376, 360, 34, 0, false, false, true);

        defaultButton("Lưu thay đổi", 500, 445, 170, 42, ID_PROFILE_SAVE);
        button("Về bảng điều khiển", 690, 445, 170, 42, ID_DASHBOARD);
        setFocusTo(7001);
    }

    void submitProfileUpdate() {
        if (currentUser == nullptr) {
            showMainScreen();
            return;
        }

        string userId = currentUser->getUserId();
        string name = getText(7001);
        string email = getText(7002);
        string currentPassword = getText(7003);
        string newPassword = getText(7004);
        string confirmPassword = getText(7005);

        if (name.empty() || email.empty() || currentPassword.empty() ||
            newPassword.empty() || confirmPassword.empty()) {
            error("Vui lòng nhập đầy đủ họ tên, email và các ô mật khẩu.");
            return;
        }
        if (email.find('@') == string::npos || email.find('.') == string::npos) {
            error("Email/Gmail không hợp lệ.");
            return;
        }
        if (!currentUser->checkPassword(currentPassword)) {
            error("Mật khẩu hiện tại không đúng.");
            return;
        }
        if (newPassword != confirmPassword) {
            error("Hai lần nhập mật khẩu mới không trùng nhau.");
            return;
        }
        if (newPassword.size() < 6) {
            error("Mật khẩu mới phải có ít nhất 6 ký tự.");
            return;
        }
        if (containsRoleKeyword(name) || containsRoleKeyword(newPassword)) {
            error("Tên và mật khẩu không được chứa từ khóa: admin, teacher, student.");
            return;
        }

        shared_ptr<User> existingUser = data.findUserByEmail(email);
        if (existingUser != nullptr && existingUser->getUserId() != userId) {
            error("Email này đã được tài khoản khác sử dụng.");
            return;
        }

        if (!data.updateUserProfile(userId, name, email, newPassword)) {
            error("Cập nhật thông tin thất bại. Vui lòng thử lại.");
            return;
        }

        currentUser = nullptr;
        message("Đã cập nhật thông tin. Vui lòng đăng nhập lại bằng thông tin mới.");
        showLoginScreen();
    }

    void showAdminDashboard() {
        clearControls();
        currentScreen = SCREEN_ADMIN_DASHBOARD;
        title("Bảng điều khiển Admin", "Xin chào " + currentUser->getName());
        addNav("Quản trị viên");
        button("Học sinh", 24, 120, 190, 40, ID_ADMIN_STUDENTS);
        button("Giáo viên", 24, 170, 190, 40, ID_ADMIN_TEACHERS);
        button("Tạo giáo viên", 24, 220, 190, 40, ID_ADMIN_CREATE_TEACHER);
        button("Câu hỏi", 24, 270, 190, 40, ID_ADMIN_QUESTIONS);
        button("Đề thi", 24, 320, 190, 40, ID_ADMIN_EXAMS);
        button("Mở/đóng đề", 24, 370, 190, 40, ID_ADMIN_TOGGLE_EXAM);
        button("Kết quả", 24, 420, 190, 40, ID_ADMIN_RESULTS);

        HWND heading = label("Quản trị hệ thống", 280, 125, 360, 32);
        SendMessageW(heading, WM_SETFONT, (WPARAM)brandFont, TRUE);
        label("Theo dõi tài khoản, đề thi và kết quả trên toàn bộ lớp học.", 280, 168, 650, 26);
        defaultButton("Quản lý học sinh", 280, 225, 200, 48, ID_ADMIN_STUDENTS);
        button("Quản lý giáo viên", 500, 225, 200, 48, ID_ADMIN_TEACHERS);
        button("Xem kết quả", 720, 225, 170, 48, ID_ADMIN_RESULTS);

        HWND statusTitle = label("Tổng quan hệ thống", 280, 330, 260, 30);
        SendMessageW(statusTitle, WM_SETFONT, (WPARAM)brandFont, TRUE);
        edit("Quản lý học sinh, giáo viên, ngân hàng câu hỏi, đề thi và kết quả tại một nơi.",
             280, 375, 660, 90, 0, false, true, true);
    }

    void showTeacherDashboard() {
        clearControls();
        currentScreen = SCREEN_TEACHER_DASHBOARD;
        title("Bảng điều khiển Giáo viên", "Xin chào " + currentUser->getName());
        addNav("Giáo viên");
        button("Câu hỏi", 24, 120, 190, 40, ID_TEACHER_QUESTIONS);
        button("Thêm câu hỏi", 24, 170, 190, 40, ID_TEACHER_ADD_QUESTION);
        button("Nhập câu hỏi", 24, 220, 190, 40, ID_TEACHER_IMPORT_QUESTIONS);
        button("Xóa câu hỏi", 24, 270, 190, 40, ID_TEACHER_DELETE_QUESTION);
        button("Đề thi", 24, 320, 190, 40, ID_TEACHER_EXAMS);
        button("Up đề thi", 24, 370, 190, 40, ID_TEACHER_CREATE_EXAM);
        button("Mở/đóng đề", 24, 420, 190, 40, ID_TEACHER_TOGGLE_EXAM);
        button("Kết quả", 24, 470, 190, 40, ID_TEACHER_RESULTS);

        HWND heading = label("Tạo và theo dõi bài thi", 280, 125, 420, 32);
        SendMessageW(heading, WM_SETFONT, (WPARAM)brandFont, TRUE);
        label("Chuẩn bị ngân hàng câu hỏi, up đề file gốc và xem kết quả học sinh.", 280, 168, 760, 26);
        defaultButton("Up đề thi", 280, 225, 170, 48, ID_TEACHER_CREATE_EXAM);
        button("Nhập câu hỏi", 470, 225, 170, 48, ID_TEACHER_IMPORT_QUESTIONS);
        button("Xem kết quả", 660, 225, 170, 48, ID_TEACHER_RESULTS);

        HWND statusTitle = label("Gợi ý quy trình", 280, 330, 260, 30);
        SendMessageW(statusTitle, WM_SETFONT, (WPARAM)brandFont, TRUE);
        edit("1. Nhập câu hỏi hoặc chọn file đề gốc.\r\n"
             "2. Tạo đề thi, đặt giờ mở/đóng và số lượt làm.\r\n"
             "3. Học sinh làm bài, giáo viên xem kết quả.",
             280, 375, 660, 140, 0, false, true, true);
    }

    void showStudentDashboard() {
        clearControls();
        currentScreen = SCREEN_STUDENT_DASHBOARD;
        title("Bảng điều khiển Học sinh", "Xin chào " + currentUser->getName());
        addNav("Học sinh");
        button("Đề đang mở", 24, 120, 190, 40, ID_STUDENT_OPEN_EXAMS);
        button("Làm bài", 24, 170, 190, 40, ID_STUDENT_TAKE_EXAM);
        button("Lịch sử điểm", 24, 220, 190, 40, ID_STUDENT_RESULTS);

        HWND heading = label("Sẵn sàng làm bài", 280, 125, 360, 32);
        SendMessageW(heading, WM_SETFONT, (WPARAM)brandFont, TRUE);
        label("Xem đề đang mở, nhập mã đề và làm bài theo thời gian giáo viên đặt.", 280, 168, 760, 26);
        defaultButton("Làm bài ngay", 280, 225, 170, 48, ID_STUDENT_TAKE_EXAM);
        button("Xem đề mở", 470, 225, 170, 48, ID_STUDENT_OPEN_EXAMS);
        button("Xem điểm", 660, 225, 170, 48, ID_STUDENT_RESULTS);

        HWND statusTitle = label("Lưu ý khi làm bài", 280, 330, 260, 30);
        SendMessageW(statusTitle, WM_SETFONT, (WPARAM)brandFont, TRUE);
        edit("• Chỉ bắt đầu được khi đã tới giờ mở đề.\r\n"
             "• Nếu đề có mật khẩu, nhập mật khẩu giáo viên cung cấp.\r\n"
             "• Khi hết giờ, hệ thống tự nộp bài.",
             280, 375, 660, 140, 0, false, true, true);
    }

    string usersText(string role) {
        stringstream ss;
        if (role == "student") {
            map<string, vector<shared_ptr<User>>> studentsByClass;
            for (shared_ptr<User> user : data.getUsers()) {
                if (user->getRole() == "student") {
                    studentsByClass[user->getClassName()].push_back(user);
                }
            }

            for (const auto& group : studentsByClass) {
                ss << "LOP: " << group.first << "\r\n";
                ss << "ID     Họ tên                 Email\r\n";
                ss << "------------------------------------------------------------\r\n";
                for (shared_ptr<User> user : group.second) {
                    ss << left << setw(7) << user->getUserId()
                       << setw(22) << user->getName()
                       << user->getEmail() << "\r\n";
                }
                ss << "\r\n";
            }

            return ss.str();
        }

        ss << "ID     Vai trò    Họ tên                 Email                       Lớp/Phụ trách\r\n";
        ss << "--------------------------------------------------------------------------------\r\n";
        for (shared_ptr<User> user : data.getUsers()) {
            if (user->getRole() == role) {
                ss << left << setw(7) << user->getUserId()
                   << setw(10) << user->getRole()
                   << setw(22) << user->getName()
                   << setw(28) << user->getEmail()
                   << user->getClassName() << "\r\n";
            }
        }
        return ss.str();
    }

    void showStudents(string keyword = "") {
        clearControls();
        currentScreen = SCREEN_VIEW;
        data.loadUsersFromFirebase();
        title("Quản lý học sinh");
        addNav("Quản trị viên");
        label("Tìm kiếm", 250, 88, 90, 26);
        edit(keyword, 350, 84, 260, 30, 6103);
        button("Tim", 625, 84, 80, 32, ID_STUDENT_SEARCH);
        userListView(250, 128, 760, 315, "student", keyword);
        label("ID học sinh", 270, 455, 110, 26);
        edit("", 390, 451, 170, 30, 6101);
        label("Lớp mới", 580, 455, 80, 26);
        edit("", 665, 451, 130, 30, 6102);
        button("Chuyển lớp", 390, 500, 130, 38, ID_STUDENT_CHANGE_CLASS);
        button("Xóa học sinh", 535, 500, 130, 38, ID_STUDENT_DELETE);
        button("Về dashboard", 680, 500, 140, 38, ID_DASHBOARD);
        label("Mật khẩu mới", 270, 555, 115, 26);
        edit("", 390, 551, 170, 30, 6104, true);
        button("Đặt lại MK", 580, 548, 130, 38, ID_STUDENT_RESET_PASSWORD);
        setFocusTo(6101);
    }

    void showTeachers(string keyword = "") {
        clearControls();
        currentScreen = SCREEN_VIEW;
        data.loadUsersFromFirebase();
        title("Quản lý giáo viên");
        addNav("Quản trị viên");
        label("Tìm kiếm", 250, 88, 90, 26);
        edit(keyword, 350, 84, 260, 30, 6203);
        button("Tim", 625, 84, 80, 32, ID_TEACHER_SEARCH);
        userListView(250, 128, 760, 315, "teacher", keyword);
        label("ID giáo viên", 270, 455, 110, 26);
        edit("", 390, 451, 170, 30, 6201);
        label("Phụ trách mới", 580, 455, 110, 26);
        edit("", 690, 451, 130, 30, 6202);
        button("Đổi phụ trách", 390, 500, 130, 38, ID_TEACHER_CHANGE_CLASS);
        button("Xóa giáo viên", 535, 500, 130, 38, ID_TEACHER_DELETE);
        button("Về dashboard", 680, 500, 140, 38, ID_DASHBOARD);
        label("Mật khẩu mới", 270, 555, 115, 26);
        edit("", 390, 551, 170, 30, 6204, true);
        button("Đặt lại MK", 580, 548, 130, 38, ID_TEACHER_RESET_PASSWORD);
        setFocusTo(6201);
    }

    void submitChangeManagedUserClass(string role) {
        int idField = role == "student" ? 6101 : 6201;
        int classField = role == "student" ? 6102 : 6202;
        string userId = getText(idField);
        string newClassName = normalizeClassName(getText(classField));

        if (userId.empty() || newClassName.empty()) {
            error("Vui lòng nhập ID và lớp/phụ trách mới.");
            return;
        }

        if (!data.updateUserClass(userId, role, newClassName)) {
            error("Cập nhật thất bại. Vui lòng thử lại.");
            return;
        }

        message(role == "student" ? "Đã chuyển lớp học sinh." : "Đã đổi lớp/môn phụ trách.");
        if (role == "student") showStudents();
        else showTeachers();
    }

    void submitDeleteManagedUser(string role) {
        int idField = role == "student" ? 6101 : 6201;
        string userId = getText(idField);

        if (userId.empty()) {
            error("Vui lòng nhập ID cần xóa.");
            return;
        }

        if (!data.deleteUser(userId, role)) {
            error("Xóa thất bại. Vui lòng thử lại.");
            return;
        }

        message(role == "student" ? "Đã xóa học sinh." : "Đã xóa giáo viên.");
        if (role == "student") showStudents();
        else showTeachers();
    }

    void submitResetManagedUserPassword(string role) {
        int idField = role == "student" ? 6101 : 6201;
        int passwordField = role == "student" ? 6104 : 6204;
        string userId = getText(idField);
        string newPassword = getText(passwordField);

        if (userId.empty() || newPassword.empty()) {
            error("Vui lòng nhập ID và mật khẩu mới.");
            return;
        }

        if (containsRoleKeyword(newPassword)) {
            error("Mật khẩu mới không được chứa từ khóa: admin, teacher, student.");
            return;
        }

        if (!data.resetUserPassword(userId, role, newPassword)) {
            error("Đặt lại mật khẩu thất bại. Vui lòng thử lại.");
            return;
        }

        message(role == "student" ? "Đã đặt lại mật khẩu học sinh." : "Đã đặt lại mật khẩu giáo viên.");
        if (role == "student") showStudents();
        else showTeachers();
    }

    void showCreateTeacher() {
        clearControls();
        currentScreen = SCREEN_CREATE_TEACHER;
        title("Tạo tài khoản giáo viên");
        label("Họ tên", 300, 125, 140, 26);
        edit("", 470, 121, 280, 30, 3001);
        label("Email", 300, 170, 140, 26);
        edit("", 470, 166, 280, 30, 3002);
        label("Mật khẩu", 300, 215, 140, 26);
        edit("", 470, 211, 280, 30, 3003, true);
        label("Lớp/môn phụ trách", 300, 260, 150, 26);
        edit("", 470, 256, 280, 30, 3004);
        defaultButton("Tạo giáo viên", 470, 315, 140, 38, ID_CREATE_TEACHER_SUBMIT);
        button("Về dashboard", 620, 315, 140, 38, ID_DASHBOARD);
        setFocusTo(3001);
    }

    void submitCreateTeacher() {
        string name = getText(3001);
        string email = getText(3002);
        string password = getText(3003);
        string className = normalizeClassName(getText(3004));
        if (name.empty() || email.empty() || password.empty() || className.empty()) {
            error("Vui lòng nhập đầy đủ thông tin.");
            return;
        }
        if (containsRoleKeyword(name) || containsRoleKeyword(password)) {
            error("Tên và mật khẩu không được chứa từ khóa: admin, teacher, student.");
            return;
        }

        if (!data.loadUsersFromFirebase()) {
            error("Không thể tải dữ liệu. Vui lòng kiểm tra Internet rồi thử lại.");
            return;
        }

        if (data.findUserByEmail(email) != nullptr) {
            error("Email này đã tồn tại.");
            return;
        }

        if (!data.addTeacher(name, email, password, className)) {
            error("Tạo giáo viên thất bại. Vui lòng thử lại.");
            return;
        }

        message("Đã tạo tài khoản giáo viên.");
        showAdminDashboard();
    }

    string questionsText(bool showAnswer) {
        string text;
        for (Question& question : data.getQuestions()) {
            text += question.toText(showAnswer);
        }
        return text;
    }

    void showQuestions(bool showAnswer) {
        clearControls();
        currentScreen = SCREEN_VIEW;
        title("Ngân hàng câu hỏi");
        if (currentUser && currentUser->getRole() == "teacher") addNav("Giáo viên");
        else addNav("Quản trị viên");
        edit(questionsText(showAnswer), 260, 100, 650, 460, 0, false, true, true);
        button("Về dashboard", 28, 510, 180, 38, ID_DASHBOARD);
    }

    void showAddQuestion() {
        clearControls();
        currentScreen = SCREEN_ADD_QUESTION;
        title("Thêm câu hỏi");
        int x1 = 280, x2 = 450, y = 100;
        label("Nội dung", x1, y, 140, 26);
        edit("", x2, y - 4, 390, 30, 4001);
        y += 42;
        label("Đáp án A", x1, y, 140, 26);
        edit("", x2, y - 4, 390, 30, 4002);
        y += 42;
        label("Đáp án B", x1, y, 140, 26);
        edit("", x2, y - 4, 390, 30, 4003);
        y += 42;
        label("Đáp án C", x1, y, 140, 26);
        edit("", x2, y - 4, 390, 30, 4004);
        y += 42;
        label("Đáp án D", x1, y, 140, 26);
        edit("", x2, y - 4, 390, 30, 4005);
        y += 42;
        label("Đáp án đúng", x1, y, 140, 26);
        edit("", x2, y - 4, 80, 30, 4006);
        y += 42;
        label("Mon hoc", x1, y, 140, 26);
        edit("", x2, y - 4, 220, 30, 4007);
        y += 42;
        label("Hình ảnh", x1, y, 140, 26);
        edit("", x2, y - 4, 300, 30, 4008);
        button("Chọn ảnh", x2 + 315, y - 4, 100, 32, ID_ADD_QUESTION_CHOOSE_IMAGE);
        defaultButton("Them", x2, y + 48, 130, 38, ID_ADD_QUESTION_SUBMIT);
        button("Về dashboard", x2 + 145, y + 48, 150, 38, ID_DASHBOARD);
        setFocusTo(4001);
    }

    void submitAddQuestion() {
        string content = getText(4001);
        array<string, 4> options = {getText(4002), getText(4003), getText(4004), getText(4005)};
        string correct = getText(4006);
        string subject = getText(4007);
        string imagePath = getText(4008);
        if (content.empty() || options[0].empty() || options[1].empty() || options[2].empty() ||
            options[3].empty() || correct.empty() || subject.empty()) {
            error("Vui lòng nhập đầy đủ thông tin câu hỏi.");
            return;
        }
        char answer = (char)toupper(correct[0]);
        if (answer < 'A' || answer > 'D') {
            error("Đáp án đúng phai la A, B, C hoac D.");
            return;
        }
        data.addQuestion(content, options, answer, subject, imagePath);
        message("Đã thêm câu hỏi.");
        showTeacherDashboard();
    }

    void showImportQuestions() {
        clearControls();
        currentScreen = SCREEN_IMPORT_QUESTIONS;
        title("Nhập câu hỏi", "Chọn file CSV từ máy, dán CSV trực tiếp hoặc tải URL CSV công khai.");
        addNav("Giáo viên");

        label("Môn học/Chủ đề", 260, 88, 130, 26);
        edit("C++ OOP", 400, 84, 210, 30, 8003);
        label("URL CSV", 260, 126, 90, 26);
        edit("", 355, 122, 255, 30, 8001);
        button("Tải URL", 625, 122, 90, 32, ID_IMPORT_QUESTIONS_FROM_URL);
        button("Chọn file", 725, 122, 90, 32, ID_IMPORT_QUESTIONS_FROM_FILE);
        button("Mẫu CSV", 825, 122, 85, 32, ID_IMPORT_QUESTIONS_TEMPLATE);

        edit(csvTemplateText(), 260, 170, 650, 318, 8002, false, true, false);

        defaultButton("Nhập vào ngân hàng", 500, 515, 175, 38, ID_IMPORT_QUESTIONS_SUBMIT);
        button("Về dashboard", 690, 515, 140, 38, ID_DASHBOARD);
        setFocusTo(8002);
    }

    string csvTemplateText() {
        return
            "content,optionA,optionB,optionC,optionD,correctAnswer\r\n"
            "\"Trong lập trình hướng đối tượng, class được dùng để làm gì?\",\"Mô tả khuôn mẫu tạo đối tượng\",\"Chạy chương trình nhanh hơn\",\"Lưu file trên máy tính\",\"Kết nối Internet\",A\r\n"
            "\"Từ khóa nào dùng để khai báo class trong C++?\",\"object\",\"class\",\"define\",\"include\",B\r\n"
            "\"Hàm nào được gọi tự động khi đối tượng được tạo?\",\"Destructor\",\"Constructor\",\"Getter\",\"Setter\",B\r\n"
            "\"Trong C++, thư viện nào thường dùng để làm việc với vector?\",\"<map>\",\"<array>\",\"<vector>\",\"<queue>\",C\r\n"
            "\"Đặc điểm nào là lợi ích của tính đóng gói?\",\"Ẩn chi tiết xử lý bên trong\",\"Làm mất dữ liệu\",\"Xóa toàn bộ biến\",\"Bắt buộc dùng biến global\",A\r\n"
            "\"Nếu đáp án đúng là lựa chọn C, cột correctAnswer cần ghi gì?\",\"A\",\"B\",\"C\",\"D\",C\r\n";
    }

    void showCsvTemplate() {
        string samplePath = appDirectory() + "\\sample_questions.csv";
        wstring widePath = utf8ToWide(samplePath);
        HINSTANCE result = ShellExecuteW(window, L"open", widePath.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
        if ((INT_PTR)result > 32) {
            message("Đã mở file sample_questions.csv. Giáo viên có thể sửa bằng Excel rồi bấm Chọn file để nạp vào app.");
            return;
        }

        HWND csvEdit = GetDlgItem(window, 8002);
        if (csvEdit != nullptr) {
            setControlText(csvEdit, csvTemplateText());
        }
        message("Không mở được file sample_questions.csv. App đã hiển thị mẫu CSV trong ô nhập để sử dụng tạm.");
    }

    void loadImportQuestionsFromUrl() {
        string url = getText(8001);
        if (url.empty()) {
            error("Vui lòng nhập URL CSV.");
            return;
        }

        string csvText;
        if (!data.downloadTextFromUrl(url, csvText)) {
            error("Không tải được CSV từ URL. Vui lòng kiểm tra đường dẫn và kết nối Internet.");
            return;
        }

        HWND csvEdit = GetDlgItem(window, 8002);
        if (csvEdit != nullptr) {
            setControlText(csvEdit, csvText);
        }
        message("Đã tải CSV. Kiểm tra nội dung rồi bấm Nhập vào ngân hàng.");
    }

    void loadImportQuestionsFromFile() {
        wchar_t fileName[MAX_PATH] = L"";
        OPENFILENAMEW dialog = {};
        dialog.lStructSize = sizeof(dialog);
        dialog.hwndOwner = window;
        dialog.lpstrFilter = L"CSV Files (*.csv)\0*.csv\0Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
        dialog.lpstrFile = fileName;
        dialog.nMaxFile = MAX_PATH;
        dialog.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
        dialog.lpstrDefExt = L"csv";

        if (!GetOpenFileNameW(&dialog)) {
            return;
        }

        loadCsvFromPath(wideToUtf8(fileName));
    }

    void loadDroppedCsv(HDROP drop) {
        if (currentScreen != SCREEN_IMPORT_QUESTIONS) {
            DragFinish(drop);
            error("Hay vao man hinh Nhập câu hỏi roi keo-tha file CSV vao app.");
            return;
        }

        wchar_t fileName[MAX_PATH] = L"";
        if (DragQueryFileW(drop, 0, fileName, MAX_PATH) == 0) {
            DragFinish(drop);
            return;
        }

        DragFinish(drop);
        loadCsvFromPath(wideToUtf8(fileName));
    }

    void loadCsvFromPath(string filePath) {
        ifstream file(filePath, ios::binary);
        if (!file) {
            error("Không mở được file CSV đã chọn.");
            return;
        }

        string csvText((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
        if (csvText.size() >= 3 &&
            (unsigned char)csvText[0] == 0xEF &&
            (unsigned char)csvText[1] == 0xBB &&
            (unsigned char)csvText[2] == 0xBF) {
            csvText.erase(0, 3);
        }

        HWND csvEdit = GetDlgItem(window, 8002);
        if (csvEdit != nullptr) {
            setControlText(csvEdit, csvText);
        }

        message("Đã nạp file CSV. Kiểm tra nội dung rồi bấm Nhập vào ngân hàng.");
    }

    void chooseExamFile() {
        wchar_t fileName[MAX_PATH] = L"";
        OPENFILENAMEW dialog = {};
        dialog.lStructSize = sizeof(dialog);
        dialog.hwndOwner = window;
        dialog.lpstrFilter =
            L"File đề thi (*.docx;*.pdf;*.jpg;*.jpeg;*.png)\0*.docx;*.pdf;*.jpg;*.jpeg;*.png\0"
            L"Word/PDF (*.docx;*.pdf)\0*.docx;*.pdf\0"
            L"Anh (*.jpg;*.jpeg;*.png)\0*.jpg;*.jpeg;*.png\0"
            L"Tat ca file (*.*)\0*.*\0";
        dialog.lpstrFile = fileName;
        dialog.nMaxFile = MAX_PATH;
        dialog.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;

        if (!GetOpenFileNameW(&dialog)) {
            return;
        }

        HWND pathEdit = GetDlgItem(window, 5007);
        if (pathEdit != nullptr) {
            setControlText(pathEdit, wideToUtf8(fileName));
        }
    }

    void chooseQuestionImage() {
        wchar_t fileName[MAX_PATH] = L"";
        OPENFILENAMEW dialog = {};
        dialog.lStructSize = sizeof(dialog);
        dialog.hwndOwner = window;
        dialog.lpstrFilter =
            L"Image Files (*.jpg;*.jpeg;*.png;*.bmp)\0*.jpg;*.jpeg;*.png;*.bmp\0"
            L"All Files (*.*)\0*.*\0";
        dialog.lpstrFile = fileName;
        dialog.nMaxFile = MAX_PATH;
        dialog.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;

        if (!GetOpenFileNameW(&dialog)) {
            return;
        }

        HWND pathEdit = GetDlgItem(window, 4008);
        if (pathEdit != nullptr) {
            setControlText(pathEdit, wideToUtf8(fileName));
        }
    }

    void submitImportQuestions() {
        string subject = getText(8003);
        string csvText = getText(8002);
        if (subject.empty()) {
            error("Vui lòng nhập Môn học/Chủ đề cho bộ câu hỏi.");
            return;
        }
        if (csvText.empty()) {
            error("Vui lòng dán CSV hoặc tải CSV từ URL.");
            return;
        }

        string importMessage;
        int importedCount = data.importQuestionsFromCsv(csvText, subject, importMessage);
        if (importedCount <= 0) {
            error("Không có câu hỏi hợp lệ để nhập.\r\n\r\nĐịnh dạng: content,optionA,optionB,optionC,optionD,correctAnswer");
            return;
        }

        message(importMessage);
        showTeacherDashboard();
    }

    void showDeleteQuestion() {
        clearControls();
        currentScreen = SCREEN_DELETE_QUESTION;
        title("Xóa câu hỏi");
        edit(questionsText(true), 260, 92, 650, 300, 0, false, true, true);
        label("Mã câu hỏi can xoa", 300, 420, 160, 26);
        edit("", 480, 416, 160, 30, 4101);
        defaultButton("Xoa", 480, 465, 100, 38, ID_DELETE_QUESTION_SUBMIT);
        button("Về dashboard", 595, 465, 150, 38, ID_DASHBOARD);
        setFocusTo(4101);
    }

    void submitDeleteQuestion() {
        string id = getText(4101);
        if (data.deleteQuestion(id)) {
            message("Đã xóa câu hỏi.");
            showTeacherDashboard();
        } else {
            error("Không tìm thấy câu hỏi.");
        }
    }

    string examsText(bool onlyOpen = false) {
        stringstream ss;
        ss << "ID    Tên đề                Làm bài   Mở lúc            Đóng lúc          Số câu  Lượt  Giáo viên tải     MK   File Trạng thái\r\n";
        ss << "-------------------------------------------------------------------------------------------------------------------------------\r\n";
        for (Exam& exam : data.getExams()) {
            string now = QuizData::currentMinuteText();
            if (onlyOpen && !exam.canTake(now)) continue;
            string status = exam.statusText(now);
            ss << left << setw(6) << exam.getExamId()
               << setw(22) << exam.getTitle()
               << setw(10) << (to_string(exam.getDurationMinutes()) + " phut")
               << setw(18) << (exam.getStartAt().empty() ? "-" : exam.getStartAt())
               << setw(18) << exam.getCloseAt()
               << setw(8) << exam.totalQuestions()
               << setw(6) << exam.getAttemptLimit()
               << setw(20) << data.getUserNameById(exam.getCreatedBy())
               << setw(5) << (exam.hasPassword() ? "Có" : "Không")
               << setw(5) << (exam.hasAttachment() ? "Có" : "Không")
               << status << "\r\n";
        }
        return ss.str();
    }

    void showExams() {
        clearControls();
        currentScreen = SCREEN_VIEW;
        title("Danh sách đề thi");
        if (currentUser && currentUser->getRole() == "teacher") addNav("Giáo viên");
        else addNav("Quản trị viên");
        examListView(250, 100, 680, 390, false);
        button("Về dashboard", 28, 510, 180, 38, ID_DASHBOARD);
    }

    void showOpenExams() {
        clearControls();
        currentScreen = SCREEN_VIEW;
        title("Đề thi đang mở");
        addNav("Học sinh");
        examListView(250, 100, 680, 390, true);
        button("Về dashboard", 28, 510, 180, 38, ID_DASHBOARD);
    }

    void showCreateExam() {
        clearControls();
        currentScreen = SCREEN_CREATE_EXAM;
        title("Giáo viên tải đề thi", "Đề từ ngân hàng: nhập Mã câu hỏi. Đề file gốc: để trống Mã câu hỏi và nhập Đáp án file đề.");
        edit(questionsText(true), 260, 78, 650, 125, 0, false, true, true);
        label("Tên đề thi", 290, 225, 170, 26);
        edit("", 500, 221, 280, 30, 5001);
        label("Thời gian làm bài", 290, 263, 170, 26);
        edit("30", 500, 259, 100, 30, 5002);
        label("Số lần làm tối đa", 620, 263, 160, 26);
        edit("1", 790, 259, 70, 30, 5010);
        label("Mở lúc", 290, 301, 170, 26);
        edit(QuizData::currentMinuteText(), 500, 297, 190, 30, 5006);
        label("Đóng lúc", 290, 339, 170, 26);
        edit("2026-12-31 23:59", 500, 335, 190, 30, 5004);
        label("Mã câu hỏi", 290, 377, 170, 26);
        edit("", 500, 373, 280, 30, 5003);
        label("Dung q1 q2... neu lay tu ngan hang", 795, 377, 270, 26);
        label("Đáp án file đề", 290, 415, 170, 26);
        edit("", 500, 411, 405, 30, 5009);
        label("Vi du: A C B B A", 920, 415, 160, 26);
        label("Mật khẩu de", 290, 453, 170, 26);
        edit("", 500, 449, 190, 30, 5005);
        label("Bỏ trống nếu đề không cần mật khẩu", 700, 453, 270, 26);
        label("File đề", 290, 491, 170, 26);
        edit("", 500, 487, 280, 30, 5007);
        button("Chọn file", 795, 487, 110, 32, ID_CREATE_EXAM_CHOOSE_FILE);
        label("Link file đề", 290, 529, 170, 26);
        edit("", 500, 525, 405, 30, 5008);
        defaultButton("Tạo đề", 500, 585, 120, 38, ID_CREATE_EXAM_SUBMIT);
        button("Về dashboard", 635, 585, 150, 38, ID_DASHBOARD);
        setFocusTo(5001);
    }

    void submitCreateExam() {
        string title = getText(5001);
        string durationText = getText(5002);
        string idsText = getText(5003);
        string closeAt = getText(5004);
        string examPassword = getText(5005);
        string startAt = getText(5006);
        string attachmentPath = getText(5007);
        string attachmentUrl = getText(5008);
        string answerKeyText = getText(5009);
        string attemptLimitText = getText(5010);
        if (title.empty() || durationText.empty() || startAt.empty() || closeAt.empty()) {
            error("Vui lòng nhập đầy đủ thông tin đề thi.");
            return;
        }

        int duration = atoi(durationText.c_str());
        if (duration <= 0) {
            error("Thời gian làm bài phai lon hon 0.");
            return;
        }
        int attemptLimit = atoi(attemptLimitText.c_str());
        if (attemptLimit <= 0) {
            error("Số lần làm tối đa phải lớn hơn 0.");
            return;
        }
        if (!isDateTimeText(startAt) || !isDateTimeText(closeAt)) {
            error("Thời gian mở/đóng đề cần theo dạng YYYY-MM-DD HH:MM.");
            return;
        }
        if (closeAt <= startAt) {
            error("Thời gian đóng đề phải sau thời gian mở đề.");
            return;
        }

        vector<string> ids = parseQuestionIds(idsText);
        vector<string> validIds;
        for (string id : ids) {
            if (data.findQuestionById(id) != nullptr) validIds.push_back(id);
        }

        vector<char> answerKey;
        string answerKeyError;
        if (!answerKeyText.empty() && !parseAnswerKey(answerKeyText, answerKey, answerKeyError)) {
            error(answerKeyError);
            return;
        }

        if (!ids.empty() && validIds.empty()) {
            error("Không có mã câu hỏi hợp lệ.");
            return;
        }
        if (!validIds.empty() && !answerKey.empty()) {
            error("Chỉ nhập Mã câu hỏi hoặc Đáp án file đề, không cần nhập cả hai.");
            return;
        }
        if (validIds.empty() && answerKey.empty()) {
            error("Nhap Mã câu hỏi tu ngan hang hoac nhap Đáp án file đề, vi du: A C B B A.");
            return;
        }
        if (validIds.empty() && attachmentPath.empty() && attachmentUrl.empty()) {
            error("De file goc can chon File đề hoac nhap Link file đề.");
            return;
        }

        data.addExam(title, duration, validIds, answerKey, currentUser->getUserId(), startAt, closeAt,
                     examPassword, attachmentPath, attachmentUrl, attemptLimit);
        message("Đã tạo đề thi.");
        showTeacherDashboard();
    }

    void showToggleExam() {
        clearControls();
        currentScreen = SCREEN_TOGGLE_EXAM;
        title("Mở/đóng đề thi");
        examListView(250, 90, 680, 330, false);
        label("Mã đề thi", 300, 455, 140, 26);
        edit("", 460, 451, 150, 30, 5101);
        defaultButton("Đổi trạng thái", 460, 505, 140, 38, ID_TOGGLE_EXAM_SUBMIT);
        button("Về dashboard", 615, 505, 150, 38, ID_DASHBOARD);
        label("Số lượt mới", 300, 565, 140, 26);
        edit("1", 460, 561, 100, 30, 5102);
        button("Cập nhật lượt", 580, 557, 150, 38, ID_UPDATE_EXAM_ATTEMPT_LIMIT);
        setFocusTo(5101);
    }

    void submitToggleExam() {
        string examId = getText(5101);
        Exam* exam = data.findExamById(examId);
        if (exam == nullptr) {
            error("Không tìm thấy đề thi.");
            return;
        }
        exam->setOpen(!exam->isOpen());
        message("Đã đổi trạng thái đề thi.");
        if (currentUser->getRole() == "teacher") showTeacherDashboard();
        else showAdminDashboard();
    }

    void submitUpdateExamAttemptLimit() {
        string examId = getText(5101);
        int attemptLimit = atoi(getText(5102).c_str());
        if (examId.empty()) {
            error("Vui lòng nhập mã đề thi.");
            return;
        }
        if (attemptLimit <= 0) {
            error("Số lượt mới phải lớn hơn 0.");
            return;
        }
        if (!data.updateExamAttemptLimit(examId, attemptLimit)) {
            error("Không thể tải danh sách đề thi. Vui lòng thử lại.");
            return;
        }

        message("Đã cập nhật số lượt làm bài.");
        showToggleExam();
    }

    void showTakeExamSelect() {
        clearControls();
        currentScreen = SCREEN_TAKE_EXAM_SELECT;
        title("Làm bài thi");
        examListView(250, 88, 680, 330, true);
        label("Mã đề thi", 300, 455, 140, 26);
        edit("", 460, 451, 150, 30, 5201);
        label("Mật khẩu de", 300, 500, 140, 26);
        edit("", 460, 496, 150, 30, 5202, true);
        defaultButton("Bắt đầu", 635, 451, 120, 38, ID_TAKE_EXAM_SUBMIT);
        button("Về dashboard", 635, 496, 150, 38, ID_DASHBOARD);
        setFocusTo(5201);
    }

    void showTakeExamForm(Exam& exam) {
        clearControls();
        currentScreen = SCREEN_TAKE_EXAM_FORM;
        activeExamId = exam.getExamId();
        title("Làm bài: " + exam.getTitle(),
              "Thời gian làm bài: " + to_string(exam.getDurationMinutes()) +
              " phut | Đóng lúc: " + exam.getCloseAt());
        examTimerLabel = label("", 735, 62, 190, 24);
        if (exam.hasAttachment()) {
            button("Mở file đề", 575, 58, 130, 30, ID_TAKE_EXAM_OPEN_FILE);
        }
        activeExamEndTime = chrono::system_clock::now() + chrono::minutes(exam.getDurationMinutes());
        SetTimer(window, EXAM_TIMER_ID, 1000, nullptr);
        examTimerActive = true;
        updateExamTimer();
        int y = 95;
        int index = 1;
        answerOptions.clear();
        answerCombos.clear();
        activeQuestionIds.clear();

        if (exam.isFileAnswerExam()) {
            vector<char> answerKey = exam.getAnswerKey();
            int totalQuestions = (int)answerKey.size();
            int rowsPerBlock = 20;
            int blockCount = min(3, max(1, (totalQuestions + rowsPerBlock - 1) / rowsPerBlock));
            bool compactAnswerSheet = blockCount > 1;
            int answerPanelX = 270;
            int answerPanelY = y;
            int blockWidth = compactAnswerSheet ? 132 : 250;
            int answerPanelW = blockCount * blockWidth + 36;
            int contentX = answerPanelX + answerPanelW + 24;
            int contentY = y;
            int contentW = max(560, 1320 - contentX);

            HWND answerTitle = label("Bảng trả lời", answerPanelX + 18, answerPanelY + 16, 190, 28);
            SendMessageW(answerTitle, WM_SETFONT, (WPARAM)brandFont, TRUE);
            HWND answerHint = label("Chọn đáp án cho từng câu", answerPanelX + 18, answerPanelY + 48,
                                    answerPanelW - 36, 22);
            SendMessageW(answerHint, WM_SETFONT, (WPARAM)smallFont, TRUE);

            HWND contentTitle = label("Nội dung đề", contentX, contentY, 160, 24);
            SendMessageW(contentTitle, WM_SETFONT, (WPARAM)brandFont, TRUE);
            addControl("STATIC", "", SS_ETCHEDFRAME, contentX, contentY + 30, contentW, 585, 0, false);
            showInlineExamAttachment(exam, contentX + 15, contentY + 45, contentW - 30, 555);

            int tableY = answerPanelY + 82;
            int rowHeight = compactAnswerSheet ? 27 : 31;
            int questionW = compactAnswerSheet ? 34 : 58;
            int optionW = compactAnswerSheet ? 23 : 42;
            int optionGap = compactAnswerSheet ? 3 : 10;

            for (int block = 0; block < blockCount; block++) {
                int startIndex = block * rowsPerBlock;
                if (startIndex >= (int)answerKey.size()) {
                    break;
                }

                int x = answerPanelX + 18 + block * blockWidth;
                HWND questionHeader = label("Câu", x, tableY, questionW, 24);
                SendMessageW(questionHeader, WM_SETFONT, (WPARAM)smallFont, TRUE);
                int optionHeaderX = x + questionW;
                label("A", optionHeaderX + 6, tableY, optionW, 24);
                label("B", optionHeaderX + (optionW + optionGap) + 6, tableY, optionW, 24);
                label("C", optionHeaderX + (optionW + optionGap) * 2 + 6, tableY, optionW, 24);
                label("D", optionHeaderX + (optionW + optionGap) * 3 + 6, tableY, optionW, 24);

                for (int row = 0; row < rowsPerBlock; row++) {
                    int index = startIndex + row;
                    if (index >= (int)answerKey.size()) {
                        break;
                    }

                    int itemY = tableY + 28 + row * rowHeight;
                    HWND questionNumber = label(to_string(index + 1), x, itemY + 3, questionW, 24);
                    SendMessageW(questionNumber, WM_SETFONT, (WPARAM)smallFont, TRUE);
                    int optionX = x + questionW;
                    array<HWND, 4> buttons = {
                        answerChoiceButton("A", optionX, itemY, optionW, 25, 5400 + index * 10 + 0, true),
                        answerChoiceButton("B", optionX + (optionW + optionGap), itemY, optionW, 25, 5400 + index * 10 + 1, false),
                        answerChoiceButton("C", optionX + (optionW + optionGap) * 2, itemY, optionW, 25, 5400 + index * 10 + 2, false),
                        answerChoiceButton("D", optionX + (optionW + optionGap) * 3, itemY, optionW, 25, 5400 + index * 10 + 3, false),
                    };
                    answerOptions.push_back(buttons);
                    activeQuestionIds.push_back("c" + to_string(index + 1));
                }
            }

            defaultButton("Nộp bài", 720, 725, 120, 38, ID_TAKE_EXAM_SUBMIT);
            button("Về dashboard", 560, 725, 145, 38, ID_DASHBOARD);
            if (!answerOptions.empty()) {
                SetFocus(answerOptions[0][0]);
            }
            return;
        }

        for (string questionId : exam.getQuestionIds()) {
            Question* question = data.findQuestionById(questionId);
            if (question == nullptr || y > 650) continue;

            string line = to_string(index) + ". " + question->getContent();
            HWND questionLabel = label(line, 270, y, 880, 28);
            SendMessageW(questionLabel, WM_SETFONT, (WPARAM)brandFont, TRUE);
            y += 36;
            if (question->hasImage() && !question->getImagePath().empty()) {
                HWND image = imageBox(question->getImagePath(), 300, y, 320, 120);
                if (image != nullptr) {
                    y += 132;
                }
            }
            array<string, 4> options = question->getOptions();
            array<HWND, 4> buttons = {
                answerChoiceButton("A. " + options[0], 300, y, 410, 34, 5300 + index * 10 + 0, true),
                answerChoiceButton("B. " + options[1], 730, y, 410, 34, 5300 + index * 10 + 1, false),
                answerChoiceButton("C. " + options[2], 300, y + 42, 410, 34, 5300 + index * 10 + 2, false),
                answerChoiceButton("D. " + options[3], 730, y + 42, 410, 34, 5300 + index * 10 + 3, false),
            };
            answerOptions.push_back(buttons);
            activeQuestionIds.push_back(questionId);
            y += 94;
            index++;
        }

        defaultButton("Nộp bài", 720, 735, 120, 38, ID_TAKE_EXAM_SUBMIT);
        button("Về dashboard", 560, 735, 145, 38, ID_DASHBOARD);
        if (!answerOptions.empty()) {
            SetFocus(answerOptions[0][0]);
        }
    }

    void openActiveExamAttachment() {
        Exam* exam = data.findExamById(activeExamId);
        if (exam == nullptr) {
            error("Không tìm thấy đề thi đang làm.");
            return;
        }

        string target = exam->getAttachmentUrl().empty()
                            ? exam->getAttachmentPath()
                            : exam->getAttachmentUrl();
        if (target.empty()) {
            error("Đề thi nay chua co file dinh kem.");
            return;
        }

        wstring wideTarget = utf8ToWide(target);
        HINSTANCE result = ShellExecuteW(window, L"open", wideTarget.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
        if ((INT_PTR)result <= 32) {
            error("Không mở được file/link đề thi. Nếu học sinh dùng máy khác, giáo viên nên dùng liên kết công khai.");
        }
    }

    void showInlineExamAttachment(Exam& exam, int x, int y, int w, int h) {
        string localPath = exam.getAttachmentPath();
        string url = exam.getAttachmentUrl();

        if (!localPath.empty() && isImageFile(localPath)) {
            HWND image = imageBox(localPath, x, y, w, h);
            if (image != nullptr) {
                return;
            }
        }

        if (!localPath.empty() && isDocxFile(localPath)) {
            string previewText;
            if (loadDocxPreviewText(localPath, previewText)) {
                edit(previewText, x, y, w, h, 0, false, true, true);
                return;
            }

            label("Không đọc được nội dung DOCX trong app.", x + 20, y + 30, w - 40, 26);
            label("Bạn vẫn có thể bấm Mở file đề để xem bằng Word.", x + 20, y + 62, w - 40, 26);
            return;
        }

        if (!localPath.empty() && isPdfFile(localPath)) {
            label("File PDF không xem trực tiếp trong khung này để tránh lỗi trắng/không kéo được.", x + 20, y + 30, w - 40, 26);
            label("Bấm Mở file đề để xem bằng Microsoft Edge, Chrome hoặc Adobe Reader.", x + 20, y + 62, w - 40, 26);
            label("Muốn xem ngay trong app thì giáo viên nên dùng file ảnh .jpg/.png.", x + 20, y + 94, w - 40, 26);
            return;
        }

        if (!url.empty() && isImageFile(url)) {
            label("Link ảnh công khai chưa hỗ trợ xem trực tiếp trong khung.", x + 20, y + 30, w - 40, 26);
            label("Bấm Mở file đề để xem bằng trình duyệt.", x + 20, y + 62, w - 40, 26);
            return;
        }

        if (!localPath.empty() || !url.empty()) {
            label("File Word/PDF hiện chưa xem trực tiếp trong khung này.", x + 20, y + 30, w - 40, 26);
            label("Bấm Mở file đề để mở bằng Word/PDF viewer.", x + 20, y + 62, w - 40, 26);
            label("Nếu muốn xem ngay trong app, hãy dùng file ảnh .jpg/.png.", x + 20, y + 94, w - 40, 26);
            return;
        }

        label("Đề này chưa có file đính kèm.", x + 20, y + 30, w - 40, 26);
    }

    void trimInPlace(string& text) {
        while (!text.empty() && isspace((unsigned char)text.front())) {
            text.erase(text.begin());
        }
        while (!text.empty() && isspace((unsigned char)text.back())) {
            text.pop_back();
        }
    }

    bool loadDocxPreviewText(string path, string& previewText) {
        vector<unsigned char> fileData;
        if (!readBinaryFile(path, fileData)) {
            return false;
        }

        vector<unsigned char> xmlData;
        if (!extractZipEntry(fileData, "word/document.xml", xmlData)) {
            return false;
        }

        string xml(xmlData.begin(), xmlData.end());
        previewText = docxXmlToText(xml);
        trimInPlace(previewText);
        return !previewText.empty();
    }

    bool readBinaryFile(string path, vector<unsigned char>& data) {
        ifstream file(path, ios::binary);
        if (!file) {
            return false;
        }

        data.assign(istreambuf_iterator<char>(file), istreambuf_iterator<char>());
        return !data.empty();
    }

    uint16_t readU16(const vector<unsigned char>& data, size_t offset) {
        if (offset + 2 > data.size()) {
            return 0;
        }
        return (uint16_t)data[offset] | ((uint16_t)data[offset + 1] << 8);
    }

    uint32_t readU32(const vector<unsigned char>& data, size_t offset) {
        if (offset + 4 > data.size()) {
            return 0;
        }
        return (uint32_t)data[offset] |
               ((uint32_t)data[offset + 1] << 8) |
               ((uint32_t)data[offset + 2] << 16) |
               ((uint32_t)data[offset + 3] << 24);
    }

    bool extractZipEntry(const vector<unsigned char>& zipData, string wantedName, vector<unsigned char>& output) {
        if (zipData.size() < 22) {
            return false;
        }

        size_t searchStart = zipData.size() > 66000 ? zipData.size() - 66000 : 0;
        size_t eocd = string::npos;
        for (size_t i = zipData.size() - 22; i + 1 > searchStart; i--) {
            if (readU32(zipData, i) == 0x06054b50) {
                eocd = i;
                break;
            }
            if (i == 0) {
                break;
            }
        }
        if (eocd == string::npos) {
            return false;
        }

        uint16_t entryCount = readU16(zipData, eocd + 10);
        uint32_t centralOffset = readU32(zipData, eocd + 16);
        size_t pos = centralOffset;

        for (uint16_t entryIndex = 0; entryIndex < entryCount && pos + 46 <= zipData.size(); entryIndex++) {
            if (readU32(zipData, pos) != 0x02014b50) {
                return false;
            }

            uint16_t method = readU16(zipData, pos + 10);
            uint32_t compressedSize = readU32(zipData, pos + 20);
            uint32_t uncompressedSize = readU32(zipData, pos + 24);
            uint16_t nameLen = readU16(zipData, pos + 28);
            uint16_t extraLen = readU16(zipData, pos + 30);
            uint16_t commentLen = readU16(zipData, pos + 32);
            uint32_t localOffset = readU32(zipData, pos + 42);

            if (pos + 46 + nameLen > zipData.size()) {
                return false;
            }

            string name((const char*)&zipData[pos + 46], nameLen);
            if (name == wantedName) {
                return inflateZipEntry(zipData, localOffset, method, compressedSize, uncompressedSize, output);
            }

            pos += 46 + nameLen + extraLen + commentLen;
        }

        return false;
    }

    bool inflateZipEntry(const vector<unsigned char>& zipData, uint32_t localOffset, uint16_t method,
                         uint32_t compressedSize, uint32_t uncompressedSize, vector<unsigned char>& output) {
        size_t pos = localOffset;
        if (pos + 30 > zipData.size() || readU32(zipData, pos) != 0x04034b50) {
            return false;
        }

        uint16_t nameLen = readU16(zipData, pos + 26);
        uint16_t extraLen = readU16(zipData, pos + 28);
        size_t dataOffset = pos + 30 + nameLen + extraLen;
        if (dataOffset + compressedSize > zipData.size()) {
            return false;
        }

        const unsigned char* compressed = zipData.data() + dataOffset;
        if (method == 0) {
            output.assign(compressed, compressed + compressedSize);
            return true;
        }

        if (method != 8) {
            return false;
        }

        output.assign(uncompressedSize, 0);
        z_stream stream = {};
        stream.next_in = (Bytef*)compressed;
        stream.avail_in = compressedSize;
        stream.next_out = output.data();
        stream.avail_out = uncompressedSize;

        if (inflateInit2(&stream, -MAX_WBITS) != Z_OK) {
            return false;
        }

        int result = inflate(&stream, Z_FINISH);
        inflateEnd(&stream);
        if (result != Z_STREAM_END) {
            return false;
        }

        output.resize(stream.total_out);
        return true;
    }

    string docxXmlToText(const string& xml) {
        string text;
        string buffer;
        for (size_t i = 0; i < xml.size();) {
            if (xml[i] == '<') {
                size_t end = xml.find('>', i);
                if (end == string::npos) {
                    break;
                }

                string tag = xml.substr(i + 1, end - i - 1);
                if (tag.rfind("/w:p", 0) == 0 || tag.rfind("w:br", 0) == 0 ||
                    tag.rfind("w:cr", 0) == 0 || tag.rfind("/w:tr", 0) == 0) {
                    appendPreviewLine(text, buffer);
                } else if (tag.rfind("w:tab", 0) == 0) {
                    buffer += "    ";
                }
                i = end + 1;
            } else {
                buffer += xml[i];
                i++;
            }
        }

        appendPreviewLine(text, buffer);
        return text;
    }

    void appendPreviewLine(string& text, string& buffer) {
        string line = decodeXmlEntities(buffer);
        trimInPlace(line);
        if (!line.empty()) {
            text += line + "\r\n";
        }
        buffer.clear();
    }

    string decodeXmlEntities(string value) {
        string result;
        for (size_t i = 0; i < value.size();) {
            if (value[i] == '&') {
                size_t semi = value.find(';', i + 1);
                if (semi != string::npos) {
                    string entity = value.substr(i + 1, semi - i - 1);
                    if (entity == "amp") result += "&";
                    else if (entity == "lt") result += "<";
                    else if (entity == "gt") result += ">";
                    else if (entity == "quot") result += "\"";
                    else if (entity == "apos") result += "'";
                    else if (!entity.empty() && entity[0] == '#') result += decodeNumericEntity(entity);
                    else result += "&" + entity + ";";
                    i = semi + 1;
                    continue;
                }
            }

            result += value[i];
            i++;
        }
        return result;
    }

    string decodeNumericEntity(string entity) {
        int codepoint = 0;
        try {
            if (entity.size() > 2 && (entity[1] == 'x' || entity[1] == 'X')) {
                codepoint = stoi(entity.substr(2), nullptr, 16);
            } else {
                codepoint = stoi(entity.substr(1), nullptr, 10);
            }
        } catch (...) {
            return "";
        }

        return utf8FromCodepoint(codepoint);
    }

    string utf8FromCodepoint(int codepoint) {
        string out;
        if (codepoint <= 0x7F) {
            out += (char)codepoint;
        } else if (codepoint <= 0x7FF) {
            out += (char)(0xC0 | ((codepoint >> 6) & 0x1F));
            out += (char)(0x80 | (codepoint & 0x3F));
        } else if (codepoint <= 0xFFFF) {
            out += (char)(0xE0 | ((codepoint >> 12) & 0x0F));
            out += (char)(0x80 | ((codepoint >> 6) & 0x3F));
            out += (char)(0x80 | (codepoint & 0x3F));
        } else {
            out += (char)(0xF0 | ((codepoint >> 18) & 0x07));
            out += (char)(0x80 | ((codepoint >> 12) & 0x3F));
            out += (char)(0x80 | ((codepoint >> 6) & 0x3F));
            out += (char)(0x80 | (codepoint & 0x3F));
        }
        return out;
    }

    bool isImageFile(string path) {
        string value = lowerText(path);
        return endsWith(value, ".jpg") ||
               endsWith(value, ".jpeg") ||
               endsWith(value, ".png") ||
               endsWith(value, ".bmp");
    }

    bool isDocxFile(string path) {
        return endsWith(lowerText(path), ".docx");
    }

    bool isPdfFile(string path) {
        return endsWith(lowerText(path), ".pdf");
    }

    bool endsWith(string text, string suffix) {
        return text.size() >= suffix.size() &&
               text.compare(text.size() - suffix.size(), suffix.size(), suffix) == 0;
    }

    void submitTakeExam(bool autoByTimeout = false) {
        if (activeExamId.empty()) {
            string examId = getText(5201);
            string examPassword = getText(5202);
            Exam* exam = data.findExamById(examId);
            if (exam == nullptr) {
                error("Không tìm thấy đề thi.");
                return;
            }
            if (!exam->canTake(QuizData::currentMinuteText())) {
                string now = QuizData::currentMinuteText();
                if (exam->isBeforeStart(now)) {
                    error("Chua den gio lam bai. Đề thi mo luc: " + exam->getStartAt());
                } else if (exam->isClosedByTime(now)) {
                    error("Đề thi da het han luc: " + exam->getCloseAt());
                } else {
                    error("Đề thi đang đóng.");
                }
                return;
            }
            if (!exam->checkExamPassword(examPassword)) {
                error("Mật khẩu đề thi không đúng.");
                return;
            }
            int usedAttempts = data.countAttempts(currentUser->getUserId(), exam->getExamId());
            if (usedAttempts >= exam->getAttemptLimit()) {
                error("Bạn đã hết lượt làm bài cho đề này. Số lượt tối đa: " + to_string(exam->getAttemptLimit()) + ".");
                return;
            }
            showTakeExamForm(*exam);
            return;
        }

        Exam* exam = data.findExamById(activeExamId);
        if (exam == nullptr) {
            error("Không tìm thấy đề thi.");
            return;
        }

        map<string, char> answers;
        if (!collectSelectedAnswers(answers, !autoByTimeout)) {
            return;
        }

        stopExamTimer();
        data.submitExam(currentUser, *exam, answers);
        if (autoByTimeout) {
            message("Hết giờ làm bài. Hệ thống đã tự động nộp bài.");
        } else {
            message("Đã nộp bài. Vào Lịch sử điểm để xem kết quả.");
        }
        showStudentDashboard();
    }

    bool collectSelectedAnswers(map<string, char>& answers, bool requireAll) {
        if (!answerCombos.empty()) {
            for (int i = 0; i < (int)answerCombos.size() && i < (int)activeQuestionIds.size(); i++) {
                int selectedIndex = (int)SendMessageW(answerCombos[i], CB_GETCURSEL, 0, 0);
                if (selectedIndex < 0 || selectedIndex > 3) {
                    if (requireAll) {
                        error("Vui lòng chọn đáp án cho tất cả câu hỏi.");
                        return false;
                    }
                    continue;
                }

                answers[activeQuestionIds[i]] = (char)('A' + selectedIndex);
            }
            return true;
        }

        for (int i = 0; i < (int)answerOptions.size() && i < (int)activeQuestionIds.size(); i++) {
            char selectedAnswer = '\0';
            for (int optionIndex = 0; optionIndex < 4; optionIndex++) {
                HWND optionButton = answerOptions[i][optionIndex];
                auto selectedIterator = answerChoiceButtons.find(optionButton);
                bool selectedByApp = selectedIterator != answerChoiceButtons.end() && selectedIterator->second;
                bool selectedByWindows = SendMessageW(optionButton, BM_GETCHECK, 0, 0) == BST_CHECKED;
                if (selectedByApp || selectedByWindows) {
                    selectedAnswer = (char)('A' + optionIndex);
                    break;
                }
            }

            if (selectedAnswer == '\0') {
                if (requireAll) {
                    error("Vui lòng chọn đáp án cho tất cả câu hỏi.");
                    return false;
                }
                continue;
            }

            answers[activeQuestionIds[i]] = selectedAnswer;
        }

        return true;
    }

    void updateExamTimer() {
        if (currentScreen != SCREEN_TAKE_EXAM_FORM || activeExamId.empty()) {
            stopExamTimer();
            return;
        }

        auto now = chrono::system_clock::now();
        long long remainingSeconds = chrono::duration_cast<chrono::seconds>(activeExamEndTime - now).count();
        if (remainingSeconds <= 0) {
            if (examTimerLabel != nullptr) {
                setControlText(examTimerLabel, "Còn lại: 00:00");
            }
            stopExamTimer();
            submitTakeExam(true);
            return;
        }

        if (examTimerLabel != nullptr) {
            setControlText(examTimerLabel, "Còn lại: " + formatDuration(remainingSeconds));
        }
    }

    void stopExamTimer() {
        if (examTimerActive && window != nullptr) {
            KillTimer(window, EXAM_TIMER_ID);
        }
        examTimerActive = false;
    }

    string formatDuration(long long totalSeconds) {
        long long minutes = totalSeconds / 60;
        long long seconds = totalSeconds % 60;
        stringstream ss;
        ss << setw(2) << setfill('0') << minutes
           << ":" << setw(2) << setfill('0') << seconds
           << setfill(' ');
        return ss.str();
    }

    string resultsText(bool onlyCurrentStudent) {
        stringstream ss;
        ss << "ID      Học sinh        Đề thi                    Điểm      Thời gian nộp\r\n";
        ss << "--------------------------------------------------------------------------------\r\n";
        for (Result& result : data.getResults()) {
            if (onlyCurrentStudent && result.getStudentId() != currentUser->getUserId()) continue;
            ss << left << setw(8) << result.getResultId()
               << setw(16) << data.getUserNameById(result.getStudentId())
               << setw(26) << data.getExamTitle(result.getExamId())
               << setw(10) << (to_string(result.getScore()) + "/" + to_string(result.getTotalQuestions()))
               << result.getSubmittedAt() << "\r\n";
        }
        return ss.str();
    }

    HWND resultsListView(int x, int y, int w, int h, bool onlyCurrentStudent) {
        HWND list = CreateWindowExW(
            WS_EX_CLIENTEDGE,
            WC_LISTVIEWW,
            L"",
            WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
            x, y, w, h, window, nullptr, instance, nullptr
        );
        styleModernListView(list, 38);
        controls.push_back(list);

        addListColumn(list, 0, "Mã kết quả", 120);
        addListColumn(list, 1, "Học sinh", 250);
        addListColumn(list, 2, "Đề thi", 280);
        addListColumn(list, 3, "Điểm", 110, LVCFMT_CENTER);
        addListColumn(list, 4, "Thời gian nộp", 220);

        int row = 0;
        for (Result& result : data.getResults()) {
            if (onlyCurrentStudent &&
                (currentUser == nullptr || result.getStudentId() != currentUser->getUserId())) {
                continue;
            }

            vector<string> values = {
                result.getResultId(),
                data.getUserNameById(result.getStudentId()),
                data.getExamTitle(result.getExamId()),
                to_string(result.getScore()) + "/" + to_string(result.getTotalQuestions()),
                result.getSubmittedAt()
            };

            wstring firstValue = utf8ToWide(values[0]);
            LVITEMW item = {};
            item.mask = LVIF_TEXT;
            item.iItem = row;
            item.iSubItem = 0;
            item.pszText = const_cast<wchar_t*>(firstValue.c_str());
            SendMessageW(list, LVM_INSERTITEMW, 0, (LPARAM)&item);

            for (int col = 1; col < (int)values.size(); col++) {
                setListText(list, row, col, values[col]);
            }
            row++;
        }

        return list;
    }

    void showAllResults() {
        clearControls();
        currentScreen = SCREEN_VIEW;
        title("Kết quả tất cả học sinh", "Theo dõi điểm và thời gian nộp bài.");
        resultsListView(280, 120, 940, 570, false);
        button("Về dashboard", 28, 640, 190, 42, ID_DASHBOARD);
    }

    void showMyResults() {
        clearControls();
        currentScreen = SCREEN_VIEW;
        title("Lịch sử điểm", "Các bài thi bạn đã hoàn thành.");
        resultsListView(280, 120, 940, 570, true);
        button("Về dashboard", 28, 640, 190, 42, ID_DASHBOARD);
    }

    vector<string> parseQuestionIds(string input) {
        replace(input.begin(), input.end(), ',', ' ');
        vector<string> ids;
        stringstream ss(input);
        string id;
        while (ss >> id) ids.push_back(id);
        return ids;
    }

    bool parseAnswerKey(string input, vector<char>& answerKey, string& errorMessage) {
        answerKey.clear();
        string normalized = input;
        for (char& ch : normalized) {
            if (ch == ',' || ch == ';' || ch == '|' || ch == '/') {
                ch = ' ';
            }
        }

        bool hasSeparator = normalized.find(' ') != string::npos ||
                            normalized.find('\t') != string::npos ||
                            normalized.find('\r') != string::npos ||
                            normalized.find('\n') != string::npos;

        if (!hasSeparator) {
            for (char ch : normalized) {
                if (isspace((unsigned char)ch)) {
                    continue;
                }
                char answer = (char)toupper((unsigned char)ch);
                if (answer < 'A' || answer > 'D') {
                    errorMessage = "Đáp án file đề chi duoc gom A, B, C, D. Vi du: A C B B A hoac ACBBA.";
                    return false;
                }
                answerKey.push_back(answer);
            }
        } else {
            stringstream ss(normalized);
            string token;
            while (ss >> token) {
                if (token.size() != 1) {
                    errorMessage = "Moi dap an phai la mot ky tu A, B, C hoac D. Vi du: A C B B A.";
                    return false;
                }
                char answer = (char)toupper((unsigned char)token[0]);
                if (answer < 'A' || answer > 'D') {
                    errorMessage = "Đáp án file đề chi duoc gom A, B, C, D. Vi du: A C B B A.";
                    return false;
                }
                answerKey.push_back(answer);
            }
        }

        if (answerKey.empty()) {
            errorMessage = "Vui lòng nhập ít nhất một đáp án cho file đề.";
            return false;
        }
        if (answerKey.size() > 60) {
            errorMessage = "Ban hien tai ho tro toi da 60 cau cho de file goc. Hay chia de thanh nhieu phan neu dai hon.";
            return false;
        }

        return true;
    }

    string lowerText(string text) {
        transform(text.begin(), text.end(), text.begin(),
                  [](unsigned char ch) { return (char)tolower(ch); });
        return text;
    }

    string normalizeClassName(string text) {
        while (!text.empty() && isspace((unsigned char)text.front())) {
            text.erase(text.begin());
        }
        while (!text.empty() && isspace((unsigned char)text.back())) {
            text.pop_back();
        }

        transform(text.begin(), text.end(), text.begin(),
                  [](unsigned char ch) { return (char)toupper(ch); });
        return text;
    }

    bool containsRoleKeyword(string text) {
        string value = lowerText(text);
        return value.find("admin") != string::npos ||
               value.find("teacher") != string::npos ||
               value.find("student") != string::npos;
    }

    string appDirectory() {
        char path[MAX_PATH] = "";
        GetModuleFileNameA(nullptr, path, MAX_PATH);
        string fullPath = path;
        size_t slash = fullPath.find_last_of("\\/");
        if (slash == string::npos) {
            return ".";
        }
        return fullPath.substr(0, slash);
    }

    bool isDateTimeText(string text) {
        if (text.size() != 16) {
            return false;
        }

        const string pattern = "0000-00-00 00:00";
        for (int i = 0; i < (int)text.size(); i++) {
            if (pattern[i] == '0') {
                if (!isdigit((unsigned char)text[i])) {
                    return false;
                }
            } else if (text[i] != pattern[i]) {
                return false;
            }
        }

        int month = atoi(text.substr(5, 2).c_str());
        int day = atoi(text.substr(8, 2).c_str());
        int hour = atoi(text.substr(11, 2).c_str());
        int minute = atoi(text.substr(14, 2).c_str());

        return month >= 1 && month <= 12 &&
               day >= 1 && day <= 31 &&
               hour >= 0 && hour <= 23 &&
               minute >= 0 && minute <= 59;
    }

    void showCurrentDashboard() {
        if (currentUser == nullptr) {
            showMainScreen();
        } else if (currentUser->getRole() == "admin") {
            showAdminDashboard();
        } else if (currentUser->getRole() == "teacher") {
            showTeacherDashboard();
        } else {
            showStudentDashboard();
        }
    }
};

int RunQuizApp(HINSTANCE hInstance) {
    GuiApp app(hInstance);
    return app.run();
}
