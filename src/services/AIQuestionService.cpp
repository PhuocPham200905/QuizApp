#include "AIQuestionService.h"

namespace {
const wchar_t* AI_CREDENTIAL_TARGET = L"HCMUTEQuizApp/GroqApiKey";
}

AIQuestionResult AIQuestionService::generateQuestions(const AIQuestionRequest& request) {
    AIQuestionResult result;
    result.prompt = buildPrompt(request);

    if (shouldUseMock()) {
        result.questions = mockQuestions(request);
        result.rawResponse = draftsToJson(result.questions);
        result.success = true;
        result.usedMock = true;
        result.message = "Đang dùng dữ liệu mẫu vì chưa cấu hình AI thật.";
        return result;
    }

    string response;
    if (!callRealApi(request, result.prompt, response)) {
        result.success = false;
        result.message = lastError.empty()
                             ? "Không gọi được dịch vụ AI. Vui lòng kiểm tra cấu hình."
                             : lastError;
        return result;
    }

    result.rawResponse = response;
    string parseError;
    if (!parseGeneratedQuestions(response, result.questions, parseError)) {
        result.success = false;
        result.message = parseError;
        return result;
    }

    result.success = true;
    result.message = "Đã sinh câu hỏi bằng AI.";
    return result;
}

string AIQuestionService::buildPrompt(const AIQuestionRequest& request) const {
    stringstream prompt;
    prompt
        << "Bạn là trợ lý tạo câu hỏi trắc nghiệm cho giáo viên Việt Nam.\n"
        << "Hãy tạo đúng " << request.count << " câu hỏi cho học sinh lớp/khối "
        << request.grade << ".\n"
        << "Môn học: " << request.subject << "\n"
        << "Chủ đề/nội dung: " << request.topic << "\n"
        << "Độ khó: " << request.difficulty << "\n";

    if (!request.extraNotes.empty()) {
        prompt << "Ghi chú thêm: " << request.extraNotes << "\n";
    }

    prompt
        << "Yêu cầu chất lượng:\n"
        << "- Mỗi câu hỏi phải rõ ràng, phù hợp lứa tuổi, không mơ hồ.\n"
        << "- Mỗi câu hỏi có đúng 4 đáp án A/B/C/D.\n"
        << "- Chỉ có một đáp án đúng.\n"
        << "- Không tạo câu hỏi mẹo, không cần kiến thức ngoài chủ đề.\n"
        << "- Viết đầy đủ dấu tiếng Việt.\n"
        << "- Không kèm giải thích, không Markdown.\n"
        << "Chỉ trả về JSON hợp lệ theo đúng schema sau:\n"
        << "{\n"
        << "  \"questions\": [\n"
        << "    {\n"
        << "      \"content\": \"Câu hỏi...\",\n"
        << "      \"options\": [\"...\", \"...\", \"...\", \"...\"],\n"
        << "      \"correctAnswer\": \"A\",\n"
        << "      \"difficulty\": \"" << request.difficulty << "\",\n"
        << "      \"subject\": \"" << request.subject << "\"\n"
        << "    }\n"
        << "  ]\n"
        << "}";
    return prompt.str();
}

bool AIQuestionService::parseGeneratedQuestions(const string& responseText,
                                                vector<GeneratedQuestionDraft>& questions,
                                                string& errorMessage) const {
    questions.clear();
    string cleaned = cleanJsonText(responseText);
    if (cleaned.empty()) {
        errorMessage = "AI không trả về nội dung nào.";
        return false;
    }

    try {
        json root = json::parse(cleaned);
        if (parseQuestionsObject(root, questions, errorMessage)) {
            return true;
        }

        string nestedText = extractTextPayload(root);
        if (!nestedText.empty()) {
            json nested = json::parse(cleanJsonText(nestedText));
            return parseQuestionsObject(nested, questions, errorMessage);
        }
    } catch (const exception& error) {
        errorMessage = string("Không đọc được JSON do AI trả về: ") + error.what();
        return false;
    }

    if (errorMessage.empty()) {
        errorMessage = "AI trả về sai định dạng. Cần có mảng questions.";
    }
    return false;
}

