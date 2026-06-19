#pragma once

#include "../Common.h"

class User {
protected:
    string userId;
    string name;
    string email;
    string passwordHash;
    string role;
    string className;

public:
    User(string userId, string name, string email, string password, string role, string className);
    virtual ~User();

    string getUserId() const;
    string getName() const;
    string getEmail() const;
    string getPassword() const;
    string getPasswordHash() const;
    string getRole() const;
    string getClassName() const;

    void setName(string value);
    void setEmail(string value);
    void setPassword(string value);
    void setRole(string value);
    void setClassName(string value);

    bool checkPassword(const string& inputPassword) const;
    virtual string getInfoLine() const;
};

class Student : public User {
public:
    Student(string userId, string name, string email, string password, string className);
};

class Admin : public User {
public:
    Admin(string userId, string name, string email, string password);
};

class Teacher : public User {
public:
    Teacher(string userId, string name, string email, string password, string className);
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
             string imagePath = "", string imageUrl = "");

    string getQuestionId() const;
    string getContent() const;
    const array<string, 4>& getOptions() const;
    char getCorrectAnswer() const;
    string getSubject() const;
    string getDifficulty() const;
    string getImagePath() const;
    string getImageUrl() const;
    bool hasImage() const;

    void setContent(string value);
    void setOptions(array<string, 4> value);
    void setCorrectAnswer(char value);
    void setSubject(string value);
    void setDifficulty(string value);
    void setImagePath(string value);
    void setImageUrl(string value);

    string toText(bool showAnswer) const;
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
         int violationLimit, bool shuffleQuestions, bool shuffleAnswers, bool open);

    string getExamId() const;
    string getTitle() const;
    int getDurationMinutes() const;
    const vector<string>& getQuestionIds() const;
    string getCreatedBy() const;
    string getStartAt() const;
    string getCloseAt() const;
    string getExamPassword() const;
    string getAttachmentPath() const;
    string getAttachmentUrl() const;
    const vector<char>& getAnswerKey() const;
    int getAttemptLimit() const;
    int getViolationLimit() const;
    bool shouldShuffleQuestions() const;
    bool shouldShuffleAnswers() const;
    bool isOpen() const;
    bool hasPassword() const;
    bool hasAttachment() const;
    bool isFileAnswerExam() const;
    int totalQuestions() const;

    void setTitle(string value);
    void setDurationMinutes(int value);
    void setQuestionIds(vector<string> value);
    void setCreatedBy(string value);
    void setStartAt(string value);
    void setCloseAt(string value);
    void setExamPassword(string value);
    void setAttachmentPath(string value);
    void setAttachmentUrl(string value);
    void setAnswerKey(vector<char> value);
    void setAttemptLimit(int value);
    void setViolationLimit(int value);
    void setShuffleQuestions(bool value);
    void setShuffleAnswers(bool value);
    void setOpen(bool value);

    bool isBeforeStart(const string& currentTime) const;
    bool isClosedByTime(const string& currentTime) const;
    bool canTake(const string& currentTime) const;
    string statusText(const string& currentTime) const;
    bool checkExamPassword(const string& inputPassword) const;
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
                map<string, char> answers, string submittedAt, int score);

    string getSheetId() const;
    string getStudentId() const;
    string getExamId() const;
    const map<string, char>& getAnswers() const;
    string getSubmittedAt() const;
    int getScore() const;

    void setAnswers(map<string, char> value);
    void setSubmittedAt(string value);
    void setScore(int value);
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
           int totalQuestions, string submittedAt);

    string getResultId() const;
    string getStudentId() const;
    string getExamId() const;
    int getScore() const;
    int getTotalQuestions() const;
    string getSubmittedAt() const;

    void setScore(int value);
    void setTotalQuestions(int value);
    void setSubmittedAt(string value);
};
