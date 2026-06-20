    string examsText(bool onlyOpen = false) {
        stringstream ss;
        ss << "ID    Tên đề                Làm bài   Mở lúc            Đóng lúc          Số câu  Lượt  Giáo viên tải     MK   File Trạng thái\r\n";
        ss << "-------------------------------------------------------------------------------------------------------------------------------\r\n";
        for (Exam& exam : data.getExams()) {
            string now = QuizData::currentMinuteText();
            if (onlyOpen && !exam.canTake(now)) continue;
            string status = exam.statusText(now);
            ss << left << setw(6) << exam.getExamId()
               << setw(22) << exam.getTitle()
               << setw(10) << (to_string(exam.getDurationMinutes()) + " phut")
               << setw(18) << (exam.getStartAt().empty() ? "-" : exam.getStartAt())
               << setw(18) << exam.getCloseAt()
               << setw(8) << exam.totalQuestions()
               << setw(6) << exam.getAttemptLimit()
               << setw(20) << data.getUserNameById(exam.getCreatedBy())
               << setw(5) << (exam.hasPassword() ? "Có" : "Không")
               << setw(5) << (exam.hasAttachment() ? "Có" : "Không")
               << status << "\r\n";
        }
        return ss.str();
    }

    void showExams() {
        clearControls();
        currentScreen = SCREEN_VIEW;
        title("Danh sách đề thi");
        if (currentUser && currentUser->getRole() == "teacher") addNav("Giáo viên");
        else addNav("Quản trị viên");
        examListView(250, 100, 1000, 470, false);
        if (currentUser && currentUser->getRole() == "teacher") {
            defaultButton("Quản lý đề", 520, 600, 150, 40, ID_TEACHER_MANAGE_EXAMS);
        }
        button("Về dashboard", 24, 535, 190, 40, ID_DASHBOARD);
    }

    void showOpenExams() {
        clearControls();
        currentScreen = SCREEN_VIEW;
        title("Đề thi đang mở");
        addNav("Học sinh");
        examListView(250, 100, 1000, 470, true);
        button("Về dashboard", 24, 535, 190, 40, ID_DASHBOARD);
    }

    void showCreateExam() {
        clearControls();
        currentScreen = SCREEN_CREATE_EXAM;
        title("Giáo viên tải đề thi", "Đề từ ngân hàng: nhập Mã câu hỏi. Đề file gốc: để trống Mã câu hỏi và nhập Đáp án file đề.");
        questionListView(260, 78, 980, 125, true);
        label("Tên đề thi", 290, 225, 170, 26);
        edit("", 500, 221, 280, 30, 5001);
        label("Thời gian làm bài", 290, 263, 170, 26);
        edit("30", 500, 259, 100, 30, 5002);
        label("Số lần làm tối đa", 620, 263, 160, 26);
        edit("1", 790, 259, 70, 30, 5010);
        checkBox("Trộn câu hỏi", 900, 259, 150, 30, 5011, false);
        label("Mở lúc", 290, 301, 170, 26);
        edit(QuizData::currentMinuteText(), 500, 297, 190, 30, 5006);
        checkBox("Trộn đáp án", 900, 297, 150, 30, 5012, false);
        label("Đóng lúc", 290, 339, 170, 26);
        edit("2026-12-31 23:59", 500, 335, 190, 30, 5004);
        label("Tự nộp sau Alt+Tab", 900, 339, 175, 26);
        edit("3", 1080, 335, 70, 30, 5013);
        label("Mã câu hỏi", 290, 377, 170, 26);
        edit("", 500, 373, 280, 30, 5003);
        label("Dung q1 q2... neu lay tu ngan hang", 795, 377, 270, 26);
        label("Đáp án file đề", 290, 415, 170, 26);
        edit("", 500, 411, 405, 30, 5009);
        label("Vi du: A C B B A", 920, 415, 160, 26);
        label("Mật khẩu de", 290, 453, 170, 26);
        edit("", 500, 449, 190, 30, 5005);
        label("Bỏ trống nếu đề không cần mật khẩu", 700, 453, 270, 26);
        label("File đề", 290, 491, 170, 26);
        edit("", 500, 487, 280, 30, 5007);
        button("Chọn file", 795, 487, 110, 32, ID_CREATE_EXAM_CHOOSE_FILE);
        label("Link file đề (dùng đa máy)", 290, 529, 195, 26);
        edit("", 500, 525, 405, 30, 5008);
        label("Nên dùng URL tải công khai nếu học sinh làm trên máy khác.", 920, 529, 300, 26);
        defaultButton("Tạo đề", 500, 585, 120, 38, ID_CREATE_EXAM_SUBMIT);
        button("Về dashboard", 635, 585, 150, 38, ID_DASHBOARD);
        setFocusTo(5001);
    }

    void submitCreateExam() {
        string title = getText(5001);
        string durationText = getText(5002);
        string idsText = getText(5003);
        string closeAt = getText(5004);
        string examPassword = getText(5005);
        string startAt = getText(5006);
        string attachmentPath = getText(5007);
        string attachmentUrl = getText(5008);
        string answerKeyText = getText(5009);
        string attemptLimitText = getText(5010);
        string violationLimitText = getText(5013);
        bool shuffleQuestions = isChecked(5011);
        bool shuffleAnswers = isChecked(5012);
        if (title.empty() || durationText.empty() || startAt.empty() || closeAt.empty()) {
            error("Vui lòng nhập đầy đủ thông tin đề thi.");
            return;
        }

        int duration = atoi(durationText.c_str());
        if (duration <= 0) {
            error("Thời gian làm bài phai lon hon 0.");
            return;
        }
        int attemptLimit = atoi(attemptLimitText.c_str());
        if (attemptLimit <= 0) {
            error("Số lần làm tối đa phải lớn hơn 0.");
            return;
        }
        int violationLimit = atoi(violationLimitText.c_str());
        if (violationLimit <= 0 || violationLimit > 20) {
            error("Số lần Alt+Tab trước khi tự nộp phải từ 1 đến 20.");
            return;
        }
        if (!isDateTimeText(startAt) || !isDateTimeText(closeAt)) {
            error("Thời gian mở/đóng đề cần theo dạng YYYY-MM-DD HH:MM.");
            return;
        }
        if (closeAt <= startAt) {
            error("Thời gian đóng đề phải sau thời gian mở đề.");
            return;
        }

        vector<string> ids = parseQuestionIds(idsText);
        vector<string> validIds;
        string questionIdError;
        if (!ids.empty() &&
            !data.validateQuestionIds(ids, validIds, questionIdError)) {
            error("Không thể tạo đề thi.\r\n" + questionIdError);
            return;
        }

        vector<char> answerKey;
        string answerKeyError;
        if (!answerKeyText.empty() && !parseAnswerKey(answerKeyText, answerKey, answerKeyError)) {
            error(answerKeyError);
            return;
        }

        if (!ids.empty() && validIds.empty()) {
            error("Không có mã câu hỏi hợp lệ.");
            return;
        }
        if (!validIds.empty() && !answerKey.empty()) {
            error("Chỉ nhập Mã câu hỏi hoặc Đáp án file đề, không cần nhập cả hai.");
            return;
        }
        if (validIds.empty() && answerKey.empty()) {
            error("Nhap Mã câu hỏi tu ngan hang hoac nhap Đáp án file đề, vi du: A C B B A.");
            return;
        }
        if (validIds.empty() && attachmentPath.empty() && attachmentUrl.empty()) {
            error("De file goc can chon File đề hoac nhap Link file đề.");
            return;
        }

        bool localFileOnly = !attachmentPath.empty() && attachmentUrl.empty();
        if (!data.addExam(title, duration, validIds, answerKey, currentUser->getUserId(),
                          startAt, closeAt, examPassword, attachmentPath, attachmentUrl,
                          attemptLimit, violationLimit, shuffleQuestions, shuffleAnswers)) {
            error(data.getFirebaseStatus());
            return;
        }
        if (localFileOnly) {
            message("Đã tạo đề thi.\r\n\r\n"
                    "Lưu ý: file hiện chỉ mở được trên máy giáo viên. "
                    "Hãy thêm Link file đề công khai trong Quản lý đề thi "
                    "để học sinh dùng máy khác có thể mở.");
        } else {
            message("Đã tạo đề thi.");
        }
        showTeacherDashboard();
    }

    void showToggleExam() {
        clearControls();
        currentScreen = SCREEN_TOGGLE_EXAM;
        title("Mở/đóng đề thi");
        examListView(250, 90, 1000, 330, false);
        label("Mã đề thi", 300, 455, 140, 26);
        edit("", 460, 451, 150, 30, 5101);
        defaultButton("Đổi trạng thái", 460, 505, 140, 38, ID_TOGGLE_EXAM_SUBMIT);
        button("Về dashboard", 615, 505, 150, 38, ID_DASHBOARD);
        label("Số lượt mới", 300, 565, 140, 26);
        edit("1", 460, 561, 100, 30, 5102);
        button("Cập nhật lượt", 580, 557, 150, 38, ID_UPDATE_EXAM_ATTEMPT_LIMIT);
        setFocusTo(5101);
    }

    void submitToggleExam() {
        string examId = getText(5101);
        Exam* exam = data.findExamById(examId);
        if (exam == nullptr) {
            error("Không tìm thấy đề thi.");
            return;
        }
        bool newOpenState = !exam->isOpen();
        if (!data.updateExamOpenState(examId, newOpenState)) {
            error("Không thể lưu trạng thái đề thi.\r\n" + data.getFirebaseStatus());
            return;
        }
        message("Đã đổi trạng thái đề thi.");
        if (currentUser->getRole() == "teacher") showTeacherDashboard();
        else showAdminDashboard();
    }

    void submitUpdateExamAttemptLimit() {
        string examId = getText(5101);
        int attemptLimit = atoi(getText(5102).c_str());
        if (examId.empty()) {
            error("Vui lòng nhập mã đề thi.");
            return;
        }
        if (attemptLimit <= 0) {
            error("Số lượt mới phải lớn hơn 0.");
            return;
        }
        if (!data.updateExamAttemptLimit(examId, attemptLimit)) {
            error("Không thể tải danh sách đề thi. Vui lòng thử lại.");
            return;
        }

        message("Đã cập nhật số lượt làm bài.");
        showToggleExam();
    }

    void showManageExam() {
        clearControls();
        currentScreen = SCREEN_MANAGE_EXAM;
        title("Quản lý đề thi", "Giáo viên có thể tải thông tin, chỉnh sửa hoặc xóa đề do mình tạo.");
        addNav("Giáo viên");
        examListView(250, 82, 1000, 180, false);

        label("Mã đề", 280, 285, 90, 26);
        edit("", 375, 281, 150, 30, 6301);
        button("Tải thông tin", 540, 279, 100, 34, ID_LOAD_EXAM_EDIT);

        label("Tên đề", 650, 285, 70, 26);
        edit("", 720, 281, 280, 30, 6302);

        label("Thời gian làm", 280, 325, 120, 26);
        edit("", 405, 321, 90, 30, 6303);
        label("phút", 500, 325, 45, 26);
        label("Số lượt", 650, 325, 70, 26);
        edit("", 720, 321, 70, 30, 6307);
        checkBox("Trộn câu", 805, 318, 90, 30, 6310, false);
        checkBox("Trộn đáp án", 900, 318, 100, 30, 6311, false);

        label("Mở lúc", 280, 365, 90, 26);
        edit("", 375, 361, 210, 30, 6304);
        label("Đóng lúc", 650, 365, 70, 26);
        edit("", 720, 361, 175, 30, 6305);
        label("Tự nộp", 900, 365, 58, 26);
        edit("", 958, 361, 42, 30, 6312);

        label("Mã câu hỏi", 280, 405, 110, 26);
        edit("", 405, 401, 605, 30, 6308);
        label("Đáp án file đề", 280, 445, 120, 26);
        edit("", 405, 441, 605, 30, 6309);

        label("Mật khẩu đề", 280, 485, 120, 26);
        edit("", 405, 481, 220, 30, 6306);
        label("Để trống nếu không dùng mật khẩu", 640, 485, 270, 26);

        label("Link file đề", 280, 525, 120, 26);
        edit("", 405, 521, 605, 30, 6313);

        defaultButton("Lưu thay đổi", 465, 565, 150, 36, ID_UPDATE_EXAM_SUBMIT);
        button("Xóa đề", 630, 565, 120, 36, ID_DELETE_EXAM_SUBMIT);
        button("Về dashboard", 765, 565, 150, 36, ID_DASHBOARD);
        setFocusTo(6301);
    }

    void loadExamForEdit() {
        string examId = getText(6301);
        Exam* exam = data.findExamById(examId);
        if (exam == nullptr) {
            error("Không tìm thấy đề thi.");
            return;
        }

        stringstream questionIds;
        for (const string& questionId : exam->getQuestionIds()) {
            if (questionIds.tellp() > 0) questionIds << " ";
            questionIds << questionId;
        }
        stringstream answerKey;
        for (char answer : exam->getAnswerKey()) {
            if (answerKey.tellp() > 0) answerKey << " ";
            answerKey << answer;
        }

        setControlText(GetDlgItem(window, 6302), exam->getTitle());
        setControlText(GetDlgItem(window, 6303), to_string(exam->getDurationMinutes()));
        setControlText(GetDlgItem(window, 6304), exam->getStartAt());
        setControlText(GetDlgItem(window, 6305), exam->getCloseAt());
        setControlText(GetDlgItem(window, 6306), exam->getExamPassword());
        setControlText(GetDlgItem(window, 6307), to_string(exam->getAttemptLimit()));
        setControlText(GetDlgItem(window, 6312), to_string(exam->getViolationLimit()));
        setControlText(GetDlgItem(window, 6308), questionIds.str());
        setControlText(GetDlgItem(window, 6309), answerKey.str());
        setControlText(GetDlgItem(window, 6313), exam->getAttachmentUrl());
        SendMessageW(GetDlgItem(window, 6310), BM_SETCHECK,
                     exam->shouldShuffleQuestions() ? BST_CHECKED : BST_UNCHECKED, 0);
        SendMessageW(GetDlgItem(window, 6311), BM_SETCHECK,
                     exam->shouldShuffleAnswers() ? BST_CHECKED : BST_UNCHECKED, 0);
    }

    void submitUpdateExam() {
        string examId = getText(6301);
        string examTitle = getText(6302);
        int duration = atoi(getText(6303).c_str());
        string startAt = getText(6304);
        string closeAt = getText(6305);
        string examPassword = getText(6306);
        string attachmentUrl = getText(6313);
        int attemptLimit = atoi(getText(6307).c_str());
        int violationLimit = atoi(getText(6312).c_str());
        string questionIdsText = getText(6308);
        string answerKeyText = getText(6309);
        bool shuffleQuestions = isChecked(6310);
        bool shuffleAnswers = isChecked(6311);

        if (examId.empty() || examTitle.empty() || duration <= 0 || attemptLimit <= 0 ||
            violationLimit <= 0 || violationLimit > 20) {
            error("Vui lòng nhập đầy đủ thông tin; số lần Alt+Tab phải từ 1 đến 20.");
            return;
        }
        if (!isDateTimeText(startAt) || !isDateTimeText(closeAt) || closeAt <= startAt) {
            error("Thời gian cần theo dạng YYYY-MM-DD HH:MM và giờ đóng phải sau giờ mở.");
            return;
        }

        vector<string> requestedIds = parseQuestionIds(questionIdsText);
        vector<string> validIds;
        string questionIdError;
        if (!requestedIds.empty() &&
            !data.validateQuestionIds(requestedIds, validIds, questionIdError)) {
            error("Không thể cập nhật đề thi.\r\n" + questionIdError);
            return;
        }
        vector<char> answerKey;
        string answerKeyError;
        if (!answerKeyText.empty() && !parseAnswerKey(answerKeyText, answerKey, answerKeyError)) {
            error(answerKeyError);
            return;
        }
        if (!validIds.empty() && !answerKey.empty()) {
            error("Chỉ dùng mã câu hỏi hoặc đáp án file đề, không dùng đồng thời cả hai.");
            return;
        }
        if (validIds.empty() && answerKey.empty()) {
            error("Đề thi cần có mã câu hỏi hoặc đáp án file đề.");
            return;
        }

        if (!data.updateExamDetails(examId, currentUser->getUserId(), examTitle, duration,
                                    startAt, closeAt, examPassword, attachmentUrl,
                                    attemptLimit, violationLimit,
                                    validIds, answerKey, shuffleQuestions, shuffleAnswers)) {
            error(data.getFirebaseStatus());
            return;
        }

        message("Đã cập nhật nội dung và thời gian đề thi.");
        showManageExam();
    }

    void submitDeleteExam() {
        string examId = getText(6301);
        if (examId.empty()) {
            error("Vui lòng nhập mã đề cần xóa.");
            return;
        }

        int choice = MessageBoxW(window,
                                 L"Bạn có chắc muốn xóa đề thi này không?\nKết quả đã nộp vẫn được giữ lại.",
                                 L"Xác nhận xóa đề",
                                 MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2);
        if (choice != IDYES) {
            return;
        }

        if (!data.deleteExam(examId, currentUser->getUserId())) {
            error(data.getFirebaseStatus());
            return;
        }

        message("Đã xóa đề thi.");
        showManageExam();
    }

    void showTakeExamSelect() {
        clearControls();
        currentScreen = SCREEN_TAKE_EXAM_SELECT;
        title("Làm bài thi");
        examListView(250, 88, 1000, 330, true);
        label("Mã đề thi", 300, 455, 140, 26);
        edit("", 460, 451, 150, 30, 5201);
        label("Mật khẩu de", 300, 500, 140, 26);
        edit("", 460, 496, 150, 30, 5202, true);
        defaultButton("Bắt đầu", 635, 451, 120, 38, ID_TAKE_EXAM_SUBMIT);
        button("Về dashboard", 635, 496, 150, 38, ID_DASHBOARD);
        setFocusTo(5201);
    }

    void showTakeExamForm(Exam& exam) {
        vector<Question*> validatedExamQuestions;
        string questionValidationError;
        if (!exam.isFileAnswerExam() &&
            !data.resolveExamQuestions(exam, validatedExamQuestions,
                                       questionValidationError)) {
            error("Không thể mở đề thi vì dữ liệu câu hỏi không toàn vẹn.\r\n" +
                  questionValidationError +
                  "\r\nVui lòng báo giáo viên kiểm tra lại đề.");
            return;
        }

        clearControls();
        currentScreen = SCREEN_TAKE_EXAM_FORM;
        activeExamId = exam.getExamId();
        title("Làm bài: " + exam.getTitle(),
              "Thời gian làm bài: " + to_string(exam.getDurationMinutes()) +
              " phut | Đóng lúc: " + exam.getCloseAt());
        examTimerLabel = label("", 735, 62, 190, 24);
        if (exam.hasAttachment()) {
            button("Mở file đề", 575, 58, 130, 30, ID_TAKE_EXAM_OPEN_FILE);
        }
        activeExamEndTime = chrono::system_clock::now() + chrono::minutes(exam.getDurationMinutes());
        SetTimer(window, EXAM_TIMER_ID, 1000, nullptr);
        examTimerActive = true;
        updateExamTimer();
        int y = 95;
        int index = 1;
        answerOptions.clear();
        activeAnswerValues.clear();
        answerCombos.clear();
        activeQuestionIds.clear();

        if (exam.isFileAnswerExam()) {
            vector<char> answerKey = exam.getAnswerKey();
            int totalQuestions = (int)answerKey.size();
            int rowsPerBlock = 15;
            int blockCount = min(3, max(1, (totalQuestions + rowsPerBlock - 1) / rowsPerBlock));
            bool compactAnswerSheet = blockCount > 1;
            int answerPanelX = 270;
            int answerPanelY = y;
            int blockWidth = compactAnswerSheet ? 170 : 250;
            int answerPanelW = blockCount * blockWidth + 36;
            int contentX = answerPanelX + answerPanelW + 24;
            int contentY = y;
            int contentW = max(380, 1240 - contentX);

            HWND answerTitle = label("Bảng trả lời", answerPanelX + 18, answerPanelY + 16, 190, 28);
            SendMessageW(answerTitle, WM_SETFONT, (WPARAM)brandFont, TRUE);
            HWND answerHint = label("Chọn đáp án cho từng câu", answerPanelX + 18, answerPanelY + 48,
                                    answerPanelW - 36, 22);
            SendMessageW(answerHint, WM_SETFONT, (WPARAM)smallFont, TRUE);

            HWND contentTitle = label("Nội dung đề", contentX, contentY, 160, 24);
            SendMessageW(contentTitle, WM_SETFONT, (WPARAM)brandFont, TRUE);
            addControl("STATIC", "", SS_ETCHEDFRAME, contentX, contentY + 30, contentW, 430, 0, false);
            showInlineExamAttachment(exam, contentX + 15, contentY + 45, contentW - 30, 400);

            int tableY = answerPanelY + 82;
            int rowHeight = totalQuestions > 15 ? 23 : 27;
            int questionW = compactAnswerSheet ? 38 : 58;
            int optionW = compactAnswerSheet ? 28 : 42;
            int optionGap = compactAnswerSheet ? 4 : 10;

            for (int block = 0; block < blockCount; block++) {
                int startIndex = block * rowsPerBlock;
                if (startIndex >= (int)answerKey.size()) {
                    break;
                }

                int x = answerPanelX + 18 + block * blockWidth;
                HWND questionHeader = label("Câu", x, tableY, questionW, 24);
                SendMessageW(questionHeader, WM_SETFONT, (WPARAM)smallFont, TRUE);
                int optionHeaderX = x + questionW;
                label("A", optionHeaderX + 6, tableY, optionW, 24);
                label("B", optionHeaderX + (optionW + optionGap) + 6, tableY, optionW, 24);
                label("C", optionHeaderX + (optionW + optionGap) * 2 + 6, tableY, optionW, 24);
                label("D", optionHeaderX + (optionW + optionGap) * 3 + 6, tableY, optionW, 24);

                for (int row = 0; row < rowsPerBlock; row++) {
                    int index = startIndex + row;
                    if (index >= (int)answerKey.size()) {
                        break;
                    }

                    int itemY = tableY + 26 + row * rowHeight;
                    HWND questionNumber = label(to_string(index + 1), x, itemY + 1, questionW, 21);
                    SendMessageW(questionNumber, WM_SETFONT, (WPARAM)smallFont, TRUE);
                    int optionX = x + questionW;
                    array<HWND, 4> buttons = {
                        answerChoiceButton("A", optionX, itemY, optionW, 21, 5400 + index * 10 + 0, true),
                        answerChoiceButton("B", optionX + (optionW + optionGap), itemY, optionW, 21, 5400 + index * 10 + 1, false),
                        answerChoiceButton("C", optionX + (optionW + optionGap) * 2, itemY, optionW, 21, 5400 + index * 10 + 2, false),
                        answerChoiceButton("D", optionX + (optionW + optionGap) * 3, itemY, optionW, 21, 5400 + index * 10 + 3, false),
                    };
                    answerOptions.push_back(buttons);
                    activeAnswerValues.push_back({'A', 'B', 'C', 'D'});
                    activeQuestionIds.push_back("c" + to_string(index + 1));
                }
            }

            defaultButton("Nộp bài", 720, 560, 120, 36, ID_TAKE_EXAM_SUBMIT);
            button("Về dashboard", 560, 560, 145, 36, ID_DASHBOARD);
            if (!answerOptions.empty()) {
                SetFocus(answerOptions[0][0]);
            }
            return;
        }

        vector<Question*> examQuestions = validatedExamQuestions;
        random_device randomDevice;
        mt19937 randomEngine(randomDevice());
        if (exam.shouldShuffleQuestions()) {
            shuffle(examQuestions.begin(), examQuestions.end(), randomEngine);
        }
        vector<array<string, 4>> displayedOptions;
        vector<array<char, 4>> displayedAnswerValues;
        for (Question* question : examQuestions) {
            array<string, 4> originalOptions = question->getOptions();
            array<int, 4> order = {0, 1, 2, 3};
            if (exam.shouldShuffleAnswers()) {
                shuffle(order.begin(), order.end(), randomEngine);
            }
            array<string, 4> options;
            array<char, 4> values;
            for (int optionIndex = 0; optionIndex < 4; optionIndex++) {
                options[optionIndex] = originalOptions[order[optionIndex]];
                values[optionIndex] = (char)('A' + order[optionIndex]);
            }
            displayedOptions.push_back(options);
            displayedAnswerValues.push_back(values);
        }

        bool hasImageQuestion = any_of(
            examQuestions.begin(), examQuestions.end(),
            [](Question* question) {
                return question != nullptr && question->hasImage() &&
                       !question->getImagePath().empty();
            }
        );
        if (hasImageQuestion && examQuestions.size() <= 5) {
            activeExamQuestionOrder.clear();
            activeExamDisplayedOptions.clear();
            activeExamDisplayedValues.clear();
            activeExamQuestionPages.clear();
            activeExamQuestionPage = 0;

            vector<int> theoryPage;
            for (int questionIndex = 0; questionIndex < (int)examQuestions.size(); questionIndex++) {
                Question* question = examQuestions[questionIndex];
                activeExamQuestionOrder.push_back(question->getQuestionId());
                activeExamDisplayedOptions[question->getQuestionId()] =
                    displayedOptions[questionIndex];
                activeExamDisplayedValues[question->getQuestionId()] =
                    displayedAnswerValues[questionIndex];

                bool hasImage = question->hasImage() && !question->getImagePath().empty();
                if (hasImage) {
                    if (!theoryPage.empty()) {
                        activeExamQuestionPages.push_back(theoryPage);
                        theoryPage.clear();
                    }
                    activeExamQuestionPages.push_back({questionIndex});
                } else {
                    theoryPage.push_back(questionIndex);
                    if (theoryPage.size() == 3) {
                        activeExamQuestionPages.push_back(theoryPage);
                        theoryPage.clear();
                    }
                }
            }
            if (!theoryPage.empty()) {
                activeExamQuestionPages.push_back(theoryPage);
            }
            renderPagedExamQuestions(exam);
            return;
        }

        if (examQuestions.size() > 5) {
            int rowsPerBlock = 15;
            int blockCount = min(3, max(1, ((int)examQuestions.size() + rowsPerBlock - 1) / rowsPerBlock));
            int answerPanelX = 270;
            int answerPanelY = y;
            int blockWidth = 170;
            int answerPanelW = blockCount * blockWidth + 36;
            int contentX = answerPanelX + answerPanelW + 24;
            int contentY = y;
            int contentW = max(380, 1240 - contentX);

            HWND answerTitle = label("Bảng trả lời", answerPanelX + 18, answerPanelY + 16, 190, 28);
            SendMessageW(answerTitle, WM_SETFONT, (WPARAM)brandFont, TRUE);
            HWND answerHint = label("Chọn đáp án cho từng câu", answerPanelX + 18, answerPanelY + 48,
                                    answerPanelW - 36, 22);
            SendMessageW(answerHint, WM_SETFONT, (WPARAM)smallFont, TRUE);

            HWND contentTitle = label("Nội dung câu hỏi", contentX, contentY, 190, 24);
            SendMessageW(contentTitle, WM_SETFONT, (WPARAM)brandFont, TRUE);
            stringstream questionText;
            for (int questionIndex = 0; questionIndex < (int)examQuestions.size(); questionIndex++) {
                Question* question = examQuestions[questionIndex];
                array<string, 4> options = displayedOptions[questionIndex];
                questionText << "Câu " << questionIndex + 1 << ". " << question->getContent() << "\r\n";
                for (int optionIndex = 0; optionIndex < 4; optionIndex++) {
                    questionText << "   " << (char)('A' + optionIndex) << ". "
                                 << options[optionIndex] << "\r\n";
                }
                questionText << "\r\n";
            }
            protectedExamText(questionText.str(), contentX, contentY + 30, contentW, 430);

            int tableY = answerPanelY + 82;
            int rowHeight = 23;
            int questionW = 38;
            int optionW = 28;
            int optionGap = 4;

            for (int block = 0; block < blockCount; block++) {
                int startIndex = block * rowsPerBlock;
                if (startIndex >= (int)examQuestions.size()) {
                    break;
                }

                int x = answerPanelX + 18 + block * blockWidth;
                HWND questionHeader = label("Câu", x, tableY, questionW, 24);
                SendMessageW(questionHeader, WM_SETFONT, (WPARAM)smallFont, TRUE);
                int optionHeaderX = x + questionW;
                label("A", optionHeaderX + 6, tableY, optionW, 24);
                label("B", optionHeaderX + (optionW + optionGap) + 6, tableY, optionW, 24);
                label("C", optionHeaderX + (optionW + optionGap) * 2 + 6, tableY, optionW, 24);
                label("D", optionHeaderX + (optionW + optionGap) * 3 + 6, tableY, optionW, 24);

                for (int row = 0; row < rowsPerBlock; row++) {
                    int questionIndex = startIndex + row;
                    if (questionIndex >= (int)examQuestions.size()) {
                        break;
                    }

                    int itemY = tableY + 26 + row * rowHeight;
                    HWND questionNumber = label(to_string(questionIndex + 1), x, itemY + 1, questionW, 21);
                    SendMessageW(questionNumber, WM_SETFONT, (WPARAM)smallFont, TRUE);
                    int optionX = x + questionW;
                    array<HWND, 4> buttons = {
                        answerChoiceButton("A", optionX, itemY, optionW, 21, 5300 + questionIndex * 10 + 0, true),
                        answerChoiceButton("B", optionX + (optionW + optionGap), itemY, optionW, 21, 5300 + questionIndex * 10 + 1, false),
                        answerChoiceButton("C", optionX + (optionW + optionGap) * 2, itemY, optionW, 21, 5300 + questionIndex * 10 + 2, false),
                        answerChoiceButton("D", optionX + (optionW + optionGap) * 3, itemY, optionW, 21, 5300 + questionIndex * 10 + 3, false),
                    };
                    answerOptions.push_back(buttons);
                    activeAnswerValues.push_back(displayedAnswerValues[questionIndex]);
                    activeQuestionIds.push_back(examQuestions[questionIndex]->getQuestionId());
                }
            }

            defaultButton("Nộp bài", 720, 560, 120, 36, ID_TAKE_EXAM_SUBMIT);
            button("Về dashboard", 560, 560, 145, 36, ID_DASHBOARD);
            if (!answerOptions.empty()) {
                SetFocus(answerOptions[0][0]);
            }
            return;
        }

        RECT examClient = {};
        GetClientRect(window, &examClient);
        RECT examWindow = {};
        GetWindowRect(window, &examWindow);
        int visibleClientWidth = max(760, (int)(examWindow.right - examWindow.left) - 16);
        int visibleClientHeight = max(560, (int)(examWindow.bottom - examWindow.top) - 40);
        int layoutWidth = min((int)examClient.right, visibleClientWidth);
        int layoutHeight = min((int)examClient.bottom, visibleClientHeight);
        int contentLeft = 270;
        int contentRight = max(contentLeft + 700, layoutWidth - 36);
        int contentWidth = contentRight - contentLeft;
        int columnGap = 20;
        int optionWidth = min(410, (contentWidth - 30 - columnGap) / 2);
        int optionLeft = contentLeft + 30;
        int secondColumnLeft = optionLeft + optionWidth + columnGap;
        int actionY = min(560, max(520, layoutHeight - 80));
        int availableQuestionHeight = max(360, actionY - y - 16);
        int imageQuestionCount = 0;
        for (Question* question : examQuestions) {
            if (question != nullptr && question->hasImage() &&
                !question->getImagePath().empty()) {
                imageQuestionCount++;
            }
        }
        int imageBlockExtra = 55;
        int questionBlockHeight = examQuestions.empty()
                                      ? 96
                                      : min(104, max(76,
                                            (availableQuestionHeight -
                                             imageQuestionCount * imageBlockExtra) /
                                                (int)examQuestions.size()));
        int optionRowGap = 5;
        int questionToOptionsGap = 26;
        int optionHeight = min(32, max(23, (questionBlockHeight -
                                            questionToOptionsGap - optionRowGap - 4) / 2));

        for (int questionIndex = 0; questionIndex < (int)examQuestions.size(); questionIndex++) {
            Question* question = examQuestions[questionIndex];
            bool hasQuestionImage = question != nullptr && question->hasImage() &&
                                    !question->getImagePath().empty();
            int currentBlockHeight = questionBlockHeight +
                                     (hasQuestionImage ? imageBlockExtra : 0);
            if (question == nullptr || y + currentBlockHeight > actionY) continue;

            string line = to_string(index) + ". " + question->getContent();
            HWND questionLabel = label(line, contentLeft, y, contentWidth, 26);
            SendMessageW(questionLabel, WM_SETFONT, (WPARAM)brandFont, TRUE);
            int optionY = y + questionToOptionsGap;
            int currentOptionLeft = optionLeft;
            int currentSecondColumnLeft = secondColumnLeft;
            int currentOptionWidth = optionWidth;
            if (hasQuestionImage) {
                int imageWidth = min(220, max(170, contentWidth / 5));
                int imageHeight = max(90, min(130,
                    currentBlockHeight - questionToOptionsGap - 8));
                HWND image = imageBox(question->getImagePath(), optionLeft, optionY,
                                      imageWidth, imageHeight, true);
                if (image != nullptr) {
                    int imageGap = 16;
                    currentOptionLeft = optionLeft + imageWidth + imageGap;
                    currentOptionWidth = max(
                        220,
                        min(optionWidth,
                            (contentRight - currentOptionLeft - columnGap) / 2)
                    );
                    currentSecondColumnLeft =
                        currentOptionLeft + currentOptionWidth + columnGap;
                }
            }
            array<string, 4> options = displayedOptions[questionIndex];
            array<HWND, 4> buttons = {
                answerChoiceButton("A. " + options[0], currentOptionLeft, optionY,
                                   currentOptionWidth, optionHeight,
                                   5300 + index * 10 + 0, true),
                answerChoiceButton("B. " + options[1], currentSecondColumnLeft, optionY,
                                   currentOptionWidth, optionHeight,
                                   5300 + index * 10 + 1, false),
                answerChoiceButton("C. " + options[2], currentOptionLeft,
                                   optionY + optionHeight + optionRowGap,
                                   currentOptionWidth, optionHeight,
                                   5300 + index * 10 + 2, false),
                answerChoiceButton("D. " + options[3], currentSecondColumnLeft,
                                   optionY + optionHeight + optionRowGap,
                                   currentOptionWidth, optionHeight,
                                   5300 + index * 10 + 3, false),
            };
            answerOptions.push_back(buttons);
            activeAnswerValues.push_back(displayedAnswerValues[questionIndex]);
            activeQuestionIds.push_back(question->getQuestionId());
            y += currentBlockHeight;
            index++;
        }

        int submitWidth = 130;
        int backWidth = 150;
        int backX = 560;
        int submitX = 720;
        button("Về dashboard", backX, actionY, backWidth, 38, ID_DASHBOARD);
        defaultButton("Nộp bài", submitX, actionY, submitWidth, 38, ID_TAKE_EXAM_SUBMIT);
        if (!answerOptions.empty()) {
            SetFocus(answerOptions[0][0]);
        }
    }

    void clearExamPageVisuals() {
        answerOptions.clear();
        activeAnswerValues.clear();
        answerCombos.clear();
        activeQuestionIds.clear();
        examTimerLabel = nullptr;
        primaryButtons.clear();
        navButtons.clear();
        answerChoiceButtons.clear();
        controlTextColors.clear();
        surfaceLabels.clear();
        sidebarLabels.clear();
        for (IPreviewHandler* handler : previewHandlers) {
            if (handler != nullptr) {
                handler->Unload();
                handler->Release();
            }
        }
        previewHandlers.clear();
        previewHandlerByHost.clear();
        for (HWND control : controls) {
            DestroyWindow(control);
        }
        controls.clear();
        for (HBITMAP bitmap : imageBitmaps) {
            DeleteObject(bitmap);
        }
        imageBitmaps.clear();
        for (HIMAGELIST imageList : listImageLists) {
            ImageList_Destroy(imageList);
        }
        listImageLists.clear();
    }

    void restorePagedAnswerSelection(const string& questionId,
                                     const array<HWND, 4>& buttons,
                                     const array<char, 4>& values) {
        auto saved = activeExamDraftAnswers.find(questionId);
        if (saved == activeExamDraftAnswers.end()) {
            return;
        }
        for (int optionIndex = 0; optionIndex < 4; optionIndex++) {
            bool selected = values[optionIndex] == saved->second;
            answerChoiceButtons[buttons[optionIndex]] = selected;
            SendMessageW(buttons[optionIndex], BM_SETCHECK,
                         selected ? BST_CHECKED : BST_UNCHECKED, 0);
        }
    }

    void renderPagedExamQuestions(Exam& exam) {
        clearExamPageVisuals();
        currentScreen = SCREEN_TAKE_EXAM_FORM;
        activeExamId = exam.getExamId();
        title("Làm bài: " + exam.getTitle(),
              "Thời gian làm bài: " + to_string(exam.getDurationMinutes()) +
              " phút | Đóng lúc: " + exam.getCloseAt());
        examTimerLabel = label("", 735, 62, 190, 24);
        updateExamTimer();

        if (activeExamQuestionPages.empty()) {
            error("Không có câu hỏi để hiển thị.");
            return;
        }
        activeExamQuestionPage = max(
            0, min(activeExamQuestionPage, (int)activeExamQuestionPages.size() - 1)
        );
        const vector<int>& page = activeExamQuestionPages[activeExamQuestionPage];
        bool imagePage = page.size() == 1;
        Question* firstQuestion = nullptr;
        if (imagePage && page[0] < (int)activeExamQuestionOrder.size()) {
            firstQuestion = data.findQuestionById(activeExamQuestionOrder[page[0]]);
            imagePage = firstQuestion != nullptr && firstQuestion->hasImage() &&
                        !firstQuestion->getImagePath().empty();
        }

        if (imagePage) {
            int questionIndex = page[0];
            string questionId = activeExamQuestionOrder[questionIndex];
            Question* question = data.findQuestionById(questionId);
            HWND questionLabel = label(
                to_string(questionIndex + 1) + ". " + question->getContent(),
                270, 102, 930, 30
            );
            SendMessageW(questionLabel, WM_SETFONT, (WPARAM)brandFont, TRUE);

            imageBox(question->getImagePath(), 285, 145, 470, 350, true);
            array<string, 4> options = activeExamDisplayedOptions[questionId];
            array<char, 4> values = activeExamDisplayedValues[questionId];
            int optionX = 785;
            int optionY = 155;
            int optionW = 410;
            int optionH = 52;
            int optionGap = 14;
            array<HWND, 4> buttons = {
                answerChoiceButton("A. " + options[0], optionX, optionY,
                                   optionW, optionH, 5300 + questionIndex * 10, true),
                answerChoiceButton("B. " + options[1], optionX,
                                   optionY + (optionH + optionGap), optionW, optionH,
                                   5300 + questionIndex * 10 + 1, false),
                answerChoiceButton("C. " + options[2], optionX,
                                   optionY + (optionH + optionGap) * 2, optionW, optionH,
                                   5300 + questionIndex * 10 + 2, false),
                answerChoiceButton("D. " + options[3], optionX,
                                   optionY + (optionH + optionGap) * 3, optionW, optionH,
                                   5300 + questionIndex * 10 + 3, false),
            };
            answerOptions.push_back(buttons);
            activeAnswerValues.push_back(values);
            activeQuestionIds.push_back(questionId);
            restorePagedAnswerSelection(questionId, buttons, values);
        } else {
            int y = 110;
            for (int questionIndex : page) {
                if (questionIndex >= (int)activeExamQuestionOrder.size()) {
                    continue;
                }
                string questionId = activeExamQuestionOrder[questionIndex];
                Question* question = data.findQuestionById(questionId);
                if (question == nullptr) {
                    continue;
                }
                HWND questionLabel = label(
                    to_string(questionIndex + 1) + ". " + question->getContent(),
                    270, y, 930, 28
                );
                SendMessageW(questionLabel, WM_SETFONT, (WPARAM)brandFont, TRUE);
                array<string, 4> options = activeExamDisplayedOptions[questionId];
                array<char, 4> values = activeExamDisplayedValues[questionId];
                array<HWND, 4> buttons = {
                    answerChoiceButton("A. " + options[0], 300, y + 34, 410, 34,
                                       5300 + questionIndex * 10, true),
                    answerChoiceButton("B. " + options[1], 730, y + 34, 410, 34,
                                       5300 + questionIndex * 10 + 1, false),
                    answerChoiceButton("C. " + options[2], 300, y + 76, 410, 34,
                                       5300 + questionIndex * 10 + 2, false),
                    answerChoiceButton("D. " + options[3], 730, y + 76, 410, 34,
                                       5300 + questionIndex * 10 + 3, false),
                };
                answerOptions.push_back(buttons);
                activeAnswerValues.push_back(values);
                activeQuestionIds.push_back(questionId);
                restorePagedAnswerSelection(questionId, buttons, values);
                y += 138;
            }
        }

        string pageText = "Trang " + to_string(activeExamQuestionPage + 1) + "/" +
                          to_string(activeExamQuestionPages.size());
        centeredLabel(pageText, 570, 560, 110, 38, smallFont, THEME_MUTED);
        if (activeExamQuestionPage > 0) {
            button("Trang trước", 430, 560, 125, 38, ID_EXAM_PREVIOUS_PAGE);
        }
        if (activeExamQuestionPage + 1 < (int)activeExamQuestionPages.size()) {
            defaultButton("Trang tiếp", 695, 560, 125, 38, ID_EXAM_NEXT_PAGE);
        }
        button("Về dashboard", 840, 560, 145, 38, ID_DASHBOARD);
        defaultButton("Nộp bài", 1000, 560, 120, 38, ID_TAKE_EXAM_SUBMIT);
        if (!answerOptions.empty()) {
            SetFocus(answerOptions[0][0]);
        }
    }

    void changeExamQuestionPage(int direction) {
        Exam* exam = data.findExamById(activeExamId);
        if (exam == nullptr || activeExamQuestionPages.empty()) {
            return;
        }
        int nextPage = activeExamQuestionPage + direction;
        if (nextPage < 0 || nextPage >= (int)activeExamQuestionPages.size()) {
            return;
        }
        activeExamQuestionPage = nextPage;
        renderPagedExamQuestions(*exam);
    }

    void openActiveExamAttachment() {
        Exam* exam = data.findExamById(activeExamId);
        if (exam == nullptr) {
            error("Không tìm thấy đề thi đang làm.");
            return;
        }

        string target;
        if (localExamFileExists(exam->getAttachmentPath())) {
            target = exam->getAttachmentPath();
        } else if (!exam->getAttachmentUrl().empty()) {
            string cachedPath = cachedExamAttachmentPath(*exam);
            if (!cachedPath.empty() && localExamFileExists(cachedPath)) {
                target = cachedPath;
            } else if (!cachedPath.empty()) {
                SetCursor(LoadCursor(nullptr, IDC_WAIT));
                bool downloaded = downloadExamAttachment(*exam, cachedPath);
                SetCursor(LoadCursor(nullptr, IDC_ARROW));
                target = downloaded ? cachedPath : exam->getAttachmentUrl();
            } else {
                target = exam->getAttachmentUrl();
            }
        }

        if (target.empty()) {
            error("File đề chỉ tồn tại trên máy giáo viên và đề chưa có URL dùng chung.\r\n"
                  "Giáo viên cần vào Quản lý đề thi và thêm Link file đề công khai.");
            return;
        }

        allowExamViewerFocusLossUntil = chrono::steady_clock::now() + chrono::seconds(5);
        wstring wideTarget = utf8ToWide(target);
        HINSTANCE result = ShellExecuteW(window, L"open", wideTarget.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
        if ((INT_PTR)result <= 32) {
            allowExamViewerFocusLossUntil = chrono::steady_clock::time_point{};
            error("Không mở được file/link đề thi. Hãy kiểm tra URL có công khai và cho phép tải xuống.");
        }
    }

    void showInlineExamAttachment(Exam& exam, int x, int y, int w, int h) {
        string localPath = exam.getAttachmentPath();
        string url = exam.getAttachmentUrl();
        if (!localExamFileExists(localPath)) {
            string cachedPath = cachedExamAttachmentPath(exam);
            localPath = localExamFileExists(cachedPath) ? cachedPath : "";
        }

        if (!localPath.empty() && isImageFile(localPath)) {
            HWND image = imageBox(localPath, x, y, w, h);
            if (image != nullptr) {
                return;
            }
        }

        if (!localPath.empty() && isDocxFile(localPath)) {
            string previewText;
            if (loadDocxPreviewText(localPath, previewText)) {
                protectedExamText(previewText, x, y, w, h);
                return;
            }

            label("Không đọc được nội dung DOCX trong app.", x + 20, y + 30, w - 40, 26);
            label("Bạn vẫn có thể bấm Mở file đề để xem bằng Word.", x + 20, y + 62, w - 40, 26);
            return;
        }

        if (!localPath.empty() && isPdfFile(localPath)) {
            label("File PDF không xem trực tiếp trong khung này để tránh lỗi trắng/không kéo được.", x + 20, y + 30, w - 40, 26);
            label("Bấm Mở file đề để xem bằng Microsoft Edge, Chrome hoặc Adobe Reader.", x + 20, y + 62, w - 40, 26);
            label("Muốn xem ngay trong app thì giáo viên nên dùng file ảnh .jpg/.png.", x + 20, y + 94, w - 40, 26);
            return;
        }

        if (!url.empty() && isImageFile(url)) {
            label("Link ảnh công khai chưa hỗ trợ xem trực tiếp trong khung.", x + 20, y + 30, w - 40, 26);
            label("Bấm Mở file đề để xem bằng trình duyệt.", x + 20, y + 62, w - 40, 26);
            return;
        }

        if (!exam.getAttachmentPath().empty() && localPath.empty() && url.empty()) {
            label("File đề đang nằm trên máy giáo viên nên máy này không thể mở.", x + 20, y + 30, w - 40, 26);
            label("Giáo viên cần thêm Link file đề công khai trong Quản lý đề thi.", x + 20, y + 62, w - 40, 26);
            return;
        }

        if (!localPath.empty() || !url.empty()) {
            label("File Word/PDF hiện chưa xem trực tiếp trong khung này.", x + 20, y + 30, w - 40, 26);
            label("Bấm Mở file đề để mở bằng Word/PDF viewer.", x + 20, y + 62, w - 40, 26);
            label("Nếu muốn xem ngay trong app, hãy dùng file ảnh .jpg/.png.", x + 20, y + 94, w - 40, 26);
            return;
        }

        label("Đề này chưa có file đính kèm.", x + 20, y + 30, w - 40, 26);
    }

    bool localExamFileExists(const string& path) {
        if (path.empty()) {
            return false;
        }
        DWORD attributes = GetFileAttributesA(path.c_str());
        return attributes != INVALID_FILE_ATTRIBUTES &&
               (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
    }

    string directAttachmentExtension(string url) {
        size_t query = url.find_first_of("?#");
        if (query != string::npos) {
            url = url.substr(0, query);
        }
        transform(url.begin(), url.end(), url.begin(),
                  [](unsigned char ch) { return (char)tolower(ch); });
        const array<string, 5> supported = {".pdf", ".docx", ".png", ".jpg", ".jpeg"};
        for (const string& extension : supported) {
            if (endsWith(url, extension)) {
                return extension;
            }
        }
        return "";
    }

    string cachedExamAttachmentPath(const Exam& exam) {
        string extension = directAttachmentExtension(exam.getAttachmentUrl());
        if (extension.empty()) {
            return "";
        }

        char localAppData[MAX_PATH] = "";
        DWORD length = GetEnvironmentVariableA("LOCALAPPDATA", localAppData, MAX_PATH);
        string root = length > 0 && length < MAX_PATH ? string(localAppData) : appDirectory();
        string appFolder = root + "\\HCMUTEQuizApp";
        string cacheFolder = appFolder + "\\exam_cache";
        CreateDirectoryA(appFolder.c_str(), nullptr);
        CreateDirectoryA(cacheFolder.c_str(), nullptr);
        return cacheFolder + "\\" + exam.getExamId() + extension;
    }

    bool downloadExamAttachment(const Exam& exam, const string& targetPath) {
        string bytes;
        if (!data.downloadTextFromUrl(exam.getAttachmentUrl(), bytes) || bytes.empty()) {
            return false;
        }

        string extension = directAttachmentExtension(exam.getAttachmentUrl());
        bool validFile =
            (extension == ".pdf" && bytes.size() >= 4 && bytes.compare(0, 4, "%PDF") == 0) ||
            (extension == ".docx" && bytes.size() >= 2 && bytes[0] == 'P' && bytes[1] == 'K') ||
            (extension == ".png" && bytes.size() >= 8 &&
             (unsigned char)bytes[0] == 0x89 && bytes.compare(1, 3, "PNG") == 0) ||
            ((extension == ".jpg" || extension == ".jpeg") && bytes.size() >= 2 &&
             (unsigned char)bytes[0] == 0xFF && (unsigned char)bytes[1] == 0xD8);
        if (!validFile) {
            return false;
        }

        ofstream output(targetPath, ios::binary);
        if (!output) {
            return false;
        }
        output.write(bytes.data(), (streamsize)bytes.size());
        return output.good();
    }

    void trimInPlace(string& text) {
        while (!text.empty() && isspace((unsigned char)text.front())) {
            text.erase(text.begin());
        }
        while (!text.empty() && isspace((unsigned char)text.back())) {
            text.pop_back();
        }
    }

    bool loadDocxPreviewText(string path, string& previewText) {
        vector<unsigned char> fileData;
        if (!readBinaryFile(path, fileData)) {
            return false;
        }

        vector<unsigned char> xmlData;
        if (!extractZipEntry(fileData, "word/document.xml", xmlData)) {
            return false;
        }

        string xml(xmlData.begin(), xmlData.end());
        previewText = docxXmlToText(xml);
        trimInPlace(previewText);
        return !previewText.empty();
    }

    bool readBinaryFile(string path, vector<unsigned char>& data) {
        ifstream file(path, ios::binary);
        if (!file) {
            return false;
        }

        data.assign(istreambuf_iterator<char>(file), istreambuf_iterator<char>());
        return !data.empty();
    }

    uint16_t readU16(const vector<unsigned char>& data, size_t offset) {
        if (offset + 2 > data.size()) {
            return 0;
        }
        return (uint16_t)data[offset] | ((uint16_t)data[offset + 1] << 8);
    }

    uint32_t readU32(const vector<unsigned char>& data, size_t offset) {
        if (offset + 4 > data.size()) {
            return 0;
        }
        return (uint32_t)data[offset] |
               ((uint32_t)data[offset + 1] << 8) |
               ((uint32_t)data[offset + 2] << 16) |
               ((uint32_t)data[offset + 3] << 24);
    }

    bool extractZipEntry(const vector<unsigned char>& zipData, string wantedName, vector<unsigned char>& output) {
        if (zipData.size() < 22) {
            return false;
        }

        size_t searchStart = zipData.size() > 66000 ? zipData.size() - 66000 : 0;
        size_t eocd = string::npos;
        for (size_t i = zipData.size() - 22; i + 1 > searchStart; i--) {
            if (readU32(zipData, i) == 0x06054b50) {
                eocd = i;
                break;
            }
            if (i == 0) {
                break;
            }
        }
        if (eocd == string::npos) {
            return false;
        }

        uint16_t entryCount = readU16(zipData, eocd + 10);
        uint32_t centralOffset = readU32(zipData, eocd + 16);
        size_t pos = centralOffset;

        for (uint16_t entryIndex = 0; entryIndex < entryCount && pos + 46 <= zipData.size(); entryIndex++) {
            if (readU32(zipData, pos) != 0x02014b50) {
                return false;
            }

            uint16_t method = readU16(zipData, pos + 10);
            uint32_t compressedSize = readU32(zipData, pos + 20);
            uint32_t uncompressedSize = readU32(zipData, pos + 24);
            uint16_t nameLen = readU16(zipData, pos + 28);
            uint16_t extraLen = readU16(zipData, pos + 30);
            uint16_t commentLen = readU16(zipData, pos + 32);
            uint32_t localOffset = readU32(zipData, pos + 42);

            if (pos + 46 + nameLen > zipData.size()) {
                return false;
            }

            string name((const char*)&zipData[pos + 46], nameLen);
            if (name == wantedName) {
                return inflateZipEntry(zipData, localOffset, method, compressedSize, uncompressedSize, output);
            }

            pos += 46 + nameLen + extraLen + commentLen;
        }

        return false;
    }

    bool inflateZipEntry(const vector<unsigned char>& zipData, uint32_t localOffset, uint16_t method,
                         uint32_t compressedSize, uint32_t uncompressedSize, vector<unsigned char>& output) {
        size_t pos = localOffset;
        if (pos + 30 > zipData.size() || readU32(zipData, pos) != 0x04034b50) {
            return false;
        }

        uint16_t nameLen = readU16(zipData, pos + 26);
        uint16_t extraLen = readU16(zipData, pos + 28);
        size_t dataOffset = pos + 30 + nameLen + extraLen;
        if (dataOffset + compressedSize > zipData.size()) {
            return false;
        }

        const unsigned char* compressed = zipData.data() + dataOffset;
        if (method == 0) {
            output.assign(compressed, compressed + compressedSize);
            return true;
        }

        if (method != 8) {
            return false;
        }

        output.assign(uncompressedSize, 0);
        z_stream stream = {};
        stream.next_in = (Bytef*)compressed;
        stream.avail_in = compressedSize;
        stream.next_out = output.data();
        stream.avail_out = uncompressedSize;

        if (inflateInit2(&stream, -MAX_WBITS) != Z_OK) {
            return false;
        }

        int result = inflate(&stream, Z_FINISH);
        inflateEnd(&stream);
        if (result != Z_STREAM_END) {
            return false;
        }

        output.resize(stream.total_out);
        return true;
    }

    string docxXmlToText(const string& xml) {
        string text;
        string buffer;
        for (size_t i = 0; i < xml.size();) {
            if (xml[i] == '<') {
                size_t end = xml.find('>', i);
                if (end == string::npos) {
                    break;
                }

                string tag = xml.substr(i + 1, end - i - 1);
                if (tag.rfind("/w:p", 0) == 0 || tag.rfind("w:br", 0) == 0 ||
                    tag.rfind("w:cr", 0) == 0 || tag.rfind("/w:tr", 0) == 0) {
                    appendPreviewLine(text, buffer);
                } else if (tag.rfind("w:tab", 0) == 0) {
                    buffer += "    ";
                }
                i = end + 1;
            } else {
                buffer += xml[i];
                i++;
            }
        }

        appendPreviewLine(text, buffer);
        return text;
    }

    void appendPreviewLine(string& text, string& buffer) {
        string line = decodeXmlEntities(buffer);
        trimInPlace(line);
        if (!line.empty()) {
            text += line + "\r\n";
        }
        buffer.clear();
    }

    string decodeXmlEntities(string value) {
        string result;
        for (size_t i = 0; i < value.size();) {
            if (value[i] == '&') {
                size_t semi = value.find(';', i + 1);
                if (semi != string::npos) {
                    string entity = value.substr(i + 1, semi - i - 1);
                    if (entity == "amp") result += "&";
                    else if (entity == "lt") result += "<";
                    else if (entity == "gt") result += ">";
                    else if (entity == "quot") result += "\"";
                    else if (entity == "apos") result += "'";
                    else if (!entity.empty() && entity[0] == '#') result += decodeNumericEntity(entity);
                    else result += "&" + entity + ";";
                    i = semi + 1;
                    continue;
                }
            }

            result += value[i];
            i++;
        }
        return result;
    }

    string decodeNumericEntity(string entity) {
        int codepoint = 0;
        try {
            if (entity.size() > 2 && (entity[1] == 'x' || entity[1] == 'X')) {
                codepoint = stoi(entity.substr(2), nullptr, 16);
            } else {
                codepoint = stoi(entity.substr(1), nullptr, 10);
            }
        } catch (...) {
            return "";
        }

        return utf8FromCodepoint(codepoint);
    }

    string utf8FromCodepoint(int codepoint) {
        string out;
        if (codepoint <= 0x7F) {
            out += (char)codepoint;
        } else if (codepoint <= 0x7FF) {
            out += (char)(0xC0 | ((codepoint >> 6) & 0x1F));
            out += (char)(0x80 | (codepoint & 0x3F));
        } else if (codepoint <= 0xFFFF) {
            out += (char)(0xE0 | ((codepoint >> 12) & 0x0F));
            out += (char)(0x80 | ((codepoint >> 6) & 0x3F));
            out += (char)(0x80 | (codepoint & 0x3F));
        } else {
            out += (char)(0xF0 | ((codepoint >> 18) & 0x07));
            out += (char)(0x80 | ((codepoint >> 12) & 0x3F));
            out += (char)(0x80 | ((codepoint >> 6) & 0x3F));
            out += (char)(0x80 | (codepoint & 0x3F));
        }
        return out;
    }

    bool isImageFile(string path) {
        string value = lowerText(path);
        return endsWith(value, ".jpg") ||
               endsWith(value, ".jpeg") ||
               endsWith(value, ".png") ||
               endsWith(value, ".bmp");
    }

    bool isDocxFile(string path) {
        return endsWith(lowerText(path), ".docx");
    }

    bool isPdfFile(string path) {
        return endsWith(lowerText(path), ".pdf");
    }

    bool endsWith(string text, string suffix) {
        return text.size() >= suffix.size() &&
               text.compare(text.size() - suffix.size(), suffix.size(), suffix) == 0;
    }

    void recordExamViolation(string reason) {
        if (currentScreen != SCREEN_TAKE_EXAM_FORM || activeExamId.empty() ||
            currentUser == nullptr || examSubmissionInProgress) {
            return;
        }

        Exam* exam = data.findExamById(activeExamId);
        int violationLimit = exam == nullptr ? 3 : max(1, exam->getViolationLimit());
        examViolationCount++;
        data.writeAuditLog(currentUser->getUserId(), activeExamId, "VIOLATION",
                           reason + " | Lần " + to_string(examViolationCount));
        data.updateExamSession(activeSessionId, "IN_PROGRESS", examViolationCount, reason);

        if (examViolationCount >= violationLimit) {
            pendingExamAutoSubmit = true;
        }

        pendingExamViolationNotice = true;
        if (examAppActive) {
            showPendingExamViolationNotice();
        }
    }

    void showPendingExamViolationNotice() {
        if (currentScreen != SCREEN_TAKE_EXAM_FORM || antiCheatDialogOpen ||
            examSubmissionInProgress) {
            return;
        }

        if (!pendingExamViolationNotice) {
            return;
        }

        pendingExamViolationNotice = false;
        bool autoSubmitAfterNotice = pendingExamAutoSubmit;
        Exam* exam = data.findExamById(activeExamId);
        int violationLimit = exam == nullptr ? 3 : max(1, exam->getViolationLimit());
        string warning = "Bạn vừa rời khỏi màn hình thi hoặc thực hiện thao tác bị hạn chế.\n\n"
                         "Số lần vi phạm: " + to_string(examViolationCount) + "/" +
                         to_string(violationLimit);
        if (autoSubmitAfterNotice) {
            warning += "\nĐã đạt giới hạn. Bài sẽ tự động nộp sau khi bạn đóng cảnh báo.";
        } else {
            warning += "\nBài sẽ tự động nộp khi đạt giới hạn.";
        }
        antiCheatDialogOpen = true;
        MessageBoxW(window, utf8ToWide(warning).c_str(),
                    L"Cảnh báo chống gian lận",
                    MB_OK | MB_ICONWARNING | MB_SETFOREGROUND);
        antiCheatDialogOpen = false;
        examAppActive = true;

        if (autoSubmitAfterNotice && currentScreen == SCREEN_TAKE_EXAM_FORM &&
            !examSubmissionInProgress) {
            pendingExamAutoSubmit = false;
            submitTakeExam(false, true);
        }
    }

    void interruptActiveExam(string reason) {
        if (activeSessionId.empty() || currentUser == nullptr || activeExamId.empty()) {
            return;
        }
        data.updateExamSession(activeSessionId, "INTERRUPTED", examViolationCount, reason);
        data.writeAuditLog(currentUser->getUserId(), activeExamId, "EXAM_INTERRUPTED",
                           reason);
        stopExamTimer();
    }

    void submitTakeExam(bool autoByTimeout = false, bool autoByViolation = false) {
        if (activeExamId.empty()) {
            string examId = getText(5201);
            string examPassword = getText(5202);
            Exam* exam = data.findExamById(examId);
            if (exam == nullptr) {
                error("Không tìm thấy đề thi.");
                return;
            }
            if (!exam->canTake(QuizData::currentMinuteText())) {
                string now = QuizData::currentMinuteText();
                if (exam->isBeforeStart(now)) {
                    error("Chua den gio lam bai. Đề thi mo luc: " + exam->getStartAt());
                } else if (exam->isClosedByTime(now)) {
                    error("Đề thi da het han luc: " + exam->getCloseAt());
                } else {
                    error("Đề thi đang đóng.");
                }
                return;
            }
            if (!exam->checkExamPassword(examPassword)) {
                error("Mật khẩu đề thi không đúng.");
                return;
            }
            int usedAttempts = data.countAttempts(currentUser->getUserId(), exam->getExamId());
            if (usedAttempts >= exam->getAttemptLimit()) {
                error("Bạn đã hết lượt làm bài cho đề này. Số lượt tối đa: " + to_string(exam->getAttemptLimit()) + ".");
                return;
            }
            if (!exam->isFileAnswerExam()) {
                vector<Question*> resolvedQuestions;
                string questionValidationError;
                if (!data.resolveExamQuestions(*exam, resolvedQuestions,
                                               questionValidationError)) {
                    error("Đề thi đang bị thiếu hoặc trùng câu hỏi nên chưa thể bắt đầu.\r\n" +
                          questionValidationError +
                          "\r\nVui lòng báo giáo viên sửa đề.");
                    return;
                }
            }
            string sessionId;
            string sessionError;
            if (!data.startExamSession(currentUser, *exam, sessionId, sessionError)) {
                error(sessionError);
                return;
            }
            showTakeExamForm(*exam);
            activeSessionId = sessionId;
            examViolationCount = 0;
            return;
        }

        Exam* exam = data.findExamById(activeExamId);
        if (exam == nullptr) {
            error("Không tìm thấy đề thi.");
            return;
        }

        map<string, char> answers;
        if (!collectSelectedAnswers(answers, false)) {
            return;
        }

        int unansweredCount = max(0, exam->totalQuestions() - (int)answers.size());
        if (!autoByTimeout && !autoByViolation && unansweredCount > 0) {
            string confirmation =
                "Bạn còn " + to_string(unansweredCount) + " câu chưa trả lời.\n"
                "Các câu bỏ trống sẽ được tính là sai.\n\n"
                "Bạn có chắc chắn muốn nộp bài?";
            antiCheatDialogOpen = true;
            int choice = MessageBoxW(window, utf8ToWide(confirmation).c_str(),
                                     L"Xác nhận nộp bài",
                                     MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2 |
                                         MB_SETFOREGROUND);
            antiCheatDialogOpen = false;
            if (choice != IDYES) {
                return;
            }
        }

        examSubmissionInProgress = true;
        stopExamTimer();
        int score = data.calculateScore(*exam, answers);
        if (!data.submitExam(currentUser, *exam, answers)) {
            examSubmissionInProgress = false;
            error("Không thể lưu bài làm. Bài chưa được đánh dấu hoàn thành.");
            return;
        }
        data.updateExamSession(activeSessionId, "COMPLETED", examViolationCount,
                               autoByTimeout ? "Tự nộp khi hết giờ"
                                             : (autoByViolation ? "Tự nộp do vi phạm" : "Nộp bài"));
        data.writeAuditLog(currentUser->getUserId(), exam->getExamId(), "EXAM_SUBMITTED",
                           "Điểm " + to_string(score) + "/" + to_string(exam->totalQuestions()));
        if (autoByTimeout) {
            message("Hết giờ làm bài. Hệ thống đã tự động nộp bài.");
        } else if (autoByViolation) {
            message("Bài đã tự động nộp do vi phạm quy định chống gian lận nhiều lần.");
        } else {
            message("Bạn đã nộp bài thành công.\r\nĐiểm của bạn: " +
                    to_string(score) + "/" + to_string(exam->totalQuestions()) +
                    "\r\nChi tiết đáp án sẽ được công bố sau.");
        }
        examSubmissionInProgress = false;
        showStudentDashboard();
    }

    bool collectSelectedAnswers(map<string, char>& answers, bool requireAll) {
        answers = activeExamDraftAnswers;
        if (!answerCombos.empty()) {
            for (int i = 0; i < (int)answerCombos.size() && i < (int)activeQuestionIds.size(); i++) {
                int selectedIndex = (int)SendMessageW(answerCombos[i], CB_GETCURSEL, 0, 0);
                if (selectedIndex < 0 || selectedIndex > 3) {
                    if (requireAll) {
                        error("Vui lòng chọn đáp án cho tất cả câu hỏi.");
                        return false;
                    }
                    continue;
                }

                answers[activeQuestionIds[i]] =
                    i < (int)activeAnswerValues.size()
                        ? activeAnswerValues[i][selectedIndex]
                        : (char)('A' + selectedIndex);
            }
            return true;
        }

        for (int i = 0; i < (int)answerOptions.size() && i < (int)activeQuestionIds.size(); i++) {
            char selectedAnswer = '\0';
            for (int optionIndex = 0; optionIndex < 4; optionIndex++) {
                HWND optionButton = answerOptions[i][optionIndex];
                auto selectedIterator = answerChoiceButtons.find(optionButton);
                bool selectedByApp = selectedIterator != answerChoiceButtons.end() && selectedIterator->second;
                bool selectedByWindows = SendMessageW(optionButton, BM_GETCHECK, 0, 0) == BST_CHECKED;
                if (selectedByApp || selectedByWindows) {
                    selectedAnswer =
                        i < (int)activeAnswerValues.size()
                            ? activeAnswerValues[i][optionIndex]
                            : (char)('A' + optionIndex);
                    break;
                }
            }

            if (selectedAnswer == '\0') {
                if (requireAll) {
                    error("Vui lòng chọn đáp án cho tất cả câu hỏi.");
                    return false;
                }
                continue;
            }

            answers[activeQuestionIds[i]] = selectedAnswer;
        }

        return true;
    }

    void updateExamTimer() {
        if (currentScreen != SCREEN_TAKE_EXAM_FORM || activeExamId.empty()) {
            stopExamTimer();
            return;
        }

        auto now = chrono::system_clock::now();
        long long remainingSeconds = chrono::duration_cast<chrono::seconds>(activeExamEndTime - now).count();
        if (remainingSeconds <= 0) {
            if (examTimerLabel != nullptr) {
                setControlText(examTimerLabel, "Còn lại: 00:00");
            }
            stopExamTimer();
            submitTakeExam(true);
            return;
        }

        if (examTimerLabel != nullptr) {
            setControlText(examTimerLabel, "Còn lại: " + formatDuration(remainingSeconds));
        }
    }

    void stopExamTimer() {
        if (examTimerActive && window != nullptr) {
            KillTimer(window, EXAM_TIMER_ID);
        }
        examTimerActive = false;
    }

    string formatDuration(long long totalSeconds) {
        long long minutes = totalSeconds / 60;
        long long seconds = totalSeconds % 60;
        stringstream ss;
        ss << setw(2) << setfill('0') << minutes
           << ":" << setw(2) << setfill('0') << seconds
           << setfill(' ');
        return ss.str();
    }