bool AIQuestionService::validateDraft(const GeneratedQuestionDraft& draft,
                                      string& errorMessage) const {
    if (draft.content.empty()) {
        errorMessage = "Thiếu nội dung câu hỏi.";
        return false;
    }
    if (draft.subject.empty()) {
        errorMessage = "Thiếu môn học.";
        return false;
    }
    if (draft.difficulty.empty()) {
        errorMessage = "Thiếu độ khó.";
        return false;
    }
    for (int i = 0; i < 4; i++) {
        if (draft.options[i].empty()) {
            errorMessage = "Thiếu đáp án " + string(1, (char)('A' + i)) + ".";
            return false;
        }
    }
    if (draft.correctAnswer < 'A' || draft.correctAnswer > 'D') {
        errorMessage = "Đáp án đúng phải là A, B, C hoặc D.";
        return false;
    }
    return true;
}

string AIQuestionService::draftsToJson(const vector<GeneratedQuestionDraft>& questions) const {
    json root;
    root["questions"] = json::array();
    for (const GeneratedQuestionDraft& draft : questions) {
        root["questions"].push_back({
            {"content", draft.content},
            {"options", {draft.options[0], draft.options[1], draft.options[2], draft.options[3]}},
            {"correctAnswer", string(1, draft.correctAnswer)},
            {"difficulty", draft.difficulty},
            {"subject", draft.subject}
        });
    }
    return root.dump(2);
}

string AIQuestionService::getLastError() const {
    return lastError;
}

bool AIQuestionService::saveApiKey(const string& apiKey) {
    string trimmed = apiKey;
    trimmed.erase(trimmed.begin(),
                  find_if(trimmed.begin(), trimmed.end(),
                          [](unsigned char ch) { return !isspace(ch); }));
    trimmed.erase(find_if(trimmed.rbegin(), trimmed.rend(),
                          [](unsigned char ch) { return !isspace(ch); }).base(),
                  trimmed.end());

    if (trimmed.size() < 20 || trimmed.rfind("gsk_", 0) != 0) {
        lastError = "API key Groq không hợp lệ. Key thường bắt đầu bằng gsk_.";
        return false;
    }

    CREDENTIALW credential = {};
    credential.Type = CRED_TYPE_GENERIC;
    credential.TargetName = const_cast<LPWSTR>(AI_CREDENTIAL_TARGET);
    credential.CredentialBlobSize = static_cast<DWORD>(trimmed.size());
    credential.CredentialBlob =
        reinterpret_cast<LPBYTE>(const_cast<char*>(trimmed.data()));
    credential.Persist = CRED_PERSIST_LOCAL_MACHINE;
    credential.UserName = const_cast<LPWSTR>(L"Groq");

    if (!CredWriteW(&credential, 0)) {
        lastError = "Windows không lưu được API key. Mã lỗi: " +
                    to_string(GetLastError()) + ".";
        return false;
    }

    lastError.clear();
    return true;
}

bool AIQuestionService::clearApiKey() {
    if (CredDeleteW(AI_CREDENTIAL_TARGET, CRED_TYPE_GENERIC, 0)) {
        lastError.clear();
        return true;
    }

    DWORD errorCode = GetLastError();
    if (errorCode == ERROR_NOT_FOUND) {
        lastError.clear();
        return true;
    }

    lastError = "Windows không xóa được API key. Mã lỗi: " +
                to_string(errorCode) + ".";
    return false;
}

bool AIQuestionService::hasSavedApiKey() const {
    return !savedApiKey().empty();
}

string AIQuestionService::configurationStatus() const {
    if (!envValue("QUIZAPP_AI_API_KEY").empty() ||
        !envValue("GROQ_API_KEY").empty()) {
        return "AI thật đã sẵn sàng bằng cấu hình hệ thống.";
    }
    if (hasSavedApiKey()) {
        return "AI thật đã sẵn sàng. API key được lưu an toàn trên máy này.";
    }
    return "Chưa có API key. Hãy dán key Groq bên dưới; app đang dùng dữ liệu mẫu.";
}

