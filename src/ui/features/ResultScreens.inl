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

    HWND resultsListView(int x, int y, int w, int h, bool onlyCurrentStudent,
                         string studentFilter = "", string examFilter = "") {
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
        string normalizedStudent = lowerText(studentFilter);
        string normalizedExam = lowerText(examFilter);
        for (Result& result : data.getResults()) {
            if (onlyCurrentStudent &&
                (currentUser == nullptr || result.getStudentId() != currentUser->getUserId())) {
                continue;
            }
            string studentName = data.getUserNameById(result.getStudentId());
            string examTitle = data.getExamTitle(result.getExamId());
            if (!normalizedStudent.empty() &&
                lowerText(studentName + " " + result.getStudentId()).find(normalizedStudent) == string::npos) {
                continue;
            }
            if (!normalizedExam.empty() &&
                lowerText(examTitle + " " + result.getExamId()).find(normalizedExam) == string::npos) {
                continue;
            }

            vector<string> values = {
                result.getResultId(),
                studentName,
                examTitle,
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

    HWND questionListView(int x, int y, int w, int h, bool showAnswer,
                          string keyword = "", string subjectFilter = "",
                          string difficultyFilter = "") {
        HWND list = CreateWindowExW(WS_EX_CLIENTEDGE, WC_LISTVIEWW, L"",
                                   WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
                                   x, y, w, h, window, nullptr, instance, nullptr);
        styleModernListView(list, 34);
        controls.push_back(list);

        addListColumn(list, 0, "Mã câu", 82);
        addListColumn(list, 1, "Nội dung câu hỏi", max(300, w - 500));
        addListColumn(list, 2, "Chủ đề", 170);
        addListColumn(list, 3, "Độ khó", 120, LVCFMT_CENTER);
        if (showAnswer) {
            addListColumn(list, 4, "Đáp án", 80, LVCFMT_CENTER);
        }

        int row = 0;
        string normalizedKeyword = lowerText(keyword);
        string normalizedSubject = lowerText(subjectFilter);
        string normalizedDifficulty = lowerText(difficultyFilter);
        for (Question& question : data.getQuestions()) {
            string difficulty = question.getDifficulty().empty() ? "Trung bình" : question.getDifficulty();
            if (!normalizedKeyword.empty() &&
                lowerText(question.getContent()).find(normalizedKeyword) == string::npos) {
                continue;
            }
            if (!normalizedSubject.empty() &&
                lowerText(question.getSubject()).find(normalizedSubject) == string::npos) {
                continue;
            }
            if (!normalizedDifficulty.empty() &&
                lowerText(difficulty).find(normalizedDifficulty) == string::npos) {
                continue;
            }
            vector<string> values = {
                question.getQuestionId(),
                question.getContent(),
                question.getSubject(),
                difficulty,
            };
            if (showAnswer) {
                values.push_back(string(1, question.getCorrectAnswer()));
            }

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

    string resultStatistics(bool onlyCurrentStudent) {
        int count = 0;
        double totalPercent = 0.0;
        double highest = 0.0;
        double lowest = 100.0;
        for (const Result& result : data.getResults()) {
            if (onlyCurrentStudent &&
                (currentUser == nullptr || result.getStudentId() != currentUser->getUserId())) {
                continue;
            }
            double percent = result.getTotalQuestions() > 0
                                 ? 100.0 * result.getScore() / result.getTotalQuestions()
                                 : 0.0;
            totalPercent += percent;
            highest = max(highest, percent);
            lowest = min(lowest, percent);
            count++;
        }
        if (count == 0) {
            return "Chưa có dữ liệu điểm.";
        }
        stringstream text;
        text << fixed << setprecision(1)
             << "Số bài: " << count
             << "   |   Trung bình: " << totalPercent / count << "%"
             << "   |   Cao nhất: " << highest << "%"
             << "   |   Thấp nhất: " << lowest << "%";
        return text.str();
    }

    string csvValue(string value) {
        size_t position = 0;
        while ((position = value.find('"', position)) != string::npos) {
            value.insert(position, 1, '"');
            position += 2;
        }
        return "\"" + value + "\"";
    }

    void exportResultsCsv() {
        wchar_t fileName[MAX_PATH] = L"ket_qua_quiz.csv";
        OPENFILENAMEW dialog = {};
        dialog.lStructSize = sizeof(dialog);
        dialog.hwndOwner = window;
        dialog.lpstrFilter = L"CSV Files (*.csv)\0*.csv\0All Files (*.*)\0*.*\0";
        dialog.lpstrFile = fileName;
        dialog.nMaxFile = MAX_PATH;
        dialog.Flags = OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;
        dialog.lpstrDefExt = L"csv";
        if (!GetSaveFileNameW(&dialog)) {
            return;
        }

        ofstream file(wideToUtf8(fileName), ios::binary);
        if (!file) {
            error("Không thể tạo file CSV kết quả.");
            return;
        }
        file << "\xEF\xBB\xBF";
        file << "resultId,studentId,studentName,examId,examTitle,score,totalQuestions,percentage,submittedAt\r\n";
        for (const Result& result : data.getResults()) {
            double percent = result.getTotalQuestions() > 0
                                 ? 100.0 * result.getScore() / result.getTotalQuestions()
                                 : 0.0;
            stringstream percentage;
            percentage << fixed << setprecision(1) << percent;
            file << csvValue(result.getResultId()) << ","
                 << csvValue(result.getStudentId()) << ","
                 << csvValue(data.getUserNameById(result.getStudentId())) << ","
                 << csvValue(result.getExamId()) << ","
                 << csvValue(data.getExamTitle(result.getExamId())) << ","
                 << result.getScore() << "," << result.getTotalQuestions() << ","
                 << percentage.str() << ","
                 << csvValue(result.getSubmittedAt()) << "\r\n";
        }
        message("Đã xuất kết quả ra file CSV.");
    }

    void showAllResults(string studentFilter = "", string examFilter = "") {
        clearControls();
        currentScreen = SCREEN_VIEW;
        title("Kết quả tất cả học sinh", "Theo dõi điểm và thời gian nộp bài.");
        label("Học sinh", 280, 88, 80, 26);
        edit(studentFilter, 365, 84, 210, 30, 6501);
        label("Đề thi", 590, 88, 65, 26);
        edit(examFilter, 660, 84, 210, 30, 6502);
        button("Lọc", 885, 82, 80, 34, ID_RESULTS_FILTER);
        button("Xuất CSV", 980, 82, 120, 34, ID_RESULTS_EXPORT);
        HWND stats = label(resultStatistics(false), 280, 125, 850, 28);
        SendMessageW(stats, WM_SETFONT, (WPARAM)brandFont, TRUE);
        resultsListView(280, 160, 940, 470, false, studentFilter, examFilter);
        button("Về dashboard", 24, 535, 190, 40, ID_DASHBOARD);
    }

    void showMyResults() {
        clearControls();
        currentScreen = SCREEN_VIEW;
        title("Lịch sử điểm", "Các bài thi bạn đã hoàn thành.");
        HWND stats = label(resultStatistics(true), 280, 92, 850, 28);
        SendMessageW(stats, WM_SETFONT, (WPARAM)brandFont, TRUE);
        resultsListView(280, 130, 940, 500, true);
        button("Về dashboard", 24, 535, 190, 40, ID_DASHBOARD);
    }

    string antiCheatMonitorText() {
        stringstream text;
        json sessions = data.getExamSessions();
        text << "PHIÊN THI\r\n";
        text << "Session ID | Học sinh | Đề | Trạng thái | Thiết bị | Vi phạm | Cập nhật\r\n";
        text << "--------------------------------------------------------------------------------\r\n";
        if (sessions.contains("documents")) {
            for (const json& document : sessions["documents"]) {
                string sessionId = document.value("name", "");
                size_t slash = sessionId.find_last_of('/');
                if (slash != string::npos) sessionId = sessionId.substr(slash + 1);
                const json& fields = document.value("fields", json::object());
                text << sessionId << " | "
                     << (fields.contains("studentId") ? fields["studentId"].value("stringValue", "") : "") << " | "
                     << (fields.contains("examId") ? fields["examId"].value("stringValue", "") : "") << " | "
                     << (fields.contains("status") ? fields["status"].value("stringValue", "") : "") << " | "
                     << (fields.contains("deviceId") ? fields["deviceId"].value("stringValue", "") : "") << " | "
                     << (fields.contains("violationCount") ? fields["violationCount"].value("integerValue", "0") : "0") << " | "
                     << (fields.contains("updatedAt") ? fields["updatedAt"].value("stringValue", "") : "")
                     << "\r\n";
            }
        }

        text << "\r\nNHẬT KÝ GẦN ĐÂY\r\n";
        text << "Thời gian | Người dùng | Đề | Hành động | Chi tiết | Thiết bị\r\n";
        text << "--------------------------------------------------------------------------------\r\n";
        json logs = data.getAuditLogs();
        if (logs.contains("documents")) {
            vector<json> documents = logs["documents"].get<vector<json>>();
            int start = max(0, (int)documents.size() - 80);
            for (int index = start; index < (int)documents.size(); index++) {
                const json& fields = documents[index].value("fields", json::object());
                auto field = [&fields](const string& name) {
                    return fields.contains(name) ? fields[name].value("stringValue", "") : "";
                };
                text << field("createdAt") << " | " << field("userId") << " | "
                     << field("examId") << " | " << field("action") << " | "
                     << field("details") << " | " << field("deviceId") << "\r\n";
            }
        }
        return text.str();
    }

    void showAntiCheatMonitor() {
        clearControls();
        currentScreen = SCREEN_VIEW;
        title("Giám sát chống gian lận", "Theo dõi phiên thi, vi phạm và mở khóa bài bị gián đoạn.");
        addNav("Quản trị viên");
        edit(antiCheatMonitorText(), 260, 100, 980, 500, 0, false, true, true);
        label("Session ID cần mở khóa", 300, 630, 190, 26);
        edit("", 500, 626, 360, 30, 6601);
        defaultButton("Mở khóa", 875, 624, 120, 38, ID_UNLOCK_SESSION);
        button("Về dashboard", 24, 535, 190, 40, ID_DASHBOARD);
    }

    void unlockExamSession() {
        string sessionId = getText(6601);
        if (sessionId.empty()) {
            error("Vui lòng nhập Session ID cần mở khóa.");
            return;
        }
        if (!data.unlockExamSession(sessionId)) {
            error("Không mở khóa được phiên thi.");
            return;
        }
        data.writeAuditLog(currentUser ? currentUser->getUserId() : "admin", "",
                           "SESSION_UNLOCKED", sessionId);
        message("Đã mở khóa phiên thi.");
        showAntiCheatMonitor();
    }

    vector<string> parseQuestionIds(string input) {
        replace(input.begin(), input.end(), ',', ' ');
        vector<string> ids;
        stringstream ss(input);
        string id;
        while (ss >> id) ids.push_back(id);
        return ids;
    }
