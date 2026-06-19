#include "Models.h"

#include "../security/PasswordHasher.h"

User::User(string userId, string name, string email, string password, string role, string className)
    : userId(move(userId)),
      name(move(name)),
      email(move(email)),
      role(move(role)),
      className(move(className)) {
    setPassword(move(password));
}

User::~User() = default;

string User::getUserId() const { return userId; }
string User::getName() const { return name; }
string User::getEmail() const { return email; }
string User::getPassword() const { return passwordHash; }
string User::getPasswordHash() const { return passwordHash; }
string User::getRole() const { return role; }
string User::getClassName() const { return className; }

void User::setName(string value) { name = move(value); }
void User::setEmail(string value) { email = move(value); }
void User::setPassword(string value) { passwordHash = PasswordHasher::normalizePassword(value); }
void User::setRole(string value) { role = move(value); }
void User::setClassName(string value) { className = move(value); }

bool User::checkPassword(const string& inputPassword) const {
    return PasswordHasher::verifyPassword(inputPassword, passwordHash);
}

string User::getInfoLine() const {
    return userId + " | " + role + " | " + name + " | " + email + " | " + className;
}

Student::Student(string userId, string name, string email, string password, string className)
    : User(move(userId), move(name), move(email), move(password), "student", move(className)) {}

Admin::Admin(string userId, string name, string email, string password)
    : User(move(userId), move(name), move(email), move(password), "admin", "Admin") {}

Teacher::Teacher(string userId, string name, string email, string password, string className)
    : User(move(userId), move(name), move(email), move(password), "teacher", move(className)) {}

Question::Question(string questionId, string content, array<string, 4> options,
                   char correctAnswer, string subject, string difficulty,
                   string imagePath, string imageUrl)
    : questionId(move(questionId)),
      content(move(content)),
      options(move(options)),
      correctAnswer((char)toupper(correctAnswer)),
      subject(move(subject)),
      difficulty(move(difficulty)),
      imagePath(move(imagePath)),
      imageUrl(move(imageUrl)) {}

string Question::getQuestionId() const { return questionId; }
string Question::getContent() const { return content; }
const array<string, 4>& Question::getOptions() const { return options; }
char Question::getCorrectAnswer() const { return correctAnswer; }
string Question::getSubject() const { return subject; }
string Question::getDifficulty() const { return difficulty; }
string Question::getImagePath() const { return imagePath; }
string Question::getImageUrl() const { return imageUrl; }
bool Question::hasImage() const { return !imagePath.empty() || !imageUrl.empty(); }

void Question::setContent(string value) { content = move(value); }
void Question::setOptions(array<string, 4> value) { options = move(value); }
void Question::setCorrectAnswer(char value) { correctAnswer = (char)toupper(value); }
void Question::setSubject(string value) { subject = move(value); }
void Question::setDifficulty(string value) { difficulty = move(value); }
void Question::setImagePath(string value) { imagePath = move(value); }
void Question::setImageUrl(string value) { imageUrl = move(value); }

string Question::toText(bool showAnswer) const {
    stringstream stream;
    stream << questionId << ". " << content << " [" << subject << "]\r\n";
    for (int i = 0; i < 4; ++i) {
        stream << "   " << (char)('A' + i) << ". " << options[i] << "\r\n";
    }
    if (showAnswer) {
        stream << "   Dap an dung: " << correctAnswer << "\r\n";
    }
    if (hasImage()) {
        stream << "   Hinh anh: Co\r\n";
    }
    stream << "\r\n";
    return stream.str();
}

Exam::Exam(string examId, string title, int durationMinutes, vector<string> questionIds,
           string createdBy, string startAt, string closeAt, string examPassword,
           string attachmentPath, string attachmentUrl, vector<char> answerKey, int attemptLimit,
           int violationLimit, bool shuffleQuestions, bool shuffleAnswers, bool open)
    : examId(move(examId)),
      title(move(title)),
      durationMinutes(durationMinutes),
      questionIds(move(questionIds)),
      createdBy(move(createdBy)),
      startAt(move(startAt)),
      closeAt(move(closeAt)),
      examPassword(move(examPassword)),
      attachmentPath(move(attachmentPath)),
      attachmentUrl(move(attachmentUrl)),
      answerKey(move(answerKey)),
      attemptLimit(attemptLimit),
      violationLimit(violationLimit),
      shuffleQuestions(shuffleQuestions),
      shuffleAnswers(shuffleAnswers),
      open(open) {}