bool AIQuestionService::shouldUseMock() const {
    string mode = envValue("QUIZAPP_AI_MODE");
    transform(mode.begin(), mode.end(), mode.begin(),
              [](unsigned char ch) { return (char)tolower(ch); });
    if (mode == "mock") {
        return true;
    }
    if (mode == "real") {
        return false;
    }
    return envValue("QUIZAPP_AI_API_KEY").empty() &&
           envValue("GROQ_API_KEY").empty() &&
           !hasSavedApiKey();
}

string AIQuestionService::savedApiKey() const {
    PCREDENTIALW credential = nullptr;
    if (!CredReadW(AI_CREDENTIAL_TARGET, CRED_TYPE_GENERIC, 0, &credential)) {
        return "";
    }

    string apiKey;
    if (credential->CredentialBlob != nullptr &&
        credential->CredentialBlobSize > 0) {
        apiKey.assign(
            reinterpret_cast<const char*>(credential->CredentialBlob),
            credential->CredentialBlobSize);
    }
    CredFree(credential);
    return apiKey;
}

vector<GeneratedQuestionDraft> AIQuestionService::mockQuestions(const AIQuestionRequest& request) const {
    int count = max(1, min(request.count, 20));
    vector<GeneratedQuestionDraft> questions;
    questions.reserve(count);

    for (int i = 0; i < count; i++) {
        GeneratedQuestionDraft draft;
        draft.subject = request.subject.empty() ? "Môn học" : request.subject;
        draft.difficulty = request.difficulty.empty() ? "Trung bình" : request.difficulty;
        draft.content = "Câu " + to_string(i + 1) +
                        ": Nội dung nào phù hợp với chủ đề \"" +
                        (request.topic.empty() ? "bài học" : request.topic) + "\"?";
        draft.options = {
            "Phương án đúng theo kiến thức bài học",
            "Phương án gần đúng nhưng thiếu ý quan trọng",
            "Phương án không liên quan đến chủ đề",
            "Phương án trái với nội dung bài học"
        };
        draft.correctAnswer = 'A';
        questions.push_back(draft);
    }

    return questions;
}

bool AIQuestionService::callRealApi(const AIQuestionRequest& request,
                                    const string& prompt,
                                    string& response) {
    string endpoint = envValue("QUIZAPP_AI_ENDPOINT");
    string apiKey = envValue("QUIZAPP_AI_API_KEY");
    if (apiKey.empty()) {
        apiKey = envValue("GROQ_API_KEY");
    }
    if (apiKey.empty()) {
        apiKey = savedApiKey();
    }
    if (endpoint.empty()) {
        endpoint = "https://api.groq.com/openai/v1/chat/completions";
    }
    if (apiKey.empty()) {
        lastError = "Chưa có API key Groq. Hãy nhập key ngay trong màn hình tạo câu hỏi AI.";
        return false;
    }

    string model = envValue("QUIZAPP_AI_MODEL");
    if (model.empty()) {
        model = "llama-3.3-70b-versatile";
    }

    json body;
    body["model"] = model;
    body["temperature"] = 0.2;
    body["max_completion_tokens"] = 2500;
    body["response_format"] = {{"type", "json_object"}};
    body["messages"] = json::array({
        {
            {"role", "system"},
            {"content", "Bạn chỉ trả về JSON hợp lệ, viết đầy đủ dấu tiếng Việt, không Markdown, không giải thích."}
        },
        {
            {"role", "user"},
            {"content", prompt}
        }
    });

    CURL* curl = curl_easy_init();
    if (curl == nullptr) {
        lastError = "Không khởi tạo được kết nối AI.";
        return false;
    }

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    string authHeader = "Authorization: Bearer " + apiKey;
    headers = curl_slist_append(headers, authHeader.c_str());
    string bodyText = body.dump();

    curl_easy_setopt(curl, CURLOPT_URL, endpoint.c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, bodyText.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, 8000L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 30000L);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "QuizApp/1.0");
    curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);

    CURLcode result = curl_easy_perform(curl);
    long statusCode = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &statusCode);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (result != CURLE_OK) {
        lastError = string("Lỗi kết nối AI: ") + curl_easy_strerror(result);
        return false;
    }
    if (statusCode < 200 || statusCode >= 300) {
        lastError = "AI trả về HTTP " + to_string(statusCode) + ": " + response;
        return false;
    }
    return true;
}

