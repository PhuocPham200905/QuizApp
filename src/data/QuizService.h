#pragma once

#include "../Common.h"
#include "../models/Models.h"
#include "../security/PasswordHasher.h"
#include "QuizRepository.h"

class QuizService {
private:
    vector<shared_ptr<User>> users;
    vector<Question> questions;
    vector<Exam> exams;
    vector<AnswerSheet> answerSheets;
    vector<Result> results;
    QuizRepository firebase;
    bool connectedToFirebase = false;
    string firebaseStatus = "Đang dùng dữ liệu mẫu trong RAM.";

    int nextAdminNumber = 1;
    int nextTeacherNumber = 1;
    int nextStudentNumber = 1;
    int nextQuestionNumber = 1;
    int nextExamNumber = 1;
    int nextSheetNumber = 1;
    int nextResultNumber = 1;

    static bool shouldSkipFirebase() {
        const char* skipFirebase = getenv("QUIZAPP_SKIP_FIREBASE");
        return skipFirebase != nullptr && string(skipFirebase) == "1";
    }

public:
    QuizService() {
        seedSampleData();
        if (!shouldSkipFirebase()) {
            loadFromFirebase();
        }
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
                             "2026-01-01 00:00", "2026-12-31 23:59", "oop123", "", "", {}, 1,
                             3, false, false, true));
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
                     string subject, string difficulty = "Trung bình",
                     string imageSourcePath = "", string imageUrl = "") {
        string questionId = createQuestionId();
        string imagePath = prepareQuestionImage(questionId, imageSourcePath);
        questions.push_back(Question(questionId, content, options, correctAnswer, subject, difficulty, imagePath, imageUrl));
        saveQuestionToFirebase(questionId, content, options, correctAnswer, subject, difficulty, imagePath, imageUrl);
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
            string difficulty = columns.size() >= 7 && !columns[6].empty()
                                    ? columns[6]
                                    : "Trung bình";

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

            addQuestion(content, options, answer, subject, difficulty);
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
                 string attachmentSourcePath, string attachmentUrl, int attemptLimit,
                 int violationLimit, bool shuffleQuestions, bool shuffleAnswers) {
        string examId = createExamId();
        string attachmentPath = prepareExamAttachment(examId, attachmentSourcePath);
        exams.push_back(Exam(examId, title, duration, questionIds,
                             createdBy, startAt, closeAt, examPassword,
                             attachmentPath, attachmentUrl, answerKey, attemptLimit,
                             violationLimit, shuffleQuestions, shuffleAnswers, true));
        saveExamToFirebase(examId, title, duration, questionIds, createdBy, startAt, closeAt,
                           examPassword, attachmentPath, attachmentUrl, answerKey, attemptLimit,
                           violationLimit, shuffleQuestions, shuffleAnswers, true);
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
                           exam->getViolationLimit(), exam->shouldShuffleQuestions(),
                           exam->shouldShuffleAnswers(), exam->isOpen());
        return true;
    }

    string deviceId() const {
        wchar_t computerName[MAX_COMPUTERNAME_LENGTH + 1] = L"";
        DWORD size = MAX_COMPUTERNAME_LENGTH + 1;
        if (GetComputerNameW(computerName, &size)) {
            int utf8Size = WideCharToMultiByte(CP_UTF8, 0, computerName, -1,
                                               nullptr, 0, nullptr, nullptr);
            if (utf8Size > 1) {
                string value(utf8Size, '\0');
                WideCharToMultiByte(CP_UTF8, 0, computerName, -1,
                                    value.data(), utf8Size, nullptr, nullptr);
                value.resize(utf8Size - 1);
                return value;
            }
        }
        return "UNKNOWN_DEVICE";
    }

    string safeDocumentPart(string value) const {
        for (char& character : value) {
            unsigned char byte = (unsigned char)character;
            if (!isalnum(byte) && character != '-' && character != '_') {
                character = '_';
            }
        }
        return value;
    }

    void writeAuditLog(string userId, string examId, string action,
                       string details = "", string device = "") {
        static unsigned long counter = 0;
        long long milliseconds = chrono::duration_cast<chrono::milliseconds>(
                                     chrono::system_clock::now().time_since_epoch())
                                     .count();
        string logId = "log_" + to_string(milliseconds) + "_" + to_string(++counter);
        json fields;
        fields["userId"] = firebase.stringValue(userId);
        fields["examId"] = firebase.stringValue(examId);
        fields["action"] = firebase.stringValue(action);
        fields["details"] = firebase.stringValue(details);
        fields["deviceId"] = firebase.stringValue(device.empty() ? deviceId() : device);
        fields["createdAt"] = firebase.stringValue(currentTimeText());
        firebase.createDocument("auditLogs", logId, fields);
    }

    bool startExamSession(shared_ptr<User> student, Exam& exam, string& sessionId,
                          string& errorMessage) {
        int attemptNumber = countAttempts(student->getUserId(), exam.getExamId()) + 1;
        sessionId = "session_" + safeDocumentPart(student->getUserId()) + "_" +
                    safeDocumentPart(exam.getExamId()) + "_" + to_string(attemptNumber);

        json existingDocument;
        if (firebase.getDocument("examSessions", sessionId, existingDocument)) {
            const json& fields = existingDocument.value("fields", json::object());
            string status = firebase.stringField(fields, "status");
            string existingDevice = firebase.stringField(fields, "deviceId");
            if (status == "IN_PROGRESS" && existingDevice != deviceId()) {
                errorMessage = "Tài khoản đang làm bài trên thiết bị khác.";
            } else if (status == "IN_PROGRESS") {
                updateExamSession(sessionId, "INTERRUPTED",
                                  firebase.intField(fields, "violationCount", 0),
                                  "Phiên trước kết thúc bất thường");
                errorMessage = "Phiên thi trước kết thúc bất thường và đã bị khóa. Vui lòng nhờ admin mở khóa.";
            } else if (status == "INTERRUPTED") {
                errorMessage = "Lượt thi trước bị gián đoạn. Vui lòng nhờ admin mở khóa.";
            } else {
                errorMessage = "Phiên thi này đã tồn tại và không thể bắt đầu lại.";
            }
            writeAuditLog(student->getUserId(), exam.getExamId(), "SESSION_DENIED",
                          errorMessage);
            return false;
        }

        json fields;
        fields["studentId"] = firebase.stringValue(student->getUserId());
        fields["examId"] = firebase.stringValue(exam.getExamId());
        fields["attemptNumber"] = firebase.intValue(attemptNumber);
        fields["deviceId"] = firebase.stringValue(deviceId());
        fields["status"] = firebase.stringValue("IN_PROGRESS");
        fields["startedAt"] = firebase.stringValue(currentTimeText());
        fields["updatedAt"] = firebase.stringValue(currentTimeText());
        fields["violationCount"] = firebase.intValue(0);
        if (!firebase.createDocument("examSessions", sessionId, fields)) {
            errorMessage = "Không thể khóa phiên thi. Vui lòng kiểm tra kết nối mạng.";
            return false;
        }
        writeAuditLog(student->getUserId(), exam.getExamId(), "EXAM_STARTED",
                      "Bắt đầu lượt " + to_string(attemptNumber));
        return true;
    }

    void updateExamSession(string sessionId, string status, int violationCount,
                           string details = "") {
        if (sessionId.empty()) return;
        json document;
        json fields;
        if (firebase.getDocument("examSessions", sessionId, document) &&
            document.contains("fields")) {
            fields = document["fields"];
        }
        fields["status"] = firebase.stringValue(status);
        fields["updatedAt"] = firebase.stringValue(currentTimeText());
        fields["violationCount"] = firebase.intValue(violationCount);
        fields["details"] = firebase.stringValue(details);
        fields["deviceId"] = firebase.stringValue(deviceId());
        firebase.setDocument("examSessions", sessionId, fields);
    }

    bool unlockExamSession(string sessionId) {
        return !sessionId.empty() && firebase.deleteDocument("examSessions", sessionId);
    }

    json getAuditLogs() {
        json output;
        if (!firebase.getCollection("auditLogs", output)) {
            return json::object();
        }
        return output;
    }

    json getExamSessions() {
        json output;
        if (!firebase.getCollection("examSessions", output)) {
            return json::object();
        }
        return output;
    }

    bool updateExamDetails(string examId, string ownerId, string title, int duration,
                           string startAt, string closeAt, string examPassword,
                           string attachmentUrl, int attemptLimit, int violationLimit,
                           vector<string> questionIds,
                           vector<char> answerKey, bool shuffleQuestions,
                           bool shuffleAnswers) {
        Exam* exam = findExamById(examId);
        if (exam == nullptr || (!ownerId.empty() && exam->getCreatedBy() != ownerId)) {
            firebaseStatus = "Không tìm thấy đề thi thuộc quyền quản lý của giáo viên.";
            return false;
        }

        exam->setTitle(title);
        exam->setDurationMinutes(duration);
        exam->setStartAt(startAt);
        exam->setCloseAt(closeAt);
        exam->setExamPassword(examPassword);
        exam->setAttachmentUrl(attachmentUrl);
        exam->setAttemptLimit(attemptLimit);
        exam->setViolationLimit(violationLimit);
        exam->setQuestionIds(questionIds);
        exam->setAnswerKey(answerKey);
        exam->setShuffleQuestions(shuffleQuestions);
        exam->setShuffleAnswers(shuffleAnswers);
        saveExamToFirebase(exam->getExamId(), exam->getTitle(), exam->getDurationMinutes(),
                           exam->getQuestionIds(), exam->getCreatedBy(), exam->getStartAt(),
                           exam->getCloseAt(), exam->getExamPassword(), exam->getAttachmentPath(),
                           exam->getAttachmentUrl(), exam->getAnswerKey(), exam->getAttemptLimit(),
                           exam->getViolationLimit(), exam->shouldShuffleQuestions(),
                           exam->shouldShuffleAnswers(), exam->isOpen());
        return true;
    }

    bool deleteExam(string examId, string ownerId) {
        auto iterator = find_if(exams.begin(), exams.end(),
                                [examId, ownerId](const Exam& exam) {
                                    return exam.getExamId() == examId &&
                                           (ownerId.empty() || exam.getCreatedBy() == ownerId);
                                });
        if (iterator == exams.end()) {
            firebaseStatus = "Không tìm thấy đề thi thuộc quyền quản lý của giáo viên.";
            return false;
        }

        if (!firebase.deleteDocument("exams", examId)) {
            connectedToFirebase = false;
            firebaseStatus = "Không xóa được đề thi trên Firestore.\r\n" + firebase.getLastError();
            return false;
        }

        exams.erase(iterator);
        connectedToFirebase = true;
        firebaseStatus = "Dữ liệu đã sẵn sàng.";
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

    bool submitExam(shared_ptr<User> student, Exam& exam, map<string, char> answers) {
        int score = calculateScore(exam, answers);
        string submittedAt = currentTimeText();
        int attemptNumber = countAttempts(student->getUserId(), exam.getExamId()) + 1;
        string documentSuffix = student->getUserId() + "_" + exam.getExamId() + "_" +
                                to_string(attemptNumber);
        string sheetId = "sheet_" + documentSuffix;
        string resultId = "result_" + documentSuffix;
        if (!saveAnswerSheetToFirebase(sheetId, student->getUserId(), exam.getExamId(),
                                       answers, submittedAt, score) ||
            !saveResultToFirebase(resultId, student->getUserId(), exam.getExamId(),
                                  score, exam.totalQuestions(), submittedAt)) {
            firebaseStatus = "Không thể lưu bài làm theo chế độ chỉ tạo mới.";
            return false;
        }
        answerSheets.push_back(AnswerSheet(sheetId, student->getUserId(), exam.getExamId(),
                                           answers, submittedAt, score));
        results.push_back(Result(resultId, student->getUserId(), exam.getExamId(),
                                 score, exam.totalQuestions(), submittedAt));
        return true;
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
                        firebase.intField(fields, "violationLimit", 3),
                        firebase.boolField(fields, "shuffleQuestions", false),
                        firebase.boolField(fields, "shuffleAnswers", false),
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
        if (shouldSkipFirebase()) {
            return true;
        }

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
        fields["passwordHash"] = firebase.stringValue(PasswordHasher::normalizePassword(password));
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
                                char correctAnswer, string subject, string difficulty,
                                string imagePath, string imageUrl) {
        json fields;
        fields["content"] = firebase.stringValue(content);
        fields["optionA"] = firebase.stringValue(options[0]);
        fields["optionB"] = firebase.stringValue(options[1]);
        fields["optionC"] = firebase.stringValue(options[2]);
        fields["optionD"] = firebase.stringValue(options[3]);
        fields["correctAnswer"] = firebase.stringValue(string(1, correctAnswer));
        fields["subject"] = firebase.stringValue(subject);
        fields["difficulty"] = firebase.stringValue(difficulty);
        fields["imagePath"] = firebase.stringValue(imagePath);
        fields["imageUrl"] = firebase.stringValue(imageUrl);
        firebase.setDocument("questions", questionId, fields);
    }

    void saveExamToFirebase(string examId, string title, int duration, vector<string> questionIds,
                            string createdBy, string startAt, string closeAt, string examPassword,
                            string attachmentPath, string attachmentUrl, vector<char> answerKey,
                            int attemptLimit, int violationLimit, bool shuffleQuestions,
                            bool shuffleAnswers, bool isOpen) {
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
        fields["violationLimit"] = firebase.intValue(violationLimit);
        fields["shuffleQuestions"] = firebase.boolValue(shuffleQuestions);
        fields["shuffleAnswers"] = firebase.boolValue(shuffleAnswers);
        fields["isOpen"] = firebase.boolValue(isOpen);
        firebase.setDocument("exams", examId, fields);
    }

    bool saveAnswerSheetToFirebase(string sheetId, string studentId, string examId,
                                   map<string, char> answers, string submittedAt, int score) {
        json fields;
        fields["studentId"] = firebase.stringValue(studentId);
        fields["examId"] = firebase.stringValue(examId);
        fields["answers"] = firebase.answerMapValue(answers);
        fields["submittedAt"] = firebase.stringValue(submittedAt);
        fields["score"] = firebase.intValue(score);
        return firebase.createDocument("answerSheets", sheetId, fields);
    }

    bool saveResultToFirebase(string resultId, string studentId, string examId,
                              int score, int totalQuestions, string submittedAt) {
        json fields;
        fields["studentId"] = firebase.stringValue(studentId);
        fields["examId"] = firebase.stringValue(examId);
        fields["score"] = firebase.intValue(score);
        fields["totalQuestions"] = firebase.intValue(totalQuestions);
        fields["submittedAt"] = firebase.stringValue(submittedAt);
        return firebase.createDocument("results", resultId, fields);
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
