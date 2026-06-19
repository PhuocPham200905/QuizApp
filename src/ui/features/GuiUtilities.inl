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
