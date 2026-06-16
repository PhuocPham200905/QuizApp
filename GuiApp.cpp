#include <windows.h>
#include <commctrl.h>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

#include <algorithm>
#include <array>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

using namespace std;
using json = nlohmann::json;

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

public:
    Question(string questionId, string content, array<string, 4> options,
             char correctAnswer, string subject, string difficulty)
        : questionId(questionId), content(content), options(options),
          correctAnswer((char)toupper(correctAnswer)), subject(subject), difficulty(difficulty) {}

    string getQuestionId() const { return questionId; }
    string getContent() const { return content; }
    array<string, 4> getOptions() const { return options; }
    char getCorrectAnswer() const { return correctAnswer; }
    string getSubject() const { return subject; }
    string getDifficulty() const { return difficulty; }

    void setContent(string value) { content = value; }
    void setOptions(array<string, 4> value) { options = value; }
    void setCorrectAnswer(char value) { correctAnswer = (char)toupper(value); }
    void setSubject(string value) { subject = value; }
    void setDifficulty(string value) { difficulty = value; }

    string toText(bool showAnswer) const {
        stringstream ss;
        ss << questionId << ". " << content << " [" << subject << ", " << difficulty << "]\r\n";
        for (int i = 0; i < 4; i++) {
            ss << "   " << (char)('A' + i) << ". " << options[i] << "\r\n";
        }
        if (showAnswer) {
            ss << "   Dap an dung: " << correctAnswer << "\r\n";
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
    string closeAt;
    string examPassword;
    bool open;

public:
    Exam(string examId, string title, int durationMinutes, vector<string> questionIds,
         string createdBy, string closeAt, string examPassword, bool open)
        : examId(examId), title(title), durationMinutes(durationMinutes),
          questionIds(questionIds), createdBy(createdBy), closeAt(closeAt),
          examPassword(examPassword), open(open) {}

    string getExamId() const { return examId; }
    string getTitle() const { return title; }
    int getDurationMinutes() const { return durationMinutes; }
    vector<string> getQuestionIds() const { return questionIds; }
    string getCreatedBy() const { return createdBy; }
    string getCloseAt() const { return closeAt; }
    string getExamPassword() const { return examPassword; }
    bool isOpen() const { return open; }
    bool hasPassword() const { return !examPassword.empty(); }

    void setTitle(string value) { title = value; }
    void setDurationMinutes(int value) { durationMinutes = value; }
    void setQuestionIds(vector<string> value) { questionIds = value; }
    void setCreatedBy(string value) { createdBy = value; }
    void setCloseAt(string value) { closeAt = value; }
    void setExamPassword(string value) { examPassword = value; }
    void setOpen(bool value) { open = value; }

    bool isClosedByTime(string currentTime) const {
        return !closeAt.empty() && currentTime > closeAt;
    }

    bool canTake(string currentTime) const {
        return open && !isClosedByTime(currentTime);
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
            lastError = string("Khong doc duoc JSON Firestore: ") + error.what();
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
            lastError = "Khong khoi tao duoc libcurl.";
            return false;
        }

        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

        if (method == "PATCH") {
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PATCH");
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
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
    string firebaseStatus = "Dang dung du lieu mau trong RAM.";

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
        users.push_back(make_shared<Student>(createStudentId(), "Nguyen An", "an@quiz.local", "123456", "12A1"));

        questions.push_back(Question(createQuestionId(), "C++ la ngon ngu lap trinh thuoc nhom nao?",
                                     {"Huong doi tuong", "Chi HTML", "He dieu hanh", "Co so du lieu"},
                                     'A', "C++", "Easy"));
        questions.push_back(Question(createQuestionId(), "Tu khoa nao dung de tao class trong C++?",
                                     {"structs", "class", "object", "newclass"},
                                     'B', "C++", "Easy"));
        questions.push_back(Question(createQuestionId(), "Tinh dong goi trong OOP giup dieu gi?",
                                     {"An chi tiet ben trong", "Tang kich thuoc file", "Xoa bien", "Tat chuong trinh"},
                                     'A', "OOP", "Medium"));
        questions.push_back(Question(createQuestionId(), "Vector trong C++ nam trong thu vien nao?",
                                     {"<map>", "<array>", "<vector>", "<string>"},
                                     'C', "C++", "Easy"));
        questions.push_back(Question(createQuestionId(), "Ke thua trong OOP duoc dung de lam gi?",
                                     {"Tai su dung va mo rong class", "Ma hoa mat khau", "Ket noi Wi-Fi", "Ve giao dien"},
                                     'A', "OOP", "Medium"));

        exams.push_back(Exam(createExamId(), "De mau C++ OOP", 30,
                             {"q1", "q2", "q3", "q4", "q5"}, "t1",
                             "2026-12-31 23:59", "oop123", true));
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

    void addStudent(string name, string email, string password, string className) {
        loadFromFirebase();
        string userId = createStudentId();
        users.push_back(make_shared<Student>(userId, name, email, password, className));
        saveUserToFirebase(userId, name, email, password, "student", className);
    }

    void addTeacher(string name, string email, string password, string className) {
        loadFromFirebase();
        string userId = createTeacherId();
        users.push_back(make_shared<Teacher>(userId, name, email, password, className));
        saveUserToFirebase(userId, name, email, password, "teacher", className);
    }

    void addQuestion(string content, array<string, 4> options, char correctAnswer, string subject, string difficulty) {
        string questionId = createQuestionId();
        questions.push_back(Question(questionId, content, options, correctAnswer, subject, difficulty));
        saveQuestionToFirebase(questionId, content, options, correctAnswer, subject, difficulty);
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

    void addExam(string title, int duration, vector<string> questionIds,
                 string createdBy, string closeAt, string examPassword) {
        string examId = createExamId();
        exams.push_back(Exam(examId, title, duration, questionIds,
                             createdBy, closeAt, examPassword, true));
        saveExamToFirebase(examId, title, duration, questionIds, createdBy, closeAt, examPassword, true);
    }

    int calculateScore(Exam& exam, map<string, char> answers) {
        int score = 0;
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
                                 score, (int)exam.getQuestionIds().size(), submittedAt));
        saveAnswerSheetToFirebase(sheetId, student->getUserId(), exam.getExamId(), answers, submittedAt, score);
        saveResultToFirebase(resultId, student->getUserId(), exam.getExamId(),
                             score, (int)exam.getQuestionIds().size(), submittedAt);
    }

    bool loadFromFirebase() {
        vector<shared_ptr<User>> loadedUsers;
        vector<Question> loadedQuestions;
        vector<Exam> loadedExams;

        json adminsJson;
        json teachersJson;
        json studentsJson;
        json questionsJson;
        json examsJson;

        if (!firebase.getCollection("admins", adminsJson) ||
            !firebase.getCollection("teachers", teachersJson) ||
            !firebase.getCollection("students", studentsJson) ||
            !firebase.getCollection("questions", questionsJson) ||
            !firebase.getCollection("exams", examsJson)) {
            connectedToFirebase = false;
            firebaseStatus = "Khong ket noi duoc Firestore. Dang dung du lieu mau trong RAM.\r\n" + firebase.getLastError();
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
                        firebase.stringField(fields, "difficulty")
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
                        firebase.stringField(fields, "closeAt"),
                        firebase.stringField(fields, "examPassword"),
                        firebase.boolField(fields, "isOpen", true)
                    ));
                }
            }

            if (!loadedUsers.empty()) users = loadedUsers;
            if (!loadedQuestions.empty()) questions = loadedQuestions;
            if (!loadedExams.empty()) exams = loadedExams;

            resetUserCounters();
            nextQuestionNumber = (int)questions.size() + 1;
            nextExamNumber = (int)exams.size() + 1;

            connectedToFirebase = true;
            firebaseStatus = "Da ket noi Firestore: quizapp-e586a";
            return true;
        } catch (const exception& error) {
            connectedToFirebase = false;
            firebaseStatus = string("Doc du lieu Firestore that bai. Dang dung RAM. Loi: ") + error.what();
            return false;
        }
    }

    void saveUserToFirebase(string userId, string name, string email, string password, string role, string className) {
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

        firebase.setDocument(collectionName, userId, fields);
    }

    void saveQuestionToFirebase(string questionId, string content, array<string, 4> options,
                                char correctAnswer, string subject, string difficulty) {
        json fields;
        fields["content"] = firebase.stringValue(content);
        fields["optionA"] = firebase.stringValue(options[0]);
        fields["optionB"] = firebase.stringValue(options[1]);
        fields["optionC"] = firebase.stringValue(options[2]);
        fields["optionD"] = firebase.stringValue(options[3]);
        fields["correctAnswer"] = firebase.stringValue(string(1, correctAnswer));
        fields["subject"] = firebase.stringValue(subject);
        fields["difficulty"] = firebase.stringValue(difficulty);
        firebase.setDocument("questions", questionId, fields);
    }

    void saveExamToFirebase(string examId, string title, int duration, vector<string> questionIds,
                            string createdBy, string closeAt, string examPassword, bool isOpen) {
        json fields;
        fields["title"] = firebase.stringValue(title);
        fields["durationMinutes"] = firebase.intValue(duration);
        fields["questionIds"] = firebase.stringArrayValue(questionIds);
        fields["createdBy"] = firebase.stringValue(createdBy);
        fields["closeAt"] = firebase.stringValue(closeAt);
        fields["examPassword"] = firebase.stringValue(examPassword);
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
    ID_TEACHER_DELETE_QUESTION,
    ID_TEACHER_EXAMS,
    ID_TEACHER_CREATE_EXAM,
    ID_TEACHER_TOGGLE_EXAM,
    ID_TEACHER_RESULTS,
    ID_STUDENT_OPEN_EXAMS,
    ID_STUDENT_TAKE_EXAM,
    ID_STUDENT_RESULTS,
    ID_CREATE_TEACHER_SUBMIT,
    ID_ADD_QUESTION_SUBMIT,
    ID_DELETE_QUESTION_SUBMIT,
    ID_CREATE_EXAM_SUBMIT,
    ID_TOGGLE_EXAM_SUBMIT,
    ID_TAKE_EXAM_SUBMIT,
    ID_DASHBOARD
};