string Exam::getExamId() const { return examId; }
string Exam::getTitle() const { return title; }
int Exam::getDurationMinutes() const { return durationMinutes; }
const vector<string>& Exam::getQuestionIds() const { return questionIds; }
string Exam::getCreatedBy() const { return createdBy; }
string Exam::getStartAt() const { return startAt; }
string Exam::getCloseAt() const { return closeAt; }
string Exam::getExamPassword() const { return examPassword; }
string Exam::getAttachmentPath() const { return attachmentPath; }
string Exam::getAttachmentUrl() const { return attachmentUrl; }
const vector<char>& Exam::getAnswerKey() const { return answerKey; }
int Exam::getAttemptLimit() const { return attemptLimit; }
int Exam::getViolationLimit() const { return violationLimit; }
bool Exam::shouldShuffleQuestions() const { return shuffleQuestions; }
bool Exam::shouldShuffleAnswers() const { return shuffleAnswers; }
bool Exam::isOpen() const { return open; }
bool Exam::hasPassword() const { return !examPassword.empty(); }
bool Exam::hasAttachment() const { return !attachmentPath.empty() || !attachmentUrl.empty(); }
bool Exam::isFileAnswerExam() const { return questionIds.empty() && !answerKey.empty(); }
int Exam::totalQuestions() const { return isFileAnswerExam() ? (int)answerKey.size() : (int)questionIds.size(); }

void Exam::setTitle(string value) { title = move(value); }
void Exam::setDurationMinutes(int value) { durationMinutes = value; }
void Exam::setQuestionIds(vector<string> value) { questionIds = move(value); }
void Exam::setCreatedBy(string value) { createdBy = move(value); }
void Exam::setStartAt(string value) { startAt = move(value); }
void Exam::setCloseAt(string value) { closeAt = move(value); }
void Exam::setExamPassword(string value) { examPassword = move(value); }
void Exam::setAttachmentPath(string value) { attachmentPath = move(value); }
void Exam::setAttachmentUrl(string value) { attachmentUrl = move(value); }
void Exam::setAnswerKey(vector<char> value) { answerKey = move(value); }
void Exam::setAttemptLimit(int value) { attemptLimit = value; }
void Exam::setViolationLimit(int value) { violationLimit = value; }
void Exam::setShuffleQuestions(bool value) { shuffleQuestions = value; }
void Exam::setShuffleAnswers(bool value) { shuffleAnswers = value; }
void Exam::setOpen(bool value) { open = value; }

bool Exam::isBeforeStart(const string& currentTime) const {
    return !startAt.empty() && currentTime < startAt;
}

bool Exam::isClosedByTime(const string& currentTime) const {
    return !closeAt.empty() && currentTime > closeAt;
}

bool Exam::canTake(const string& currentTime) const {
    return open && !isBeforeStart(currentTime) && !isClosedByTime(currentTime);
}

string Exam::statusText(const string& currentTime) const {
    if (!open) return "DONG";
    if (isBeforeStart(currentTime)) return "CHUA_MO";
    if (isClosedByTime(currentTime)) return "HET_HAN";
    return "OPEN";
}

bool Exam::checkExamPassword(const string& inputPassword) const {
    return examPassword.empty() || examPassword == inputPassword;
}

AnswerSheet::AnswerSheet(string sheetId, string studentId, string examId,
                         map<string, char> answers, string submittedAt, int score)
    : sheetId(move(sheetId)),
      studentId(move(studentId)),
      examId(move(examId)),
      answers(move(answers)),
      submittedAt(move(submittedAt)),
      score(score) {}

string AnswerSheet::getSheetId() const { return sheetId; }
string AnswerSheet::getStudentId() const { return studentId; }
string AnswerSheet::getExamId() const { return examId; }
const map<string, char>& AnswerSheet::getAnswers() const { return answers; }
string AnswerSheet::getSubmittedAt() const { return submittedAt; }
int AnswerSheet::getScore() const { return score; }

void AnswerSheet::setAnswers(map<string, char> value) { answers = move(value); }
void AnswerSheet::setSubmittedAt(string value) { submittedAt = move(value); }
void AnswerSheet::setScore(int value) { score = value; }

Result::Result(string resultId, string studentId, string examId, int score,
               int totalQuestions, string submittedAt)
    : resultId(move(resultId)),
      studentId(move(studentId)),
      examId(move(examId)),
      score(score),
      totalQuestions(totalQuestions),
      submittedAt(move(submittedAt)) {}

string Result::getResultId() const { return resultId; }
string Result::getStudentId() const { return studentId; }
string Result::getExamId() const { return examId; }
int Result::getScore() const { return score; }
int Result::getTotalQuestions() const { return totalQuestions; }
string Result::getSubmittedAt() const { return submittedAt; }

void Result::setScore(int value) { score = value; }
void Result::setTotalQuestions(int value) { totalQuestions = value; }
void Result::setSubmittedAt(string value) { submittedAt = move(value); }
