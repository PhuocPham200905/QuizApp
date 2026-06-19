#pragma once

#include "../Common.h"

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
    int violationLimit;
    bool shuffleQuestions;
    bool shuffleAnswers;
    bool open;

public:
    Exam(string examId, string title, int durationMinutes, vector<string> questionIds,
         string createdBy, string startAt, string closeAt, string examPassword,
         string attachmentPath, string attachmentUrl, vector<char> answerKey, int attemptLimit,
         int violationLimit, bool shuffleQuestions, bool shuffleAnswers, bool open)
        : examId(examId), title(title), durationMinutes(durationMinutes),
          questionIds(questionIds), createdBy(createdBy), startAt(startAt), closeAt(closeAt),
          examPassword(examPassword), attachmentPath(attachmentPath), attachmentUrl(attachmentUrl),
          answerKey(answerKey), attemptLimit(attemptLimit), violationLimit(violationLimit),
          shuffleQuestions(shuffleQuestions), shuffleAnswers(shuffleAnswers), open(open) {}

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
    int getViolationLimit() const { return violationLimit; }
    bool shouldShuffleQuestions() const { return shuffleQuestions; }
    bool shouldShuffleAnswers() const { return shuffleAnswers; }
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
    void setViolationLimit(int value) { violationLimit = value; }
    void setShuffleQuestions(bool value) { shuffleQuestions = value; }
    void setShuffleAnswers(bool value) { shuffleAnswers = value; }
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
