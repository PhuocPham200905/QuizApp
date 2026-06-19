    int run() {
        SetProcessDPIAware();
        HRESULT comResult = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
        comInitialized = SUCCEEDED(comResult);

        GdiplusStartupInput gdiplusStartupInput;
        GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);

        backgroundBrush = CreateSolidBrush(THEME_BACKGROUND);
        surfaceBrush = CreateSolidBrush(THEME_SURFACE);
        sidebarBrush = CreateSolidBrush(THEME_SIDEBAR);
        inputBrush = CreateSolidBrush(THEME_SURFACE);
        loginBackgroundBrush = CreateSolidBrush(RGB(243, 244, 246));

        WNDCLASSW wc = {};
        wc.lpfnWndProc = GuiApp::windowProc;
        wc.hInstance = instance;
        wc.lpszClassName = L"QuizAppGuiWindow";
        wc.hbrBackground = backgroundBrush;
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

        RegisterClassW(&wc);

        WNDCLASSW previewWc = {};
        previewWc.lpfnWndProc = GuiApp::previewHostProc;
        previewWc.hInstance = instance;
        previewWc.lpszClassName = L"QuizPreviewHostWindow";
        previewWc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        previewWc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        RegisterClassW(&previewWc);

        WNDCLASSW cardWc = {};
        cardWc.lpfnWndProc = GuiApp::cardPanelProc;
        cardWc.hInstance = instance;
        cardWc.lpszClassName = L"QuizCardPanel";
        cardWc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        RegisterClassW(&cardWc);

        font = CreateFontW(19, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                           DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                           CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
        titleFont = CreateFontW(30, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
                                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
        brandFont = CreateFontW(22, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
                                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
        smallFont = CreateFontW(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");

        INITCOMMONCONTROLSEX controlsInfo = {};
        controlsInfo.dwSize = sizeof(controlsInfo);
        controlsInfo.dwICC = ICC_LISTVIEW_CLASSES;
        InitCommonControlsEx(&controlsInfo);

        RECT workArea = {};
        SystemParametersInfoW(SPI_GETWORKAREA, 0, &workArea, 0);
        int availableWidth = workArea.right - workArea.left;
        int availableHeight = workArea.bottom - workArea.top;
        int windowWidth = min(1280, availableWidth);
        int windowHeight = min(800, availableHeight);
        int windowX = workArea.left + max(0, (availableWidth - windowWidth) / 2);
        int windowY = workArea.top + max(0, (availableHeight - windowHeight) / 2);
        window = CreateWindowW(L"QuizAppGuiWindow", L"HCMUTE Quiz App",
                               WS_OVERLAPPEDWINDOW,
                               windowX, windowY, windowWidth, windowHeight,
                               nullptr, nullptr, instance, this);
        DragAcceptFiles(window, TRUE);

        ShowWindow(window, SW_SHOW);
        UpdateWindow(window);
        showLoginScreen();

        MSG msg = {};
        while (GetMessage(&msg, nullptr, 0, 0)) {
            if (handleShortcut(msg)) {
                continue;
            }

            if (translatePreviewMessage(msg)) {
                continue;
            }

            if (!IsDialogMessageW(window, &msg)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }

        if (gdiplusToken != 0) {
            GdiplusShutdown(gdiplusToken);
            gdiplusToken = 0;
        }
        if (comInitialized) {
            CoUninitialize();
            comInitialized = false;
        }

        return 0;
    }

    LRESULT handleMessage(UINT message, WPARAM wParam, LPARAM lParam) {
        if (message == WM_ERASEBKGND) {
            paintAppBackground((HDC)wParam);
            return 1;
        }

        if (message == WM_CTLCOLORSTATIC) {
            HDC hdc = (HDC)wParam;
            HWND control = (HWND)lParam;
            wchar_t className[32] = L"";
            GetClassNameW(control, className, 32);
            if (wcscmp(className, L"Edit") == 0) {
                SetTextColor(hdc, THEME_TEXT);
                SetBkMode(hdc, OPAQUE);
                SetBkColor(hdc, THEME_SURFACE);
                return (LRESULT)inputBrush;
            }
            auto color = controlTextColors.find(control);
            SetTextColor(hdc, color != controlTextColors.end() ? color->second : THEME_TEXT);
            SetBkMode(hdc, TRANSPARENT);
            if (surfaceLabels.find(control) != surfaceLabels.end()) {
                return (LRESULT)surfaceBrush;
            }
            if (sidebarLabels.find(control) != sidebarLabels.end()) {
                return (LRESULT)sidebarBrush;
            }
            if (currentScreen == SCREEN_LOGIN) {
                return (LRESULT)loginBackgroundBrush;
            }
            return (LRESULT)backgroundBrush;
        }

        if (message == WM_CTLCOLOREDIT || message == WM_CTLCOLORLISTBOX) {
            HDC hdc = (HDC)wParam;
            SetTextColor(hdc, THEME_TEXT);
            SetBkColor(hdc, THEME_SURFACE);
            return (LRESULT)inputBrush;
        }

        if (message == WM_DRAWITEM) {
            DRAWITEMSTRUCT* drawInfo = (DRAWITEMSTRUCT*)lParam;
            if (drawInfo != nullptr && drawInfo->CtlType == ODT_BUTTON) {
                if (answerChoiceButtons.find(drawInfo->hwndItem) != answerChoiceButtons.end()) {
                    drawAnswerChoiceButton(drawInfo);
                } else {
                    drawModernButton(drawInfo);
                }
                return TRUE;
            }
        }

        if (message == WM_COMMAND) {
            handleCommand(LOWORD(wParam), HIWORD(wParam));
            return 0;
        }

        if (message == WM_ACTIVATEAPP && wParam == FALSE &&
            currentScreen == SCREEN_TAKE_EXAM_FORM && !antiCheatDialogOpen &&
            !examSubmissionInProgress) {
            if (chrono::steady_clock::now() <= allowExamViewerFocusLossUntil) {
                allowExamViewerFocusLossUntil = chrono::steady_clock::time_point{};
                return 0;
            }
            recordExamViolation("Mất focus khỏi cửa sổ thi");
            return 0;
        }

        if (message == WM_SIZE && wParam == SIZE_MINIMIZED &&
            currentScreen == SCREEN_TAKE_EXAM_FORM && !examSubmissionInProgress) {
            recordExamViolation("Thu nhỏ cửa sổ thi");
            return 0;
        }

        if (message == WM_TIMER && wParam == EXAM_TIMER_ID) {
            updateExamTimer();
            return 0;
        }

        if (message == WM_DROPFILES) {
            loadDroppedCsv((HDROP)wParam);
            return 0;
        }

        if (message == WM_MOUSEWHEEL) {
            HWND previewHost = previewHostFromPoint(lParam);
            if (previewHost != nullptr) {
                focusPreviewHandler(previewHost);
                POINT point = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
                HWND target = WindowFromPoint(point);
                if (target != nullptr && target != window && target != previewHost) {
                    SendMessageW(target, message, wParam, lParam);
                    return 0;
                }
            }
        }

        if (message == WM_CLOSE) {
            if (currentScreen == SCREEN_TAKE_EXAM_FORM && !activeSessionId.empty() &&
                !examSubmissionInProgress) {
                interruptActiveExam("Thoát ứng dụng giữa chừng");
            }
            DestroyWindow(window);
            return 0;
        }

        if (message == WM_DESTROY) {
            stopExamTimer();
            releaseThemeObjects();
            PostQuitMessage(0);
            return 0;
        }

        return DefWindowProcW(window, message, wParam, lParam);
    }

private:
    static LRESULT CALLBACK windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
        GuiApp* app = nullptr;

        if (message == WM_NCCREATE) {
            CREATESTRUCTA* create = (CREATESTRUCTA*)lParam;
            app = (GuiApp*)create->lpCreateParams;
            SetWindowLongPtrA(hwnd, GWLP_USERDATA, (LONG_PTR)app);
            app->window = hwnd;
        } else {
            app = (GuiApp*)GetWindowLongPtrA(hwnd, GWLP_USERDATA);
        }

        if (app != nullptr) {
            return app->handleMessage(message, wParam, lParam);
        }

        return DefWindowProcW(hwnd, message, wParam, lParam);
    }

    static LRESULT CALLBACK previewHostProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
        GuiApp* app = nullptr;

        if (message == WM_NCCREATE) {
            CREATESTRUCTW* create = (CREATESTRUCTW*)lParam;
            app = (GuiApp*)create->lpCreateParams;
            SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)app);
        } else {
            app = (GuiApp*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
        }

        if (app != nullptr) {
            LRESULT result = 0;
            if (app->handlePreviewHostMessage(hwnd, message, wParam, lParam, result)) {
                return result;
            }
        }

        return DefWindowProcW(hwnd, message, wParam, lParam);
    }

    static LRESULT CALLBACK cardPanelProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
        if (message == WM_ERASEBKGND) {
            return 1;
        }

        if (message == WM_PAINT) {
            PAINTSTRUCT ps = {};
            HDC hdc = BeginPaint(hwnd, &ps);
            RECT rect = {};
            GetClientRect(hwnd, &rect);

            HBRUSH fillBrush = CreateSolidBrush(THEME_SURFACE);
            HPEN borderPen = CreatePen(PS_SOLID, 1, THEME_BORDER);
            HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, fillBrush);
            HPEN oldPen = (HPEN)SelectObject(hdc, borderPen);

            RoundRect(hdc, rect.left, rect.top, rect.right - 1, rect.bottom - 1, 18, 18);

            SelectObject(hdc, oldPen);
            SelectObject(hdc, oldBrush);
            DeleteObject(borderPen);
            DeleteObject(fillBrush);
            EndPaint(hwnd, &ps);
            return 0;
        }

        return DefWindowProcW(hwnd, message, wParam, lParam);
    }

    bool handlePreviewHostMessage(HWND host, UINT message, WPARAM wParam, LPARAM lParam, LRESULT& result) {
        if (message == WM_MOUSEACTIVATE) {
            focusPreviewHandler(host);
            result = MA_ACTIVATE;
            return true;
        }

        if (message == WM_SETFOCUS) {
            focusPreviewHandler(host);
            result = 0;
            return true;
        }

        if (message == WM_MOUSEWHEEL ||
            message == WM_LBUTTONDOWN || message == WM_LBUTTONUP ||
            message == WM_LBUTTONDBLCLK || message == WM_MOUSEMOVE ||
            message == WM_RBUTTONDOWN || message == WM_RBUTTONUP) {
            focusPreviewHandler(host);
            if (forwardPreviewMouseMessage(host, message, wParam, lParam)) {
                result = 0;
                return true;
            }
        }

        return false;
    }

    void paintAppBackground(HDC hdc) {
        RECT client = {};
        GetClientRect(window, &client);

        if (currentScreen == SCREEN_LOGIN) {
            FillRect(hdc, &client, loginBackgroundBrush);

            int centerX = (int)(client.right - client.left) / 2;
            RECT card = { centerX - 270, 145, centerX + 270, min(client.bottom - 12, 548L) };
            HBRUSH cardBrush = CreateSolidBrush(THEME_SURFACE);
            HPEN cardPen = CreatePen(PS_SOLID, 1, THEME_BORDER);
            HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, cardBrush);
            HPEN oldPen = (HPEN)SelectObject(hdc, cardPen);
            RoundRect(hdc, card.left, card.top, card.right, card.bottom, 12, 12);
            SelectObject(hdc, oldPen);
            SelectObject(hdc, oldBrush);
            DeleteObject(cardPen);
            DeleteObject(cardBrush);
            return;
        }

        FillRect(hdc, &client, backgroundBrush);

        RECT sidebar = { 0, 0, 240, client.bottom };
        FillRect(hdc, &sidebar, sidebarBrush);

        HPEN borderPen = CreatePen(PS_SOLID, 1, THEME_BORDER);
        HPEN oldPen = (HPEN)SelectObject(hdc, borderPen);
        MoveToEx(hdc, 240, 0, nullptr);
        LineTo(hdc, 240, client.bottom);
        SelectObject(hdc, oldPen);
        DeleteObject(borderPen);
    }

    void drawModernButton(DRAWITEMSTRUCT* drawInfo) {
        HWND button = drawInfo->hwndItem;
        bool primary = primaryButtons.find(button) != primaryButtons.end() && primaryButtons[button];
        bool pressed = (drawInfo->itemState & ODS_SELECTED) != 0;
        bool focused = (drawInfo->itemState & ODS_FOCUS) != 0;
        bool disabled = (drawInfo->itemState & ODS_DISABLED) != 0;

        RECT rect = drawInfo->rcItem;
        HDC hdc = drawInfo->hDC;

        COLORREF fillColor = primary ? THEME_PRIMARY : THEME_SURFACE;
        COLORREF borderColor = primary ? THEME_PRIMARY : THEME_BORDER;
        COLORREF textColor = primary ? RGB(255, 255, 255) : THEME_TEXT;

        if (pressed && primary) {
            fillColor = THEME_PRIMARY_HOVER;
            borderColor = THEME_PRIMARY_HOVER;
        } else if (pressed) {
            fillColor = RGB(226, 232, 240);
        }
        if (disabled) {
            fillColor = RGB(226, 232, 240);
            borderColor = RGB(203, 213, 225);
            textColor = RGB(100, 116, 139);
        }

        HBRUSH fillBrush = CreateSolidBrush(fillColor);
        HPEN borderPen = CreatePen(PS_SOLID, focused ? 2 : 1, borderColor);
        HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, fillBrush);
        HPEN oldPen = (HPEN)SelectObject(hdc, borderPen);

        int offset = pressed ? 1 : 0;
        RECT buttonRect = rect;
        InflateRect(&buttonRect, -1, -1);
        RoundRect(hdc, buttonRect.left, buttonRect.top, buttonRect.right, buttonRect.bottom, 8, 8);

        SelectObject(hdc, oldBrush);
        SelectObject(hdc, oldPen);
        DeleteObject(fillBrush);
        DeleteObject(borderPen);

        wchar_t text[256] = L"";
        GetWindowTextW(button, text, 256);
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, textColor);
        HFONT oldFont = (HFONT)SelectObject(hdc, font);
        RECT textRect = rect;
        OffsetRect(&textRect, offset, offset);
        DrawTextW(hdc, text, -1, &textRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
        SelectObject(hdc, oldFont);
    }

    void drawAnswerChoiceButton(DRAWITEMSTRUCT* drawInfo) {
        HWND button = drawInfo->hwndItem;
        auto selectedIterator = answerChoiceButtons.find(button);
        bool checked = selectedIterator != answerChoiceButtons.end() && selectedIterator->second;
        bool pressed = (drawInfo->itemState & ODS_SELECTED) != 0;
        bool focused = (drawInfo->itemState & ODS_FOCUS) != 0;

        RECT rect = drawInfo->rcItem;
        HDC hdc = drawInfo->hDC;

        wchar_t text[512] = L"";
        GetWindowTextW(button, text, 512);
        bool compactBubble = wcslen(text) <= 2;

        COLORREF fillColor = checked ? THEME_PRIMARY : THEME_SURFACE;
        COLORREF borderColor = checked ? THEME_PRIMARY : RGB(148, 163, 184);
        COLORREF textColor = checked ? RGB(255, 255, 255) : THEME_TEXT;

        if (checked && pressed) {
            fillColor = THEME_PRIMARY_HOVER;
            borderColor = THEME_PRIMARY_HOVER;
        } else if (!checked && pressed) {
            fillColor = RGB(219, 234, 254);
            borderColor = THEME_PRIMARY;
        } else if (!checked && focused) {
            borderColor = THEME_PRIMARY;
        }

        HBRUSH fillBrush = CreateSolidBrush(fillColor);
        HPEN borderPen = CreatePen(PS_SOLID, checked || focused ? 2 : 1, borderColor);
        HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, fillBrush);
        HPEN oldPen = (HPEN)SelectObject(hdc, borderPen);

        RECT buttonRect = rect;
        InflateRect(&buttonRect, -1, -1);
        if (compactBubble) {
            int diameter = min(buttonRect.right - buttonRect.left - 4, buttonRect.bottom - buttonRect.top - 4);
            int centerX = (buttonRect.left + buttonRect.right) / 2;
            int centerY = (buttonRect.top + buttonRect.bottom) / 2;
            RECT circle = {
                centerX - diameter / 2,
                centerY - diameter / 2,
                centerX + diameter / 2,
                centerY + diameter / 2
            };
            Ellipse(hdc, circle.left, circle.top, circle.right, circle.bottom);
        } else {
            RoundRect(hdc, buttonRect.left, buttonRect.top, buttonRect.right, buttonRect.bottom, 9, 9);
        }

        SelectObject(hdc, oldBrush);
        SelectObject(hdc, oldPen);
        DeleteObject(fillBrush);
        DeleteObject(borderPen);

        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, textColor);

        HFONT oldFont = (HFONT)SelectObject(hdc, smallFont);
        RECT textRect = rect;
        if (compactBubble) {
            DrawTextW(hdc, text, -1, &textRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        } else {
            textRect.left += 14;
            textRect.right -= 10;
            DrawTextW(hdc, text, -1, &textRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
        }
        SelectObject(hdc, oldFont);
    }

    void releaseThemeObjects() {
        if (font != nullptr) {
            DeleteObject(font);
            font = nullptr;
        }
        if (titleFont != nullptr) {
            DeleteObject(titleFont);
            titleFont = nullptr;
        }
        if (brandFont != nullptr) {
            DeleteObject(brandFont);
            brandFont = nullptr;
        }
        if (smallFont != nullptr) {
            DeleteObject(smallFont);
            smallFont = nullptr;
        }
        if (backgroundBrush != nullptr) {
            DeleteObject(backgroundBrush);
            backgroundBrush = nullptr;
        }
        if (surfaceBrush != nullptr) {
            DeleteObject(surfaceBrush);
            surfaceBrush = nullptr;
        }
        if (sidebarBrush != nullptr) {
            DeleteObject(sidebarBrush);
            sidebarBrush = nullptr;
        }
        if (inputBrush != nullptr) {
            DeleteObject(inputBrush);
            inputBrush = nullptr;
        }
        if (loginBackgroundBrush != nullptr) {
            DeleteObject(loginBackgroundBrush);
            loginBackgroundBrush = nullptr;
        }
    }

    void handleCommand(int id, int notifyCode) {
        if (notifyCode == BN_CLICKED && id >= 5300 && id < 6200 && selectAnswerChoice(id)) {
            return;
        }

        if (id == ID_MAIN_REGISTER) showRegisterScreen();
        else if (id == ID_MAIN_LOGIN) showLoginScreen();
        else if (id == ID_BACK_MAIN) showMainScreen();
        else if (id == ID_LOGIN_SUBMIT) submitLogin();
        else if (id == ID_REGISTER_SUBMIT) submitRegisterStudent();
        else if (id == ID_PROFILE) showProfileScreen();
        else if (id == ID_PROFILE_SAVE) submitProfileUpdate();
        else if (id == ID_HELP) showUserGuide();
        else if (id == ID_LOGOUT) { currentUser = nullptr; showMainScreen(); }
        else if (id == ID_ADMIN_STUDENTS) showStudents();
        else if (id == ID_ADMIN_TEACHERS) showTeachers();
        else if (id == ID_ADMIN_CREATE_TEACHER) showCreateTeacher();
        else if (id == ID_ADMIN_QUESTIONS) showQuestions(true);
        else if (id == ID_ADMIN_EXAMS) showExams();
        else if (id == ID_ADMIN_TOGGLE_EXAM) showToggleExam();
        else if (id == ID_ADMIN_RESULTS) showAllResults();
        else if (id == ID_ADMIN_AUDIT) showAntiCheatMonitor();
        else if (id == ID_TEACHER_QUESTIONS) showQuestions(true);
        else if (id == ID_TEACHER_ADD_QUESTION) showAddQuestion();
        else if (id == ID_TEACHER_AI_QUESTIONS) showAiQuestionGenerator();
        else if (id == ID_TEACHER_IMPORT_QUESTIONS) showImportQuestions();
        else if (id == ID_TEACHER_DELETE_QUESTION) showDeleteQuestion();
        else if (id == ID_TEACHER_EXAMS) showExams();
        else if (id == ID_TEACHER_CREATE_EXAM) showCreateExam();
        else if (id == ID_TEACHER_TOGGLE_EXAM) showToggleExam();
        else if (id == ID_TEACHER_MANAGE_EXAMS) showManageExam();
        else if (id == ID_TEACHER_RESULTS) showAllResults();
        else if (id == ID_STUDENT_OPEN_EXAMS) showOpenExams();
        else if (id == ID_STUDENT_TAKE_EXAM) showTakeExamSelect();
        else if (id == ID_STUDENT_RESULTS) showMyResults();
        else if (id == ID_CREATE_TEACHER_SUBMIT) submitCreateTeacher();
        else if (id == ID_ADD_QUESTION_CHOOSE_IMAGE) chooseQuestionImage();
        else if (id == ID_ADD_QUESTION_SUBMIT) submitAddQuestion();
        else if (id == ID_IMPORT_QUESTIONS_SUBMIT) submitImportQuestions();
        else if (id == ID_IMPORT_QUESTIONS_FROM_URL) loadImportQuestionsFromUrl();
        else if (id == ID_IMPORT_QUESTIONS_FROM_FILE) loadImportQuestionsFromFile();
        else if (id == ID_IMPORT_QUESTIONS_TEMPLATE) showCsvTemplate();
        else if (id == ID_AI_GENERATE_QUESTIONS) submitGenerateAiQuestions();
        else if (id == ID_AI_REFRESH_PREVIEW) refreshAiPreviewFromJson();
        else if (id == ID_AI_SAVE_ONE) saveOneAiQuestion();
        else if (id == ID_AI_SAVE_ALL) saveAllAiQuestions();
        else if (id == ID_DELETE_QUESTION_SUBMIT) submitDeleteQuestion();
        else if (id == ID_CREATE_EXAM_CHOOSE_FILE) chooseExamFile();
        else if (id == ID_CREATE_EXAM_SUBMIT) submitCreateExam();
        else if (id == ID_TOGGLE_EXAM_SUBMIT) submitToggleExam();
        else if (id == ID_UPDATE_EXAM_ATTEMPT_LIMIT) submitUpdateExamAttemptLimit();
        else if (id == ID_LOAD_EXAM_EDIT) loadExamForEdit();
        else if (id == ID_UPDATE_EXAM_SUBMIT) submitUpdateExam();
        else if (id == ID_DELETE_EXAM_SUBMIT) submitDeleteExam();
        else if (id == ID_QUESTION_FILTER) showQuestions(true, getText(6401), getText(6402), getText(6403));
        else if (id == ID_RESULTS_EXPORT) exportResultsCsv();
        else if (id == ID_RESULTS_FILTER) showAllResults(getText(6501), getText(6502));
        else if (id == ID_UNLOCK_SESSION) unlockExamSession();
        else if (id == ID_TAKE_EXAM_OPEN_FILE) openActiveExamAttachment();
        else if (id == ID_TAKE_EXAM_SUBMIT) submitTakeExam();
        else if (id == ID_STUDENT_DELETE) submitDeleteManagedUser("student");
        else if (id == ID_STUDENT_CHANGE_CLASS) submitChangeManagedUserClass("student");
        else if (id == ID_STUDENT_SEARCH) showStudents(getText(6103));
        else if (id == ID_STUDENT_RESET_PASSWORD) submitResetManagedUserPassword("student");
        else if (id == ID_TEACHER_DELETE) submitDeleteManagedUser("teacher");
        else if (id == ID_TEACHER_CHANGE_CLASS) submitChangeManagedUserClass("teacher");
        else if (id == ID_TEACHER_SEARCH) showTeachers(getText(6203));
        else if (id == ID_TEACHER_RESET_PASSWORD) submitResetManagedUserPassword("teacher");
        else if (id == ID_DASHBOARD) showCurrentDashboard();
        else if (id == IDCANCEL) DestroyWindow(window);
    }

    void refreshAnswerChoiceButtons() {
        for (array<HWND, 4>& row : answerOptions) {
            for (HWND button : row) {
                if (button != nullptr &&
                    answerChoiceButtons.find(button) != answerChoiceButtons.end()) {
                    InvalidateRect(button, nullptr, TRUE);
                    UpdateWindow(button);
                }
            }
        }
    }

    bool selectAnswerChoice(int id) {
        HWND selectedButton = GetDlgItem(window, id);
        if (selectedButton == nullptr) {
            return false;
        }

        for (array<HWND, 4>& row : answerOptions) {
            bool foundInRow = false;
            for (HWND button : row) {
                if (button == selectedButton) {
                    foundInRow = true;
                    break;
                }
            }

            if (!foundInRow) {
                continue;
            }

            for (HWND button : row) {
                bool selected = button == selectedButton;
                answerChoiceButtons[button] = selected;
                SendMessageW(button, BM_SETCHECK, selected ? BST_CHECKED : BST_UNCHECKED, 0);
            }
            refreshAnswerChoiceButtons();
            SetFocus(selectedButton);
            return true;
        }

        return false;
    }

    wstring utf8ToWide(string text) {
        if (text.empty()) {
            return L"";
        }

        int size = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, nullptr, 0);
        if (size <= 0) {
            size = MultiByteToWideChar(CP_ACP, 0, text.c_str(), -1, nullptr, 0);
            if (size <= 0) {
                return L"";
            }
            wstring wide(size, L'\0');
            MultiByteToWideChar(CP_ACP, 0, text.c_str(), -1, wide.data(), size);
            wide.resize(size - 1);
            return wide;
        }

        wstring wide(size, L'\0');
        MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, wide.data(), size);
        wide.resize(size - 1);
        return wide;
    }

    string wideToUtf8(wstring text) {
        if (text.empty()) {
            return "";
        }

        int size = WideCharToMultiByte(CP_UTF8, 0, text.c_str(), -1, nullptr, 0, nullptr, nullptr);
        if (size <= 0) {
            return "";
        }

        string utf8(size, '\0');
        WideCharToMultiByte(CP_UTF8, 0, text.c_str(), -1, utf8.data(), size, nullptr, nullptr);
        utf8.resize(size - 1);
        return utf8;
    }

    void setControlText(HWND control, string text) {
        wstring wide = utf8ToWide(text);
        SetWindowTextW(control, wide.c_str());
    }

    bool handleShortcut(MSG& msg) {
        if (msg.message != WM_KEYDOWN) {
            return false;
        }

        bool ctrlDown = (GetKeyState(VK_CONTROL) & 0x8000) != 0;

        if (currentScreen == SCREEN_TAKE_EXAM_FORM && ctrlDown &&
            (msg.wParam == 'C' || msg.wParam == 'V' || msg.wParam == 'X' ||
             msg.wParam == VK_INSERT)) {
            recordExamViolation(msg.wParam == 'V'
                                    ? "Phát hiện thao tác dán"
                                    : "Phát hiện thao tác sao chép/cắt");
            return true;
        }

        if (msg.wParam == VK_F1) {
            showShortcutHelp();
            return true;
        }

        if (ctrlDown && msg.wParam == 'Q') {
            DestroyWindow(window);
            return true;
        }

        if (ctrlDown && msg.wParam == 'L') {
            currentUser = nullptr;
            showMainScreen();
            return true;
        }

        if (msg.wParam == VK_ESCAPE) {
            goBackByShortcut();
            return true;
        }

        if (msg.wParam == VK_DELETE && currentScreen == SCREEN_DELETE_QUESTION) {
            submitDeleteQuestion();
            return true;
        }

        if (msg.wParam == VK_RETURN) {
            return submitDefaultByShortcut();
        }

        return false;
    }

    void clearControls() {
        stopExamTimer();
        answerOptions.clear();
        activeAnswerValues.clear();
        answerCombos.clear();
        activeQuestionIds.clear();
        activeExamId.clear();
        activeSessionId.clear();
        examViolationCount = 0;
        lastExamViolationAt = chrono::steady_clock::time_point{};
        allowExamViewerFocusLossUntil = chrono::steady_clock::time_point{};
        examTimerLabel = nullptr;
        primaryButtons.clear();
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

    HWND addControl(string cls, string text, DWORD style, int x, int y, int w, int h, int id = 0, bool tabStop = false) {
        DWORD baseStyle = WS_CHILD | WS_VISIBLE;
        if (tabStop) {
            baseStyle |= WS_TABSTOP;
        }

        wstring wideClass = utf8ToWide(cls);
        wstring wideText = utf8ToWide(text);
        DWORD exStyle = 0;
        if (cls == "EDIT") {
            exStyle = WS_EX_CLIENTEDGE;
        }

        HWND control = CreateWindowExW(exStyle, wideClass.c_str(), wideText.c_str(),
                                      baseStyle | style,
                                      x, y, w, h, window, (HMENU)(INT_PTR)id, instance, nullptr);
        SendMessageW(control, WM_SETFONT, (WPARAM)font, TRUE);
        controls.push_back(control);
        return control;
    }

    HWND label(string text, int x, int y, int w, int h) {
        return addControl("STATIC", text, 0, x, y, w, h, 0, false);
    }

    HWND centeredLabel(string text, int x, int y, int w, int h, HFONT labelFont = nullptr,
                       COLORREF textColor = THEME_TEXT) {
        HWND control = addControl("STATIC", text, SS_CENTER | SS_CENTERIMAGE, x, y, w, h, 0, false);
        if (labelFont != nullptr) {
            SendMessageW(control, WM_SETFONT, (WPARAM)labelFont, TRUE);
        }
        controlTextColors[control] = textColor;
        return control;
    }

    HWND cardPanel(int x, int y, int w, int h) {
        HWND control = CreateWindowExW(0, L"QuizCardPanel", L"",
                                      WS_CHILD | WS_VISIBLE,
                                      x, y, w, h, window, nullptr, instance, nullptr);
        SetWindowPos(control, HWND_BOTTOM, x, y, w, h, SWP_NOACTIVATE);
        controls.push_back(control);
        return control;
    }

    HWND button(string text, int x, int y, int w, int h, int id) {
        HWND control = addControl("BUTTON", text, BS_OWNERDRAW, x, y, w, h, id, true);
        primaryButtons[control] = false;
        return control;
    }

    HWND defaultButton(string text, int x, int y, int w, int h, int id) {
        HWND control = addControl("BUTTON", text, BS_OWNERDRAW, x, y, w, h, id, true);
        primaryButtons[control] = true;
        return control;
    }

    HWND radioButton(string text, int x, int y, int w, int h, int id, bool startGroup) {
        DWORD style = BS_AUTORADIOBUTTON;
        if (startGroup) {
            style |= WS_GROUP;
        }
        return addControl("BUTTON", text, style, x, y, w, h, id, true);
    }

    HWND answerChoiceButton(string text, int x, int y, int w, int h, int id, bool startGroup) {
        DWORD style = BS_OWNERDRAW | BS_CENTER | BS_VCENTER;
        if (startGroup) {
            style |= WS_GROUP;
        }

        HWND control = addControl("BUTTON", text, style, x, y, w, h, id, true);
        SendMessageW(control, WM_SETFONT, (WPARAM)smallFont, TRUE);
        answerChoiceButtons[control] = false;
        return control;
    }

    HWND comboBox(vector<string> items, int x, int y, int w, int h, int id) {
        HWND combo = CreateWindowExW(0, L"COMBOBOX", L"",
                                    WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST,
                                    x, y, w, h, window, (HMENU)(INT_PTR)id, instance, nullptr);
        SendMessageW(combo, WM_SETFONT, (WPARAM)font, TRUE);
        for (const string& item : items) {
            wstring wideItem = utf8ToWide(item);
            SendMessageW(combo, CB_ADDSTRING, 0, (LPARAM)wideItem.c_str());
        }
        SendMessageW(combo, CB_SETCURSEL, (WPARAM)-1, 0);
        controls.push_back(combo);
        return combo;
    }

    HWND checkBox(string text, int x, int y, int w, int h, int id, bool checked = false) {
        HWND control = addControl("BUTTON", text, BS_AUTOCHECKBOX | BS_LEFTTEXT,
                                  x, y, w, h, id, true);
        SendMessageW(control, BM_SETCHECK, checked ? BST_CHECKED : BST_UNCHECKED, 0);
        return control;
    }

    bool isChecked(int id) {
        HWND control = GetDlgItem(window, id);
        return control != nullptr &&
               SendMessageW(control, BM_GETCHECK, 0, 0) == BST_CHECKED;
    }

    HWND edit(string text, int x, int y, int w, int h, int id, bool password = false, bool multi = false, bool readOnly = false) {
        DWORD style = ES_AUTOHSCROLL;
        if (password) style |= ES_PASSWORD;
        if (multi) style = ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL;
        if (readOnly) style |= ES_READONLY;
        return addControl("EDIT", text, style, x, y, w, h, id, !readOnly);
    }

    static LRESULT CALLBACK protectedExamEditProc(HWND control, UINT message, WPARAM wParam,
                                                   LPARAM lParam, UINT_PTR subclassId,
                                                   DWORD_PTR referenceData) {
        GuiApp* app = reinterpret_cast<GuiApp*>(referenceData);
        if (message == WM_COPY || message == WM_CUT || message == WM_PASTE ||
            message == WM_CONTEXTMENU) {
            if (app != nullptr) {
                app->recordExamViolation(message == WM_PASTE
                                             ? "Phát hiện thao tác dán"
                                             : "Phát hiện sao chép nội dung đề");
            }
            return 0;
        }
        if (message == WM_KEYDOWN && (GetKeyState(VK_CONTROL) & 0x8000) != 0 &&
            (wParam == 'C' || wParam == 'V' || wParam == 'X' || wParam == VK_INSERT)) {
            if (app != nullptr) {
                app->recordExamViolation("Phát hiện phím tắt sao chép/dán");
            }
            return 0;
        }
        if (message == WM_NCDESTROY) {
            RemoveWindowSubclass(control, protectedExamEditProc, subclassId);
        }
        return DefSubclassProc(control, message, wParam, lParam);
    }

    HWND protectedExamText(string text, int x, int y, int w, int h) {
        HWND control = edit(text, x, y, w, h, 0, false, true, true);
        SetWindowSubclass(control, protectedExamEditProc, 1, (DWORD_PTR)this);
        return control;
    }

    HWND imageBox(string path, int x, int y, int maxW, int maxH) {
        if (path.empty()) {
            return nullptr;
        }

        wstring widePath = utf8ToWide(path);
        Image image(widePath.c_str());
        if (image.GetLastStatus() != Ok || image.GetWidth() == 0 || image.GetHeight() == 0) {
            return nullptr;
        }

        int originalW = (int)image.GetWidth();
        int originalH = (int)image.GetHeight();
        double scale = min((double)maxW / originalW, (double)maxH / originalH);
        if (scale > 1.0) {
            scale = 1.0;
        }

        int targetW = max(1, (int)(originalW * scale));
        int targetH = max(1, (int)(originalH * scale));

        Bitmap canvas(targetW, targetH, PixelFormat32bppARGB);
        Graphics graphics(&canvas);
        graphics.SetInterpolationMode(InterpolationModeHighQualityBicubic);
        graphics.Clear(Color(255, 245, 247, 250));
        graphics.DrawImage(&image, 0, 0, targetW, targetH);

        HBITMAP bitmap = nullptr;
        canvas.GetHBITMAP(Color(255, 245, 247, 250), &bitmap);
        if (bitmap == nullptr) {
            return nullptr;
        }

        HWND control = addControl("STATIC", "", SS_BITMAP | SS_CENTERIMAGE, x, y, targetW, targetH, 0, false);
        SendMessageW(control, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)bitmap);
        imageBitmaps.push_back(bitmap);
        return control;
    }

    HWND shellPreviewBox(string path, int x, int y, int maxW, int maxH) {
        if (path.empty()) {
            return nullptr;
        }

        wstring widePath = utf8ToWide(path);
        IShellItemImageFactory* imageFactory = nullptr;
        HRESULT result = SHCreateItemFromParsingName(
            widePath.c_str(),
            nullptr,
            IID_PPV_ARGS(&imageFactory)
        );
        if (FAILED(result) || imageFactory == nullptr) {
            return nullptr;
        }

        SIZE size = { maxW, maxH };
        HBITMAP bitmap = nullptr;
        result = imageFactory->GetImage(size, SIIGBF_RESIZETOFIT, &bitmap);
        imageFactory->Release();
        if (FAILED(result) || bitmap == nullptr) {
            return nullptr;
        }

        HWND control = addControl("STATIC", "", SS_BITMAP | SS_CENTERIMAGE, x, y, maxW, maxH, 0, false);
        SendMessageW(control, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)bitmap);
        imageBitmaps.push_back(bitmap);
        return control;
    }

    HWND previewHandlerBox(string path, int x, int y, int w, int h) {
        if (path.empty()) {
            return nullptr;
        }

        wstring widePath = utf8ToWide(path);
        CLSID handlerClsid;
        if (!findPreviewHandlerClsid(widePath, handlerClsid)) {
            return nullptr;
        }

        IPreviewHandler* handler = nullptr;
        HRESULT result = CoCreateInstance(
            handlerClsid,
            nullptr,
            CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER,
            IID_PPV_ARGS(&handler)
        );
        if (FAILED(result) || handler == nullptr) {
            return nullptr;
        }

        IInitializeWithFile* initializer = nullptr;
        result = handler->QueryInterface(IID_PPV_ARGS(&initializer));
        if (SUCCEEDED(result) && initializer != nullptr) {
            result = initializer->Initialize(widePath.c_str(), STGM_READ);
            initializer->Release();
        }
        if (FAILED(result)) {
            handler->Release();
            return nullptr;
        }

        HWND host = CreateWindowExW(
            WS_EX_CLIENTEDGE,
            L"QuizPreviewHostWindow",
            L"",
            WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_TABSTOP,
            x, y, w, h,
            window,
            nullptr,
            instance,
            this
        );
        if (host == nullptr) {
            handler->Release();
            return nullptr;
        }
        controls.push_back(host);
        RECT rect = { 0, 0, w, h };
        result = handler->SetWindow(host, &rect);
        if (SUCCEEDED(result)) {
            result = handler->DoPreview();
        }
        if (FAILED(result)) {
            handler->Unload();
            handler->Release();
            controls.erase(remove(controls.begin(), controls.end(), host), controls.end());
            DestroyWindow(host);
            return nullptr;
        }

        handler->SetRect(&rect);
        previewHandlers.push_back(handler);
        previewHandlerByHost[host] = handler;
        return host;
    }

    bool forwardPreviewMouseMessage(HWND host, UINT message, WPARAM wParam, LPARAM lParam) {
        HWND target = nullptr;

        if (message == WM_MOUSEWHEEL) {
            POINT screenPoint = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            target = WindowFromPoint(screenPoint);
        } else {
            POINT hostPoint = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            POINT screenPoint = hostPoint;
            ClientToScreen(host, &screenPoint);
            target = WindowFromPoint(screenPoint);
        }

        if (target == nullptr || target == host || !IsChild(host, target)) {
            target = deepestPreviewChild(host);
        }

        if (target == nullptr || target == host) {
            return false;
        }

        if (message == WM_MOUSEWHEEL) {
            SendMessageW(target, message, wParam, lParam);
            return true;
        }

        POINT targetPoint = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        ClientToScreen(host, &targetPoint);
        ScreenToClient(target, &targetPoint);
        LPARAM targetLParam = MAKELPARAM(targetPoint.x, targetPoint.y);
        SendMessageW(target, message, wParam, targetLParam);
        return true;
    }

    HWND deepestPreviewChild(HWND host) {
        HWND child = GetWindow(host, GW_CHILD);
        if (child == nullptr) {
            return nullptr;
        }

        HWND current = child;
        while (true) {
            HWND nested = GetWindow(current, GW_CHILD);
            if (nested == nullptr) {
                return current;
            }
            current = nested;
        }
    }

    HWND previewHostFromPoint(LPARAM lParam) {
        POINT point = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        for (const auto& item : previewHandlerByHost) {
            HWND host = item.first;
            RECT rect = {};
            if (IsWindow(host) && GetWindowRect(host, &rect) && PtInRect(&rect, point)) {
                return host;
            }
        }

        return nullptr;
    }

    void focusPreviewHandler(HWND host) {
        auto iterator = previewHandlerByHost.find(host);
        if (iterator != previewHandlerByHost.end() && iterator->second != nullptr) {
            iterator->second->SetFocus();
            return;
        }

        HWND previewChild = GetWindow(host, GW_CHILD);
        if (previewChild != nullptr) {
            SetFocus(previewChild);
        }
    }

    bool translatePreviewMessage(MSG& msg) {
        if (previewHandlerByHost.empty()) {
            return false;
        }

        if (msg.message != WM_KEYDOWN && msg.message != WM_KEYUP) {
            return false;
        }

        HWND previewHost = nullptr;
        if (msg.hwnd != nullptr) {
            previewHost = previewHostFromWindow(msg.hwnd);
        }

        if (previewHost == nullptr) {
            return false;
        }

        auto iterator = previewHandlerByHost.find(previewHost);
        if (iterator == previewHandlerByHost.end() || iterator->second == nullptr) {
            return false;
        }

        if (msg.message == WM_KEYDOWN) {
            focusPreviewHandler(previewHost);
        }

        return iterator->second->TranslateAccelerator(&msg) == S_OK;
    }

    HWND previewHostFromWindow(HWND hwnd) {
        HWND current = hwnd;
        while (current != nullptr) {
            if (previewHandlerByHost.find(current) != previewHandlerByHost.end()) {
                return current;
            }
            current = GetParent(current);
        }

        return nullptr;
    }

    bool findPreviewHandlerClsid(const wstring& filePath, CLSID& clsid) {
        wstring extension = fileExtension(filePath);
        if (extension.empty()) {
            return false;
        }

        const wstring previewHandlerKey = L"\\shellex\\{8895b1c6-b41f-4c1c-a562-0d564250836f}";
        vector<wstring> candidateKeys;
        candidateKeys.push_back(extension + previewHandlerKey);

        wstring progId;
        if (readRegistryDefault(HKEY_CLASSES_ROOT, extension, progId) && !progId.empty()) {
            candidateKeys.push_back(progId + previewHandlerKey);
        }

        candidateKeys.push_back(L"SystemFileAssociations\\" + extension + previewHandlerKey);

        for (const wstring& key : candidateKeys) {
            wstring clsidText;
            if (readRegistryDefault(HKEY_CLASSES_ROOT, key, clsidText) && !clsidText.empty()) {
                if (SUCCEEDED(CLSIDFromString(clsidText.c_str(), &clsid))) {
                    return true;
                }
            }
        }

        return false;
    }

    wstring fileExtension(const wstring& path) {
        size_t slash = path.find_last_of(L"\\/");
        size_t dot = path.find_last_of(L'.');
        if (dot == wstring::npos || (slash != wstring::npos && dot < slash)) {
            return L"";
        }

        wstring extension = path.substr(dot);
        transform(extension.begin(), extension.end(), extension.begin(),
                  [](wchar_t ch) { return (wchar_t)towlower(ch); });
        return extension;
    }

    bool readRegistryDefault(HKEY root, const wstring& subKey, wstring& value) {
        HKEY key = nullptr;
        if (RegOpenKeyExW(root, subKey.c_str(), 0, KEY_READ, &key) != ERROR_SUCCESS) {
            return false;
        }

        DWORD type = 0;
        DWORD size = 0;
        LONG result = RegQueryValueExW(key, nullptr, nullptr, &type, nullptr, &size);
        if (result != ERROR_SUCCESS || (type != REG_SZ && type != REG_EXPAND_SZ) || size == 0) {
            RegCloseKey(key);
            return false;
        }

        vector<wchar_t> buffer(size / sizeof(wchar_t) + 1, L'\0');
        result = RegQueryValueExW(key, nullptr, nullptr, &type, (LPBYTE)buffer.data(), &size);
        RegCloseKey(key);
        if (result != ERROR_SUCCESS) {
            return false;
        }

        value.assign(buffer.data());
        return !value.empty();
    }

    HWND examListView(int x, int y, int w, int h, bool onlyOpen) {
        RECT client = {};
        GetClientRect(window, &client);
        w = min(w, max(700, (int)client.right - x - 18));
        h = min(h, max(150, (int)client.bottom - y - 70));
        HWND list = CreateWindowExW(WS_EX_CLIENTEDGE, WC_LISTVIEWW, L"",
                                   WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
                                   x, y, w, h, window, nullptr, instance, nullptr);
        styleModernListView(list, 34);
        controls.push_back(list);

        addListColumn(list, 0, "Mã đề", 72);
        addListColumn(list, 1, "Tên đề", 220);
        addListColumn(list, 2, "Thời gian", 92, LVCFMT_CENTER);
        addListColumn(list, 3, "Mở lúc", 145);
        addListColumn(list, 4, "Đóng lúc", 145);
        addListColumn(list, 5, "Số câu", 72, LVCFMT_CENTER);
        addListColumn(list, 6, "Lượt", 60, LVCFMT_CENTER);
        addListColumn(list, 7, "Giáo viên", 145);
        addListColumn(list, 8, "Mật khẩu", 80, LVCFMT_CENTER);
        addListColumn(list, 9, "File", 60, LVCFMT_CENTER);
        addListColumn(list, 10, "Trạng thái", 100, LVCFMT_CENTER);

        int row = 0;
        for (Exam& exam : data.getExams()) {
            string now = QuizData::currentMinuteText();
            if (onlyOpen && !exam.canTake(now)) {
                continue;
            }

            string status = exam.statusText(now);
            vector<string> values = {
                exam.getExamId(),
                exam.getTitle(),
                to_string(exam.getDurationMinutes()) + " phut",
                exam.getStartAt().empty() ? "-" : exam.getStartAt(),
                exam.getCloseAt(),
                to_string(exam.totalQuestions()),
                to_string(exam.getAttemptLimit()),
                data.getUserNameById(exam.getCreatedBy()),
                exam.hasPassword() ? "Có" : "Không",
                exam.hasAttachment() ? "Có" : "Không",
                status
            };

            wstring wideValue = utf8ToWide(values[0]);
            LVITEMW item = {};
            item.mask = LVIF_TEXT;
            item.iItem = row;
            item.iSubItem = 0;
            item.pszText = const_cast<wchar_t*>(wideValue.c_str());
            SendMessageW(list, LVM_INSERTITEMW, 0, (LPARAM)&item);

            for (int col = 1; col < (int)values.size(); col++) {
                setListText(list, row, col, values[col]);
            }
            row++;
        }

        return list;
    }

    HWND userListView(int x, int y, int w, int h, string role, string keyword = "") {
        RECT client = {};
        GetClientRect(window, &client);
        w = min(w, max(700, (int)client.right - x - 18));
        h = max(h, 360);
        HWND list = CreateWindowExW(WS_EX_CLIENTEDGE, WC_LISTVIEWW, L"",
                                   WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
                                   x, y, w, h, window, nullptr, instance, nullptr);
        styleModernListView(list, 34);
        controls.push_back(list);

        addListColumn(list, 0, "ID", 125);
        addListColumn(list, 1, "Họ tên", 220);
        addListColumn(list, 2, "Email", 320);
        addListColumn(list, 3, role == "student" ? "Lớp" : "Lớp/môn phụ trách", 180);

        string normalizedKeyword = lowerText(keyword);
        vector<shared_ptr<User>> rows;
        for (shared_ptr<User> user : data.getUsers()) {
            string searchable = lowerText(user->getUserId() + " " + user->getName() + " " +
                                          user->getEmail() + " " + user->getClassName());
            if (user->getRole() == role &&
                (normalizedKeyword.empty() || searchable.find(normalizedKeyword) != string::npos)) {
                rows.push_back(user);
            }
        }

        sort(rows.begin(), rows.end(), [](shared_ptr<User> leftUser, shared_ptr<User> rightUser) {
            if (leftUser->getClassName() != rightUser->getClassName()) {
                return leftUser->getClassName() < rightUser->getClassName();
            }
            return leftUser->getName() < rightUser->getName();
        });

        for (int row = 0; row < (int)rows.size(); row++) {
            vector<string> values = {
                rows[row]->getUserId(),
                rows[row]->getName(),
                rows[row]->getEmail(),
                rows[row]->getClassName(),
            };

            wstring wideValue = utf8ToWide(values[0]);
            LVITEMW item = {};
            item.mask = LVIF_TEXT;
            item.iItem = row;
            item.iSubItem = 0;
            item.pszText = const_cast<wchar_t*>(wideValue.c_str());
            SendMessageW(list, LVM_INSERTITEMW, 0, (LPARAM)&item);

            for (int col = 1; col < (int)values.size(); col++) {
                setListText(list, row, col, values[col]);
            }
        }

        return list;
    }

    void addListColumn(HWND list, int index, string title, int width, int format = LVCFMT_LEFT) {
        wstring wideTitle = utf8ToWide(title);
        LVCOLUMNW column = {};
        column.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM | LVCF_FMT;
        column.pszText = const_cast<wchar_t*>(wideTitle.c_str());
        column.cx = width;
        column.iSubItem = index;
        column.fmt = format;
        SendMessageW(list, LVM_INSERTCOLUMNW, index, (LPARAM)&column);
    }

    void setListText(HWND list, int row, int col, string text) {
        wstring wideText = utf8ToWide(text);
        LVITEMW item = {};
        item.iSubItem = col;
        item.pszText = const_cast<wchar_t*>(wideText.c_str());
        SendMessageW(list, LVM_SETITEMTEXTW, row, (LPARAM)&item);
    }

    void styleModernListView(HWND list, int rowHeight) {
        SendMessageW(list, WM_SETFONT, (WPARAM)font, TRUE);
        ListView_SetExtendedListViewStyle(
            list,
            LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER | LVS_EX_INFOTIP | LVS_EX_GRIDLINES
        );
        ListView_SetBkColor(list, THEME_SURFACE);
        ListView_SetTextBkColor(list, THEME_SURFACE);
        ListView_SetTextColor(list, THEME_TEXT);

        HIMAGELIST imageList = ImageList_Create(1, rowHeight, ILC_COLOR32, 1, 1);
        if (imageList != nullptr) {
            ListView_SetImageList(list, imageList, LVSIL_SMALL);
            listImageLists.push_back(imageList);
        }

        HWND header = ListView_GetHeader(list);
        if (header != nullptr) {
            SendMessageW(header, WM_SETFONT, (WPARAM)smallFont, TRUE);
        }
    }

    string getText(int id) {
        HWND control = GetDlgItem(window, id);
        if (control == nullptr) {
            return "";
        }
        int len = GetWindowTextLengthW(control);
        wstring text(len + 1, L'\0');
        GetWindowTextW(control, text.data(), len + 1);
        text.resize(len);
        return wideToUtf8(text);
    }

    void setFocusTo(int id) {
        HWND control = GetDlgItem(window, id);
        if (control != nullptr) {
            SetFocus(control);
        }
    }

    void message(string text, string title = "Quiz App") {
        MessageBoxW(window, utf8ToWide(text).c_str(), utf8ToWide(title).c_str(), MB_OK);
    }

    void error(string text) {
        MessageBoxW(window, utf8ToWide(text).c_str(), L"Thông báo", MB_OK | MB_SETFOREGROUND);
    }

    void showShortcutHelp() {
        string text =
            "Phím tắt trong app:\r\n\r\n"
            "Tab: Chuyển sang ô/nút tiếp theo\r\n"
            "Shift + Tab: Quay lại ô/nút trước\r\n"
            "Enter: Thực hiện nút chính của màn hình hiện tại\r\n"
            "Esc: Quay lại dashboard hoặc màn hình chính\r\n"
            "Delete: Xóa câu hỏi khi đang ở màn hình Xóa câu hỏi\r\n"
            "Ctrl + L: Đăng xuất nhanh\r\n"
            "Ctrl + Q: Thoát app\r\n"
            "F1: Xem bảng phím tắt này";
        MessageBoxW(window, utf8ToWide(text).c_str(), L"Phím tắt", MB_OK | MB_ICONINFORMATION);
    }

    void goBackByShortcut() {
        if (currentScreen == SCREEN_MAIN) {
            DestroyWindow(window);
        } else if (currentUser == nullptr ||
                   currentScreen == SCREEN_LOGIN ||
                   currentScreen == SCREEN_REGISTER) {
            showMainScreen();
        } else {
            showCurrentDashboard();
        }
    }

    bool submitDefaultByShortcut() {
        switch (currentScreen) {
            case SCREEN_LOGIN:
                submitLogin();
                return true;
            case SCREEN_REGISTER:
                submitRegisterStudent();
                return true;
            case SCREEN_PROFILE:
                submitProfileUpdate();
                return true;
            case SCREEN_CREATE_TEACHER:
                submitCreateTeacher();
                return true;
            case SCREEN_ADD_QUESTION:
                submitAddQuestion();
                return true;
            case SCREEN_AI_QUESTION_GENERATOR:
                submitGenerateAiQuestions();
                return true;
            case SCREEN_IMPORT_QUESTIONS:
                submitImportQuestions();
                return true;
            case SCREEN_DELETE_QUESTION:
                submitDeleteQuestion();
                return true;
            case SCREEN_CREATE_EXAM:
                submitCreateExam();
                return true;
            case SCREEN_TOGGLE_EXAM:
                submitToggleExam();
                return true;
            case SCREEN_TAKE_EXAM_SELECT:
            case SCREEN_TAKE_EXAM_FORM:
                submitTakeExam();
                return true;
            default:
                return false;
        }
    }

    void title(string text, string subtitle = "") {
        RedrawWindow(
            window,
            nullptr,
            nullptr,
            RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN | RDW_UPDATENOW
        );

        HWND brand = label("QUIZ APP", 24, 22, 190, 30);
        SendMessageW(brand, WM_SETFONT, (WPARAM)brandFont, TRUE);
        controlTextColors[brand] = THEME_PRIMARY;
        sidebarLabels[brand] = true;
        HWND titleLabel = label(text, 280, 24, 820, 38);
        SendMessageW(titleLabel, WM_SETFONT, (WPARAM)titleFont, TRUE);
        if (!subtitle.empty()) {
            HWND subtitleLabel = label(subtitle, 282, 66, 820, 24);
            SendMessageW(subtitleLabel, WM_SETFONT, (WPARAM)smallFont, TRUE);
        }
    }
