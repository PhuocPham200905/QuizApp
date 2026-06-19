    string questionsText(bool showAnswer) {
        string text;
        for (Question& question : data.getQuestions()) {
            text += question.toText(showAnswer);
        }
        return text;
    }

    void showAiQuestionGenerator() {
        if (currentUser == nullptr || currentUser->getRole() != "teacher") {
            error("Chỉ giáo viên mới được dùng chức năng tạo câu hỏi bằng AI.");
            showCurrentDashboard();
            return;
        }

        clearControls();
        currentScreen = SCREEN_AI_QUESTION_GENERATOR;
        title("Tạo câu hỏi bằng AI", "Nhập yêu cầu, xem preview, chỉnh JSON nếu cần rồi lưu vào ngân hàng câu hỏi.");
        addNav("Giáo viên");

        if (aiQuestionRequest.subject.empty()) aiQuestionRequest.subject = currentUser->getClassName();
        if (aiQuestionRequest.grade.empty()) aiQuestionRequest.grade = "12";
        if (aiQuestionRequest.difficulty.empty()) aiQuestionRequest.difficulty = "Trung bình";
        if (aiQuestionRequest.count <= 0) aiQuestionRequest.count = 5;

        RECT client = {};
        GetClientRect(window, &client);
        int x = 260;
        int y = 82;
        int panelWidth = min(900, max(700, (int)client.right - x - 18));
        label("Môn học", x, y, 68, 26);
        edit(aiQuestionRequest.subject, x + 72, y - 4, 140, 30, 8501);
        label("Lớp/khối", x + 225, y, 68, 26);
        edit(aiQuestionRequest.grade, x + 295, y - 4, 70, 30, 8502);
        label("Độ khó", x + 380, y, 68, 26);
        edit(aiQuestionRequest.difficulty, x + 450, y - 4, 110, 30, 8504);
        label("Số câu", x + 575, y, 58, 26);
        edit(to_string(aiQuestionRequest.count), x + 635, y - 4, 60, 30, 8505);

        y += 40;
        label("Chủ đề", x, y, 68, 26);
        edit(aiQuestionRequest.topic, x + 72, y - 4, 300, 30, 8503);
        label("Ghi chú", x + 390, y, 63, 26);
        edit(aiQuestionRequest.extraNotes, x + 455, y - 4,
             max(180, panelWidth - 455), 30, 8506);

        int keyInputWidth = max(220, panelWidth - 390);
        label("Groq API key", x, y + 43, 95, 26);
        edit("", x + 100, y + 39, keyInputWidth, 30, 8510, true);
        button("Lưu key", x + 110 + keyInputWidth, y + 38, 105, 34, ID_AI_SAVE_API_KEY);
        button("Xóa key", x + 225 + keyInputWidth, y + 38, 95, 34, ID_AI_CLEAR_API_KEY);

        defaultButton("Sinh câu hỏi", x, y + 80, 130, 34, ID_AI_GENERATE_QUESTIONS);
        button("Cập nhật preview", x + 140, y + 80, 140, 34, ID_AI_REFRESH_PREVIEW);
        button("Lưu tất cả", x + 290, y + 80, 105, 34, ID_AI_SAVE_ALL);
        label("Câu số", x + 405, y + 85, 58, 24);
        edit("1", x + 465, y + 82, 45, 30, 8507);
        button("Lưu 1 câu", x + 520, y + 80, 105, 34, ID_AI_SAVE_ONE);
        button("Quay lại", x + 635, y + 80, 85, 34, ID_DASHBOARD);

        string status = aiQuestionStatus.empty()
                            ? aiQuestionService.configurationStatus() +
                                  " Hỗ trợ bàn phím tiếng Việt Telex/VNI của Windows."
                            : aiQuestionStatus;
        edit(status, x, y + 122, panelWidth, 38, 8508, false, true, true);

        aiQuestionDraftListView(x, y + 168, panelWidth, 125);

        label("JSON có thể chỉnh sửa", x, y + 302, 210, 24);
        edit(aiQuestionService.draftsToJson(generatedQuestionDrafts),
             x, y + 328, panelWidth, 82, 8509, false, true, false);
        setFocusTo(8503);
    }

    void saveAiApiKeyFromUi() {
        string apiKey = getText(8510);
        if (!aiQuestionService.saveApiKey(apiKey)) {
            error(aiQuestionService.getLastError());
            return;
        }

        HWND keyBox = GetDlgItem(window, 8510);
        if (keyBox != nullptr) {
            SetWindowTextW(keyBox, L"");
        }
        aiQuestionStatus =
            "Đã lưu API key an toàn trên máy này. Từ lần sinh câu hỏi tiếp theo, app sẽ dùng AI thật.";
        HWND statusBox = GetDlgItem(window, 8508);
        if (statusBox != nullptr) {
            setControlText(statusBox, aiQuestionStatus);
        }
        message("Đã lưu API key. Giáo viên không cần chạy lệnh setx.");
    }

    void clearAiApiKeyFromUi() {
        if (!aiQuestionService.clearApiKey()) {
            error(aiQuestionService.getLastError());
            return;
        }

        aiQuestionStatus =
            "Đã xóa API key khỏi máy này. App sẽ dùng dữ liệu mẫu cho đến khi lưu key mới.";
        HWND statusBox = GetDlgItem(window, 8508);
        if (statusBox != nullptr) {
            setControlText(statusBox, aiQuestionStatus);
        }
        message("Đã xóa API key đã lưu trên máy.");
    }

    void submitGenerateAiQuestions() {
        if (currentUser == nullptr || currentUser->getRole() != "teacher") {
            error("Chỉ giáo viên mới được dùng chức năng tạo câu hỏi bằng AI.");
            return;
        }

        aiQuestionRequest.subject = getText(8501);
        aiQuestionRequest.grade = getText(8502);
        aiQuestionRequest.topic = getText(8503);
        aiQuestionRequest.difficulty = getText(8504);
        aiQuestionRequest.extraNotes = getText(8506);
        aiQuestionRequest.count = atoi(getText(8505).c_str());

        if (aiQuestionRequest.subject.empty() || aiQuestionRequest.grade.empty() ||
            aiQuestionRequest.topic.empty() || aiQuestionRequest.difficulty.empty()) {
            error("Vui lòng nhập môn học, lớp/khối, chủ đề và độ khó.");
            return;
        }
        if (aiQuestionRequest.count <= 0 || aiQuestionRequest.count > 20) {
            error("Số lượng câu phải từ 1 đến 20.");
            return;
        }

        HWND statusBox = GetDlgItem(window, 8508);
        if (statusBox != nullptr) {
            setControlText(statusBox, "Đang sinh câu hỏi, vui lòng chờ...");
            UpdateWindow(window);
        }

        AIQuestionResult result = aiQuestionService.generateQuestions(aiQuestionRequest);
        if (!result.success) {
            aiQuestionStatus = "Không sinh được câu hỏi: " + result.message;
            generatedQuestionDrafts.clear();
            showAiQuestionGenerator();
            error(aiQuestionStatus);
            return;
        }

        generatedQuestionDrafts = result.questions;
        aiQuestionStatus = result.message + " Số câu: " + to_string((int)generatedQuestionDrafts.size()) + ".";
        showAiQuestionGenerator();
    }

    void refreshAiPreviewFromJson() {
        string jsonText = getText(8509);
        vector<GeneratedQuestionDraft> parsedQuestions;
        string parseError;
        if (!aiQuestionService.parseGeneratedQuestions(jsonText, parsedQuestions, parseError)) {
            error("Không cập nhật được preview.\r\n" + parseError);
            return;
        }

        generatedQuestionDrafts = parsedQuestions;
        aiQuestionStatus = "Đã cập nhật preview từ JSON. Số câu: " +
                           to_string((int)generatedQuestionDrafts.size()) + ".";
        showAiQuestionGenerator();
    }

    void saveAllAiQuestions() {
        if (!syncAiDraftsFromEditor()) {
            return;
        }
        if (generatedQuestionDrafts.empty()) {
            error("Chưa có câu hỏi AI để lưu.");
            return;
        }

        int savedCount = 0;
        for (const GeneratedQuestionDraft& draft : generatedQuestionDrafts) {
            if (saveAiDraft(draft)) {
                savedCount++;
            }
        }

        aiQuestionStatus = "Đã lưu " + to_string(savedCount) + " câu vào ngân hàng câu hỏi.";
        message(aiQuestionStatus);
        showQuestions(true, "", aiQuestionRequest.subject, "");
    }

    void saveOneAiQuestion() {
        if (!syncAiDraftsFromEditor()) {
            return;
        }
        int index = atoi(getText(8507).c_str());
        if (index < 1 || index > (int)generatedQuestionDrafts.size()) {
            error("Số thứ tự câu cần lưu không hợp lệ.");
            return;
        }

        if (!saveAiDraft(generatedQuestionDrafts[index - 1])) {
            return;
        }

        aiQuestionStatus = "Đã lưu câu số " + to_string(index) + " vào ngân hàng câu hỏi.";
        message(aiQuestionStatus);
        showAiQuestionGenerator();
    }

    bool syncAiDraftsFromEditor() {
        string jsonText = getText(8509);
        if (jsonText.empty()) {
            return true;
        }

        vector<GeneratedQuestionDraft> parsedQuestions;
        string parseError;
        if (!aiQuestionService.parseGeneratedQuestions(jsonText, parsedQuestions, parseError)) {
            error("JSON preview chưa hợp lệ nên chưa thể lưu.\r\n" + parseError);
            return false;
        }

        generatedQuestionDrafts = parsedQuestions;
        return true;
    }

    bool saveAiDraft(const GeneratedQuestionDraft& draft) {
        string validationError;
        if (!aiQuestionService.validateDraft(draft, validationError)) {
            error("Câu hỏi AI chưa hợp lệ: " + validationError);
            return false;
        }

        data.addQuestion(draft.content, draft.options, draft.correctAnswer,
                         draft.subject, draft.difficulty);
        return true;
    }

    HWND aiQuestionDraftListView(int x, int y, int w, int h) {
        HWND list = CreateWindowExW(WS_EX_STATICEDGE, WC_LISTVIEWW, L"",
                                   WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
                                   x, y, w, h, window, nullptr, instance, nullptr);
        styleModernListView(list, 34);
        controls.push_back(list);

        addListColumn(list, 0, "#", 40, LVCFMT_CENTER);
        addListColumn(list, 1, "Nội dung", max(220, w - 460));
        addListColumn(list, 2, "A", 70);
        addListColumn(list, 3, "B", 70);
        addListColumn(list, 4, "C", 70);
        addListColumn(list, 5, "D", 70);
        addListColumn(list, 6, "Đáp án", 60, LVCFMT_CENTER);
        addListColumn(list, 7, "Độ khó", 80, LVCFMT_CENTER);

        for (int row = 0; row < (int)generatedQuestionDrafts.size(); row++) {
            const GeneratedQuestionDraft& draft = generatedQuestionDrafts[row];
            vector<string> values = {
                to_string(row + 1),
                draft.content,
                draft.options[0],
                draft.options[1],
                draft.options[2],
                draft.options[3],
                string(1, draft.correctAnswer),
                draft.difficulty
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
        }

        return list;
    }

    void showQuestions(bool showAnswer, string keyword = "", string subject = "",
                       string difficulty = "") {
        clearControls();
        currentScreen = SCREEN_VIEW;
        title("Ngân hàng câu hỏi");
        if (currentUser && currentUser->getRole() == "teacher") addNav("Giáo viên");
        else addNav("Quản trị viên");
        label("Tìm kiếm", 260, 88, 80, 26);
        edit(keyword, 345, 84, 235, 30, 6401);
        label("Chủ đề", 595, 88, 65, 26);
        edit(subject, 665, 84, 170, 30, 6402);
        label("Độ khó", 850, 88, 65, 26);
        edit(difficulty, 920, 84, 150, 30, 6403);
        button("Lọc", 1085, 82, 85, 34, ID_QUESTION_FILTER);
        questionListView(260, 128, 980, 420, showAnswer, keyword, subject, difficulty);
        if (currentUser && currentUser->getRole() == "teacher") {
            button("Tạo câu hỏi bằng AI", 260, 565, 180, 36, ID_TEACHER_AI_QUESTIONS);
        }
        button("Về dashboard", 24, 535, 190, 40, ID_DASHBOARD);
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
        label("Độ khó", x1, y, 140, 26);
        edit("Trung bình", x2, y - 4, 220, 30, 4009);
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
        string difficulty = getText(4009);
        string imagePath = getText(4008);
        if (content.empty() || options[0].empty() || options[1].empty() || options[2].empty() ||
            options[3].empty() || correct.empty() || subject.empty() || difficulty.empty()) {
            error("Vui lòng nhập đầy đủ thông tin câu hỏi.");
            return;
        }
        char answer = (char)toupper(correct[0]);
        if (answer < 'A' || answer > 'D') {
            error("Đáp án đúng phai la A, B, C hoac D.");
            return;
        }
        data.addQuestion(content, options, answer, subject, difficulty, imagePath);
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
            "content,optionA,optionB,optionC,optionD,correctAnswer,difficulty\r\n"
            "\"Trong lập trình hướng đối tượng, class được dùng để làm gì?\",\"Mô tả khuôn mẫu tạo đối tượng\",\"Chạy chương trình nhanh hơn\",\"Lưu file trên máy tính\",\"Kết nối Internet\",A,\"Dễ\"\r\n"
            "\"Từ khóa nào dùng để khai báo class trong C++?\",\"object\",\"class\",\"define\",\"include\",B,\"Dễ\"\r\n"
            "\"Hàm nào được gọi tự động khi đối tượng được tạo?\",\"Destructor\",\"Constructor\",\"Getter\",\"Setter\",B,\"Trung bình\"\r\n"
            "\"Trong C++, thư viện nào thường dùng để làm việc với vector?\",\"<map>\",\"<array>\",\"<vector>\",\"<queue>\",C,\"Dễ\"\r\n"
            "\"Đặc điểm nào là lợi ích của tính đóng gói?\",\"Ẩn chi tiết xử lý bên trong\",\"Làm mất dữ liệu\",\"Xóa toàn bộ biến\",\"Bắt buộc dùng biến global\",A,\"Trung bình\"\r\n"
            "\"Nếu đáp án đúng là lựa chọn C, cột correctAnswer cần ghi gì?\",\"A\",\"B\",\"C\",\"D\",C,\"Dễ\"\r\n";
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
        questionListView(260, 92, 980, 420, true);
        label("Mã câu hỏi cần xóa", 300, 545, 170, 26);
        edit("", 480, 541, 180, 30, 4101);
        defaultButton("Xóa", 480, 595, 110, 38, ID_DELETE_QUESTION_SUBMIT);
        button("Về dashboard", 605, 595, 150, 38, ID_DASHBOARD);
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
