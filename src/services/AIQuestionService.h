#pragma once

#include "../Common.h"

struct AIQuestionRequest {
    string subject;
    string grade;
    string topic;
    string difficulty;
    int count = 5;
    string extraNotes;
};

struct GeneratedQuestionDraft {
    string content;
    array<string, 4> options;
    char correctAnswer = 'A';
    string subject;
    string difficulty;
};

struct AIQuestionResult {
    bool success = false;
    bool usedMock = false;
    string message;
    string prompt;
    string rawResponse;
    vector<GeneratedQuestionDraft> questions;
};

class AIQuestionService {
private:
    string lastError;

public:
    AIQuestionResult generateQuestions(const AIQuestionRequest& request);
    bool saveApiKey(const string& apiKey);
    bool clearApiKey();
    bool hasSavedApiKey() const;
    string configurationStatus() const;
    string buildPrompt(const AIQuestionRequest& request) const;
    bool parseGeneratedQuestions(const string& responseText,
                                 vector<GeneratedQuestionDraft>& questions,
                                 string& errorMessage) const;
    bool validateDraft(const GeneratedQuestionDraft& draft, string& errorMessage) const;
    string draftsToJson(const vector<GeneratedQuestionDraft>& questions) const;
    string getLastError() const;

private:
    bool shouldUseMock() const;
    string savedApiKey() const;
    vector<GeneratedQuestionDraft> mockQuestions(const AIQuestionRequest& request) const;
    bool callRealApi(const AIQuestionRequest& request, const string& prompt, string& response);
    bool parseQuestionsObject(const json& root,
                              vector<GeneratedQuestionDraft>& questions,
                              string& errorMessage) const;
    string extractTextPayload(const json& root) const;
    string cleanJsonText(string text) const;
    static string envValue(const char* name);
    static size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp);
};