bool AIQuestionService::parseQuestionsObject(const json& root,
                                             vector<GeneratedQuestionDraft>& questions,
                                             string& errorMessage) const {
    if (!root.is_object() || !root.contains("questions") || !root.at("questions").is_array()) {
        errorMessage = "AI trả về sai định dạng. Cần có mảng questions.";
        return false;
    }

    for (const json& item : root.at("questions")) {
        if (!item.is_object()) {
            errorMessage = "Một câu hỏi AI trả về không phải object.";
            return false;
        }
        if (!item.contains("content") || !item.contains("options") ||
            !item.contains("correctAnswer") || !item.contains("subject") ||
            !item.contains("difficulty")) {
            errorMessage = "Một câu hỏi AI trả về bị thiếu trường.";
            return false;
        }
        if (!item.at("options").is_array() || item.at("options").size() != 4) {
            errorMessage = "Mỗi câu hỏi phải có đúng 4 đáp án.";
            return false;
        }

        GeneratedQuestionDraft draft;
        draft.content = item.at("content").get<string>();
        for (int i = 0; i < 4; i++) {
            draft.options[i] = item.at("options").at(i).get<string>();
        }
        string answer = item.at("correctAnswer").get<string>();
        draft.correctAnswer = answer.empty() ? '\0' : (char)toupper((unsigned char)answer[0]);
        draft.subject = item.at("subject").get<string>();
        draft.difficulty = item.at("difficulty").get<string>();

        string validationError;
        if (!validateDraft(draft, validationError)) {
            errorMessage = "Câu hỏi AI không hợp lệ: " + validationError;
            return false;
        }
        questions.push_back(draft);
    }

    if (questions.empty()) {
        errorMessage = "AI không tạo câu hỏi nào.";
        return false;
    }
    return true;
}

string AIQuestionService::extractTextPayload(const json& root) const {
    if (root.contains("choices") && root.at("choices").is_array() && !root.at("choices").empty()) {
        const json& choice = root.at("choices").at(0);
        if (choice.contains("message") && choice.at("message").contains("content")) {
            return choice.at("message").at("content").get<string>();
        }
        if (choice.contains("text")) {
            return choice.at("text").get<string>();
        }
    }
    if (root.contains("output_text")) {
        return root.at("output_text").get<string>();
    }
    return "";
}

string AIQuestionService::cleanJsonText(string text) const {
    while (!text.empty() && isspace((unsigned char)text.front())) {
        text.erase(text.begin());
    }
    while (!text.empty() && isspace((unsigned char)text.back())) {
        text.pop_back();
    }
    if (text.rfind("```json", 0) == 0) {
        text.erase(0, 7);
    } else if (text.rfind("```", 0) == 0) {
        text.erase(0, 3);
    }
    while (!text.empty() && isspace((unsigned char)text.front())) {
        text.erase(text.begin());
    }
    if (text.size() >= 3 && text.substr(text.size() - 3) == "```") {
        text.erase(text.size() - 3);
    }
    while (!text.empty() && isspace((unsigned char)text.back())) {
        text.pop_back();
    }
    return text;
}

string AIQuestionService::envValue(const char* name) {
    const char* value = getenv(name);
    return value == nullptr ? "" : string(value);
}

size_t AIQuestionService::writeCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t totalSize = size * nmemb;
    string* response = static_cast<string*>(userp);
    response->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}