enum AppScreen {
    SCREEN_MAIN,
    SCREEN_LOGIN,
    SCREEN_REGISTER,
    SCREEN_ADMIN_DASHBOARD,
    SCREEN_TEACHER_DASHBOARD,
    SCREEN_STUDENT_DASHBOARD,
    SCREEN_CREATE_TEACHER,
    SCREEN_ADD_QUESTION,
    SCREEN_DELETE_QUESTION,
    SCREEN_CREATE_EXAM,
    SCREEN_TOGGLE_EXAM,
    SCREEN_TAKE_EXAM_SELECT,
    SCREEN_TAKE_EXAM_FORM,
    SCREEN_VIEW
};

class GuiApp {
private:
    HINSTANCE instance;
    HWND window = nullptr;
    HFONT font = nullptr;
    QuizData data;
    shared_ptr<User> currentUser = nullptr;
    vector<HWND> controls;
    vector<array<HWND, 4>> answerOptions;
    vector<string> activeQuestionIds;
    string activeExamId;
    AppScreen currentScreen = SCREEN_MAIN;

public:
    GuiApp(HINSTANCE instance) : instance(instance) {}

    int run() {
        WNDCLASSA wc = {};
        wc.lpfnWndProc = GuiApp::windowProc;
        wc.hInstance = instance;
        wc.lpszClassName = "QuizAppGuiWindow";
        wc.hbrBackground = CreateSolidBrush(RGB(245, 247, 250));
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

        RegisterClassA(&wc);

        font = CreateFontA(18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                           DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                           CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");

        INITCOMMONCONTROLSEX controlsInfo = {};
        controlsInfo.dwSize = sizeof(controlsInfo);
        controlsInfo.dwICC = ICC_LISTVIEW_CLASSES;
        InitCommonControlsEx(&controlsInfo);

        window = CreateWindowA("QuizAppGuiWindow", "Quiz App - OOP Core GUI",
                               WS_OVERLAPPEDWINDOW,
                               CW_USEDEFAULT, CW_USEDEFAULT, 980, 680,
                               nullptr, nullptr, instance, this);

        ShowWindow(window, SW_SHOW);
        UpdateWindow(window);
        showMainScreen();

        MSG msg = {};
        while (GetMessage(&msg, nullptr, 0, 0)) {
            if (handleShortcut(msg)) {
                continue;
            }

            if (!IsDialogMessageA(window, &msg)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }

        return 0;
    }

    LRESULT handleMessage(UINT message, WPARAM wParam, LPARAM lParam) {
        if (message == WM_COMMAND) {
            handleCommand(LOWORD(wParam));
            return 0;
        }

        if (message == WM_DESTROY) {
            PostQuitMessage(0);
            return 0;
        }

        return DefWindowProcA(window, message, wParam, lParam);
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

        return DefWindowProcA(hwnd, message, wParam, lParam);
    }

    void handleCommand(int id) {
        if (id == ID_MAIN_REGISTER) showRegisterScreen();
        else if (id == ID_MAIN_LOGIN) showLoginScreen();
        else if (id == ID_BACK_MAIN) showMainScreen();
        else if (id == ID_LOGIN_SUBMIT) submitLogin();
        else if (id == ID_REGISTER_SUBMIT) submitRegisterStudent();
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
        else if (id == ID_TEACHER_DELETE_QUESTION) showDeleteQuestion();
        else if (id == ID_TEACHER_EXAMS) showExams();
        else if (id == ID_TEACHER_CREATE_EXAM) showCreateExam();
        else if (id == ID_TEACHER_TOGGLE_EXAM) showToggleExam();
        else if (id == ID_TEACHER_RESULTS) showAllResults();
        else if (id == ID_STUDENT_OPEN_EXAMS) showOpenExams();
        else if (id == ID_STUDENT_TAKE_EXAM) showTakeExamSelect();
        else if (id == ID_STUDENT_RESULTS) showMyResults();
        else if (id == ID_CREATE_TEACHER_SUBMIT) submitCreateTeacher();
        else if (id == ID_ADD_QUESTION_SUBMIT) submitAddQuestion();
        else if (id == ID_DELETE_QUESTION_SUBMIT) submitDeleteQuestion();
        else if (id == ID_CREATE_EXAM_SUBMIT) submitCreateExam();
        else if (id == ID_TOGGLE_EXAM_SUBMIT) submitToggleExam();
        else if (id == ID_TAKE_EXAM_SUBMIT) submitTakeExam();
        else if (id == ID_DASHBOARD) showCurrentDashboard();
        else if (id == IDCANCEL) DestroyWindow(window);
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
        answerOptions.clear();
        activeQuestionIds.clear();
        activeExamId.clear();
        for (HWND control : controls) {
            DestroyWindow(control);
        }
        controls.clear();
    }

    HWND addControl(string cls, string text, DWORD style, int x, int y, int w, int h, int id = 0, bool tabStop = false) {
        DWORD baseStyle = WS_CHILD | WS_VISIBLE;
        if (tabStop) {
            baseStyle |= WS_TABSTOP;
        }

        HWND control = CreateWindowExA(0, cls.c_str(), text.c_str(),
                                      baseStyle | style,
                                      x, y, w, h, window, (HMENU)(INT_PTR)id, instance, nullptr);
        SendMessageA(control, WM_SETFONT, (WPARAM)font, TRUE);
        controls.push_back(control);
        return control;
    }

    HWND label(string text, int x, int y, int w, int h) {
        return addControl("STATIC", text, 0, x, y, w, h, 0, false);
    }

    HWND button(string text, int x, int y, int w, int h, int id) {
        return addControl("BUTTON", text, BS_PUSHBUTTON, x, y, w, h, id, true);
    }

    HWND defaultButton(string text, int x, int y, int w, int h, int id) {
        return addControl("BUTTON", text, BS_DEFPUSHBUTTON, x, y, w, h, id, true);
    }

    HWND radioButton(string text, int x, int y, int w, int h, int id, bool startGroup) {
        DWORD style = BS_AUTORADIOBUTTON;
        if (startGroup) {
            style |= WS_GROUP;
        }
        return addControl("BUTTON", text, style, x, y, w, h, id, true);
    }

    HWND edit(string text, int x, int y, int w, int h, int id, bool password = false, bool multi = false, bool readOnly = false) {
        DWORD style = WS_BORDER | ES_AUTOHSCROLL;
        if (password) style |= ES_PASSWORD;
        if (multi) style = WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL;
        if (readOnly) style |= ES_READONLY;
        return addControl("EDIT", text, style, x, y, w, h, id, !readOnly);
    }

    HWND examListView(int x, int y, int w, int h, bool onlyOpen) {
        HWND list = CreateWindowExA(WS_EX_CLIENTEDGE, WC_LISTVIEWA, "",
                                   WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
                                   x, y, w, h, window, nullptr, instance, nullptr);
        SendMessageA(list, WM_SETFONT, (WPARAM)font, TRUE);
        ListView_SetExtendedListViewStyle(list, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_DOUBLEBUFFER);
        controls.push_back(list);

        addListColumn(list, 0, "ID", 45);
        addListColumn(list, 1, "Ten de", 150);
        addListColumn(list, 2, "Lam bai", 72);
        addListColumn(list, 3, "Dong luc", 125);
        addListColumn(list, 4, "So cau", 58);
        addListColumn(list, 5, "GV up", 110);
        addListColumn(list, 6, "MK", 42);
        addListColumn(list, 7, "Trang thai", 78);

        int row = 0;
        for (Exam& exam : data.getExams()) {
            string now = QuizData::currentMinuteText();
            if (onlyOpen && !exam.canTake(now)) {
                continue;
            }

            string status = exam.canTake(now) ? "OPEN" : (exam.isClosedByTime(now) ? "CLOSED_TIME" : "CLOSED");
            vector<string> values = {
                exam.getExamId(),
                exam.getTitle(),
                to_string(exam.getDurationMinutes()) + " phut",
                exam.getCloseAt(),
                to_string(exam.getQuestionIds().size()),
                data.getUserNameById(exam.getCreatedBy()),
                exam.hasPassword() ? "Co" : "Khong",
                status
            };

            LVITEMA item = {};
            item.mask = LVIF_TEXT;
            item.iItem = row;
            item.iSubItem = 0;
            item.pszText = const_cast<char*>(values[0].c_str());
            ListView_InsertItem(list, &item);

            for (int col = 1; col < (int)values.size(); col++) {
                ListView_SetItemText(list, row, col, const_cast<char*>(values[col].c_str()));
            }
            row++;
        }

        return list;
    }

    void addListColumn(HWND list, int index, string title, int width) {
        LVCOLUMNA column = {};
        column.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
        column.pszText = const_cast<char*>(title.c_str());
        column.cx = width;
        column.iSubItem = index;
        ListView_InsertColumn(list, index, &column);
    }

    string getText(int id) {
        HWND control = GetDlgItem(window, id);
        int len = GetWindowTextLengthA(control);
        string text(len, '\0');
        GetWindowTextA(control, text.data(), len + 1);
        return text;
    }

    void setFocusTo(int id) {
        HWND control = GetDlgItem(window, id);
        if (control != nullptr) {
            SetFocus(control);
        }
    }

    void message(string text, string title = "Quiz App") {
        MessageBoxA(window, text.c_str(), title.c_str(), MB_OK | MB_ICONINFORMATION);
    }

    void error(string text) {
        MessageBoxA(window, text.c_str(), "Loi", MB_OK | MB_ICONERROR);
    }

    void showShortcutHelp() {
        string text =
            "Phim tat trong app:\r\n\r\n"
            "Tab: Chuyen sang o/nut tiep theo\r\n"
            "Shift + Tab: Quay lai o/nut truoc\r\n"
            "Enter: Thuc hien nut chinh cua man hinh hien tai\r\n"
            "Esc: Quay lai dashboard hoac man hinh chinh\r\n"
            "Delete: Xoa cau hoi khi dang o man hinh Xoa cau hoi\r\n"
            "Ctrl + L: Dang xuat nhanh\r\n"
            "Ctrl + Q: Thoat app\r\n"
            "F1: Xem bang phim tat nay";
        MessageBoxA(window, text.c_str(), "Phim tat", MB_OK | MB_ICONINFORMATION);
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
            case SCREEN_CREATE_TEACHER:
                submitCreateTeacher();
                return true;
            case SCREEN_ADD_QUESTION:
                submitAddQuestion();
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
        label("QUIZ APP", 28, 20, 220, 28);
        HWND titleLabel = label(text, 280, 24, 640, 34);
        SendMessageA(titleLabel, WM_SETFONT, (WPARAM)font, TRUE);
        if (!subtitle.empty()) {
            label(subtitle, 282, 62, 640, 24);
        }
    }

    void showMainScreen() {
        clearControls();
        currentScreen = SCREEN_MAIN;
        title("Man hinh chinh");
        button("Dang ki hoc sinh", 360, 150, 260, 44, ID_MAIN_REGISTER);
        button("Dang nhap", 360, 210, 260, 44, ID_MAIN_LOGIN);
        button("Thoat", 360, 270, 260, 44, IDCANCEL);
    }

    void showSampleAccounts() {
        clearControls();
        title("Tai khoan mau", "Dung de test nhanh du lieu trong RAM.");
        string text =
            "Admin:   admin@quiz.local / 123456\r\n"
            "Teacher: teacher@quiz.local / 123456\r\n"
            "Student: an@quiz.local / 123456\r\n\r\n"
            "Hoc sinh duoc dang ki o man hinh chinh.\r\n"
            "Admin co san trong he thong.\r\n"
            "Giao vien do admin tao.";
        edit(text, 280, 120, 520, 220, 0, false, true, true);
        button("Quay lai", 280, 370, 150, 40, ID_BACK_MAIN);
    }

    void showLoginScreen() {
        clearControls();
        currentScreen = SCREEN_LOGIN;
        title("Dang nhap", "Nhap email va mat khau.");
        label("Email", 320, 150, 120, 26);
        edit("", 450, 146, 260, 30, 2001);
        label("Mat khau", 320, 200, 120, 26);
        edit("", 450, 196, 260, 30, 2002, true);
        defaultButton("Dang nhap", 450, 250, 125, 38, ID_LOGIN_SUBMIT);
        button("Quay lai", 585, 250, 125, 38, ID_BACK_MAIN);
        setFocusTo(2001);
    }

    void showRegisterScreen() {
        clearControls();
        currentScreen = SCREEN_REGISTER;
        title("Dang ki hoc sinh", "Chi hoc sinh duoc tu dang ki.");
        label("Ho ten", 300, 130, 130, 26);
        edit("", 450, 126, 280, 30, 2101);
        label("Email", 300, 175, 130, 26);
        edit("", 450, 171, 280, 30, 2102);
        label("Mat khau", 300, 220, 130, 26);
        edit("", 450, 216, 280, 30, 2103, true);
        label("Lop", 300, 265, 130, 26);
        edit("", 450, 261, 280, 30, 2104);
        defaultButton("Dang ki", 450, 320, 125, 38, ID_REGISTER_SUBMIT);
        button("Quay lai", 585, 320, 125, 38, ID_BACK_MAIN);
        setFocusTo(2101);
    }

    void submitLogin() {
        string email = getText(2001);
        string password = getText(2002);
        shared_ptr<User> user = data.findUserByEmail(email);
        if (user == nullptr || !user->checkPassword(password)) {
            error("Sai email hoac mat khau.");
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
        string className = getText(2104);

        if (name.empty() || email.empty() || password.empty() || className.empty()) {
            error("Vui long nhap day du thong tin.");
            return;
        }

        data.loadFromFirebase();
        if (data.findUserByEmail(email) != nullptr) {
            error("Email nay da ton tai.");
            return;
        }

        data.addStudent(name, email, password, className);
        message("Dang ki thanh cong. Ban co the dang nhap.");
        showLoginScreen();
    }

    void addNav(string roleTitle) {
        label(roleTitle, 26, 66, 220, 28);
        button("Dang xuat", 28, 560, 180, 38, ID_LOGOUT);
    }

    void showAdminDashboard() {
        clearControls();
        currentScreen = SCREEN_ADMIN_DASHBOARD;
        title("Admin Dashboard", "Xin chao " + currentUser->getName());
        addNav("Quan tri vien");
        button("Hoc sinh", 28, 110, 180, 36, ID_ADMIN_STUDENTS);
        button("Giao vien", 28, 155, 180, 36, ID_ADMIN_TEACHERS);
        button("Tao giao vien", 28, 200, 180, 36, ID_ADMIN_CREATE_TEACHER);
        button("Cau hoi", 28, 245, 180, 36, ID_ADMIN_QUESTIONS);
        button("De thi", 28, 290, 180, 36, ID_ADMIN_EXAMS);
        button("Mo/dong de", 28, 335, 180, 36, ID_ADMIN_TOGGLE_EXAM);
        button("Ket qua", 28, 380, 180, 36, ID_ADMIN_RESULTS);
        edit("Admin quan ly hoc sinh, giao vien, trang thai de thi va ket qua.\r\nDe thi do giao vien up len.\r\n\r\n" + data.getFirebaseStatus(),
             280, 120, 600, 220, 0, false, true, true);
    }

    void showTeacherDashboard() {
        clearControls();
        currentScreen = SCREEN_TEACHER_DASHBOARD;
        title("Teacher Dashboard", "Xin chao " + currentUser->getName());
        addNav("Giao vien");
        button("Cau hoi", 28, 110, 180, 36, ID_TEACHER_QUESTIONS);
        button("Them cau hoi", 28, 155, 180, 36, ID_TEACHER_ADD_QUESTION);
        button("Xoa cau hoi", 28, 200, 180, 36, ID_TEACHER_DELETE_QUESTION);
        button("De thi", 28, 245, 180, 36, ID_TEACHER_EXAMS);
        button("Up de thi", 28, 290, 180, 36, ID_TEACHER_CREATE_EXAM);
        button("Mo/dong de", 28, 335, 180, 36, ID_TEACHER_TOGGLE_EXAM);
        button("Ket qua", 28, 380, 180, 36, ID_TEACHER_RESULTS);
        edit("Giao vien tao cau hoi, up de thi, dat thoi gian dong de va mat khau neu can.\r\n\r\n" + data.getFirebaseStatus(),
             280, 120, 600, 220, 0, false, true, true);
    }

    void showStudentDashboard() {
        clearControls();
        currentScreen = SCREEN_STUDENT_DASHBOARD;
        title("Student Dashboard", "Xin chao " + currentUser->getName());
        addNav("Hoc sinh");
        button("De dang mo", 28, 110, 180, 36, ID_STUDENT_OPEN_EXAMS);
        button("Lam bai", 28, 155, 180, 36, ID_STUDENT_TAKE_EXAM);
        button("Lich su diem", 28, 200, 180, 36, ID_STUDENT_RESULTS);
        edit("Hoc sinh xem de thi dang mo, lam bai va xem lich su diem.\r\n\r\n" + data.getFirebaseStatus(),
             280, 120, 600, 220, 0, false, true, true);
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
                ss << "ID     Ho ten                 Email\r\n";
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

        ss << "ID     Vai tro    Ho ten                 Email                       Lop/Phu trach\r\n";
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

    void showStudents() {
        clearControls();
        currentScreen = SCREEN_VIEW;
        title("Danh sach hoc sinh");
        addNav("Quan tri vien");
        edit(usersText("student"), 260, 100, 650, 460, 0, false, true, true);
        button("Ve dashboard", 28, 510, 180, 38, ID_DASHBOARD);
    }

    void showTeachers() {
        clearControls();
        currentScreen = SCREEN_VIEW;
        title("Danh sach giao vien");
        addNav("Quan tri vien");
        edit(usersText("teacher"), 260, 100, 650, 460, 0, false, true, true);
        button("Ve dashboard", 28, 510, 180, 38, ID_DASHBOARD);
    }

    void showCreateTeacher() {
        clearControls();
        currentScreen = SCREEN_CREATE_TEACHER;
        title("Tao tai khoan giao vien");
        label("Ho ten", 300, 125, 140, 26);
        edit("", 470, 121, 280, 30, 3001);
        label("Email", 300, 170, 140, 26);
        edit("", 470, 166, 280, 30, 3002);
        label("Mat khau", 300, 215, 140, 26);
        edit("", 470, 211, 280, 30, 3003, true);
        label("Lop/mon phu trach", 300, 260, 150, 26);
        edit("", 470, 256, 280, 30, 3004);
        defaultButton("Tao giao vien", 470, 315, 140, 38, ID_CREATE_TEACHER_SUBMIT);
        button("Ve dashboard", 620, 315, 140, 38, ID_DASHBOARD);
        setFocusTo(3001);
    }

    void submitCreateTeacher() {
        string name = getText(3001);
        string email = getText(3002);
        string password = getText(3003);
        string className = getText(3004);
        if (name.empty() || email.empty() || password.empty() || className.empty()) {
            error("Vui long nhap day du thong tin.");
            return;
        }

        data.loadFromFirebase();
        if (data.findUserByEmail(email) != nullptr) {
            error("Email nay da ton tai.");
            return;
        }
        data.addTeacher(name, email, password, className);
        message("Da tao tai khoan giao vien.");
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
        title("Question Bank");
        if (currentUser && currentUser->getRole() == "teacher") addNav("Giao vien");
        else addNav("Quan tri vien");
        edit(questionsText(showAnswer), 260, 100, 650, 460, 0, false, true, true);
        button("Ve dashboard", 28, 510, 180, 38, ID_DASHBOARD);
    }

    void showAddQuestion() {
        clearControls();
        currentScreen = SCREEN_ADD_QUESTION;
        title("Them cau hoi");
        int x1 = 280, x2 = 450, y = 100;
        label("Noi dung", x1, y, 140, 26);
        edit("", x2, y - 4, 390, 30, 4001);
        y += 42;
        label("Dap an A", x1, y, 140, 26);
        edit("", x2, y - 4, 390, 30, 4002);
        y += 42;
        label("Dap an B", x1, y, 140, 26);
        edit("", x2, y - 4, 390, 30, 4003);
        y += 42;
        label("Dap an C", x1, y, 140, 26);
        edit("", x2, y - 4, 390, 30, 4004);
        y += 42;
        label("Dap an D", x1, y, 140, 26);
        edit("", x2, y - 4, 390, 30, 4005);
        y += 42;
        label("Dap an dung", x1, y, 140, 26);
        edit("", x2, y - 4, 80, 30, 4006);
        y += 42;
        label("Mon hoc", x1, y, 140, 26);
        edit("", x2, y - 4, 220, 30, 4007);
        y += 42;
        label("Do kho", x1, y, 140, 26);
        edit("", x2, y - 4, 220, 30, 4008);
        defaultButton("Them", x2, y + 50, 130, 38, ID_ADD_QUESTION_SUBMIT);
        button("Ve dashboard", x2 + 145, y + 50, 150, 38, ID_DASHBOARD);
        setFocusTo(4001);
    }

    void submitAddQuestion() {
        string content = getText(4001);
        array<string, 4> options = {getText(4002), getText(4003), getText(4004), getText(4005)};
        string correct = getText(4006);
        string subject = getText(4007);
        string difficulty = getText(4008);
        if (content.empty() || options[0].empty() || options[1].empty() || options[2].empty() ||
            options[3].empty() || correct.empty() || subject.empty() || difficulty.empty()) {
            error("Vui long nhap day du thong tin cau hoi.");
            return;
        }
        char answer = (char)toupper(correct[0]);
        if (answer < 'A' || answer > 'D') {
            error("Dap an dung phai la A, B, C hoac D.");
            return;
        }
        data.addQuestion(content, options, answer, subject, difficulty);
        message("Da them cau hoi.");
        showTeacherDashboard();
    }

    void showDeleteQuestion() {
        clearControls();
        currentScreen = SCREEN_DELETE_QUESTION;
        title("Xoa cau hoi");
        edit(questionsText(true), 260, 92, 650, 300, 0, false, true, true);
        label("Ma cau hoi can xoa", 300, 420, 160, 26);
        edit("", 480, 416, 160, 30, 4101);
        defaultButton("Xoa", 480, 465, 100, 38, ID_DELETE_QUESTION_SUBMIT);
        button("Ve dashboard", 595, 465, 150, 38, ID_DASHBOARD);
        setFocusTo(4101);
    }

    void submitDeleteQuestion() {
        string id = getText(4101);
        if (data.deleteQuestion(id)) {
            message("Da xoa cau hoi.");
            showTeacherDashboard();
        } else {
            error("Khong tim thay cau hoi.");
        }
    }

    string examsText(bool onlyOpen = false) {
        stringstream ss;
        ss << "ID    Ten de                Lam bai   Dong luc          So cau  Giao vien up       MK   Trang thai\r\n";
        ss << "------------------------------------------------------------------------------------------------\r\n";
        for (Exam& exam : data.getExams()) {
            string now = QuizData::currentMinuteText();
            if (onlyOpen && !exam.canTake(now)) continue;
            string status = exam.canTake(now) ? "OPEN" : (exam.isClosedByTime(now) ? "CLOSED_TIME" : "CLOSED");
            ss << left << setw(6) << exam.getExamId()
               << setw(22) << exam.getTitle()
               << setw(10) << (to_string(exam.getDurationMinutes()) + " phut")
               << setw(18) << exam.getCloseAt()
               << setw(8) << exam.getQuestionIds().size()
               << setw(20) << data.getUserNameById(exam.getCreatedBy())
               << setw(5) << (exam.hasPassword() ? "Co" : "Khong")
               << status << "\r\n";
        }
        return ss.str();
    }

    void showExams() {
        clearControls();
        currentScreen = SCREEN_VIEW;
        title("Danh sach de thi");
        if (currentUser && currentUser->getRole() == "teacher") addNav("Giao vien");
        else addNav("Quan tri vien");
        examListView(250, 100, 680, 390, false);
        button("Ve dashboard", 28, 510, 180, 38, ID_DASHBOARD);
    }

    void showOpenExams() {
        clearControls();
        currentScreen = SCREEN_VIEW;
        title("De thi dang mo");
        addNav("Hoc sinh");
        examListView(250, 100, 680, 390, true);
        button("Ve dashboard", 28, 510, 180, 38, ID_DASHBOARD);
    }

    void showCreateExam() {
        clearControls();
        currentScreen = SCREEN_CREATE_EXAM;
        title("Giao vien up de thi", "Nhap Tab de chuyen nhanh giua cac o.");
        edit(questionsText(true), 260, 86, 650, 210, 0, false, true, true);
        label("Ten de thi", 290, 315, 170, 26);
        edit("", 500, 311, 280, 30, 5001);
        label("Thoi gian lam bai", 290, 355, 170, 26);
        edit("30", 500, 351, 100, 30, 5002);
        label("Dong luc", 290, 395, 170, 26);
        edit("2026-12-31 23:59", 500, 391, 190, 30, 5004);
        label("Ma cau hoi", 290, 435, 170, 26);
        edit("q1 q2 q3", 500, 431, 280, 30, 5003);
        label("Mat khau de", 290, 475, 170, 26);
        edit("", 500, 471, 190, 30, 5005);
        label("Bo trong neu de khong can mat khau", 700, 475, 220, 26);
        defaultButton("Tao de", 500, 535, 120, 38, ID_CREATE_EXAM_SUBMIT);
        button("Ve dashboard", 635, 535, 150, 38, ID_DASHBOARD);
        setFocusTo(5001);
    }

    void submitCreateExam() {
        string title = getText(5001);
        string durationText = getText(5002);
        string idsText = getText(5003);
        string closeAt = getText(5004);
        string examPassword = getText(5005);
        if (title.empty() || durationText.empty() || idsText.empty() || closeAt.empty()) {
            error("Vui long nhap day du thong tin de thi.");
            return;
        }

        int duration = atoi(durationText.c_str());
        if (duration <= 0) {
            error("Thoi gian lam bai phai lon hon 0.");
            return;
        }
        if (closeAt.size() != 16 || closeAt[4] != '-' || closeAt[7] != '-' || closeAt[10] != ' ' || closeAt[13] != ':') {
            error("Thoi gian dong de can theo dang YYYY-MM-DD HH:MM.");
            return;
        }

        vector<string> ids = parseQuestionIds(idsText);
        vector<string> validIds;
        for (string id : ids) {
            if (data.findQuestionById(id) != nullptr) validIds.push_back(id);
        }
        if (validIds.empty()) {
            error("Khong co ma cau hoi hop le.");
            return;
        }

        data.addExam(title, duration, validIds, currentUser->getUserId(), closeAt, examPassword);
        message("Da tao de thi.");
        showTeacherDashboard();
    }

    void showToggleExam() {
        clearControls();
        currentScreen = SCREEN_TOGGLE_EXAM;
        title("Mo/dong de thi");
        examListView(250, 90, 680, 330, false);
        label("Ma de thi", 300, 455, 140, 26);
        edit("", 460, 451, 150, 30, 5101);
        defaultButton("Doi trang thai", 460, 505, 140, 38, ID_TOGGLE_EXAM_SUBMIT);
        button("Ve dashboard", 615, 505, 150, 38, ID_DASHBOARD);
        setFocusTo(5101);
    }

    void submitToggleExam() {
        string examId = getText(5101);
        Exam* exam = data.findExamById(examId);
        if (exam == nullptr) {
            error("Khong tim thay de thi.");
            return;
        }
        exam->setOpen(!exam->isOpen());
        message("Da doi trang thai de thi.");
        if (currentUser->getRole() == "teacher") showTeacherDashboard();
        else showAdminDashboard();
    }

    void showTakeExamSelect() {
        clearControls();
        currentScreen = SCREEN_TAKE_EXAM_SELECT;
        title("Lam bai thi");
        examListView(250, 88, 680, 330, true);
        label("Ma de thi", 300, 455, 140, 26);
        edit("", 460, 451, 150, 30, 5201);
        label("Mat khau de", 300, 500, 140, 26);
        edit("", 460, 496, 150, 30, 5202, true);
        defaultButton("Bat dau", 635, 451, 120, 38, ID_TAKE_EXAM_SUBMIT);
        button("Ve dashboard", 635, 496, 150, 38, ID_DASHBOARD);
        setFocusTo(5201);
    }

    void showTakeExamForm(Exam& exam) {
        clearControls();
        currentScreen = SCREEN_TAKE_EXAM_FORM;
        activeExamId = exam.getExamId();
        title("Lam bai: " + exam.getTitle(),
              "Thoi gian lam bai: " + to_string(exam.getDurationMinutes()) +
              " phut | Dong luc: " + exam.getCloseAt());
        int y = 95;
        int index = 1;
        answerOptions.clear();
        activeQuestionIds.clear();

        for (string questionId : exam.getQuestionIds()) {
            Question* question = data.findQuestionById(questionId);
            if (question == nullptr || y > 470) continue;

            string line = to_string(index) + ". " + question->getContent();
            label(line, 260, y, 640, 24);
            y += 28;
            array<string, 4> options = question->getOptions();
            array<HWND, 4> buttons = {
                radioButton("A. " + options[0], 285, y, 285, 24, 5300 + index * 10 + 0, true),
                radioButton("B. " + options[1], 585, y, 285, 24, 5300 + index * 10 + 1, false),
                radioButton("C. " + options[2], 285, y + 26, 285, 24, 5300 + index * 10 + 2, false),
                radioButton("D. " + options[3], 585, y + 26, 285, 24, 5300 + index * 10 + 3, false),
            };
            answerOptions.push_back(buttons);
            activeQuestionIds.push_back(questionId);
            y += 62;
            index++;
        }

        defaultButton("Nop bai", 720, 560, 120, 38, ID_TAKE_EXAM_SUBMIT);
        button("Ve dashboard", 560, 560, 145, 38, ID_DASHBOARD);
        if (!answerOptions.empty()) {
            SetFocus(answerOptions[0][0]);
        }
    }

    void submitTakeExam() {
        if (activeExamId.empty()) {
            string examId = getText(5201);
            string examPassword = getText(5202);
            Exam* exam = data.findExamById(examId);
            if (exam == nullptr) {
                error("Khong tim thay de thi.");
                return;
            }
            if (!exam->canTake(QuizData::currentMinuteText())) {
                error("De thi khong ton tai hoac dang dong.");
                return;
            }
            if (!exam->checkExamPassword(examPassword)) {
                error("Mat khau de thi khong dung.");
                return;
            }
            showTakeExamForm(*exam);
            return;
        }

        Exam* exam = data.findExamById(activeExamId);
        if (exam == nullptr) {
            error("Khong tim thay de thi.");
            return;
        }

        map<string, char> answers;
        for (int i = 0; i < (int)answerOptions.size() && i < (int)activeQuestionIds.size(); i++) {
            char selectedAnswer = '\0';
            for (int optionIndex = 0; optionIndex < 4; optionIndex++) {
                if (SendMessageA(answerOptions[i][optionIndex], BM_GETCHECK, 0, 0) == BST_CHECKED) {
                    selectedAnswer = (char)('A' + optionIndex);
                    break;
                }
            }

            if (selectedAnswer == '\0') {
                error("Vui long chon dap an cho tat ca cau hoi.");
                return;
            }

            answers[activeQuestionIds[i]] = selectedAnswer;
        }

        data.submitExam(currentUser, *exam, answers);
        message("Da nop bai. Vao Lich su diem de xem ket qua.");
        showStudentDashboard();
    }

    string resultsText(bool onlyCurrentStudent) {
        stringstream ss;
        ss << "ID      Hoc sinh        De thi                    Diem      Thoi gian nop\r\n";
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

    void showAllResults() {
        clearControls();
        currentScreen = SCREEN_VIEW;
        title("Ket qua tat ca hoc sinh");
        edit(resultsText(false), 260, 100, 650, 460, 0, false, true, true);
        button("Ve dashboard", 28, 510, 180, 38, ID_DASHBOARD);
    }

    void showMyResults() {
        clearControls();
        currentScreen = SCREEN_VIEW;
        title("Lich su diem");
        edit(resultsText(true), 260, 100, 650, 460, 0, false, true, true);
        button("Ve dashboard", 28, 510, 180, 38, ID_DASHBOARD);
    }

    vector<string> parseQuestionIds(string input) {
        replace(input.begin(), input.end(), ',', ' ');
        vector<string> ids;
        stringstream ss(input);
        string id;
        while (ss >> id) ids.push_back(id);
        return ids;
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
