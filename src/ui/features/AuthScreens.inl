    void showMainScreen() {
        showLoginScreen();
    }

    void showSampleAccounts() {
        clearControls();
        title("Tài khoản mẫu", "Dùng để kiểm tra nhanh dữ liệu trong bộ nhớ.");
        string text =
            "Admin:   admin@quiz.local / 123456\r\n"
            "Teacher: teacher@quiz.local / 123456\r\n"
            "Student: an@quiz.local / 123456\r\n\r\n"
            "Học sinh được đăng ký ở màn hình chính.\r\n"
            "Admin có sẵn trong hệ thống.\r\n"
            "Giáo viên do admin tạo.";
        edit(text, 280, 120, 520, 220, 0, false, true, true);
        button("Quay lại", 280, 370, 150, 40, ID_BACK_MAIN);
    }

    void showLoginScreen() {
        clearControls();
        currentScreen = SCREEN_LOGIN;
        InvalidateRect(window, nullptr, TRUE);
        UpdateWindow(window);

        RECT client = {};
        GetClientRect(window, &client);
        int centerX = (int)(client.right - client.left) / 2;
        int cardX = centerX - Layout::Login::CardHalfWidth;
        int fieldX = cardX + 35;

        string logoPath = appDirectory() + "\\hcmute-logo.png";
        imageBox(logoPath, centerX - 45, 8, 90, 100);

        centeredLabel(
            "TRƯỜNG ĐẠI HỌC CÔNG NGHỆ KỸ THUẬT TP.HCM",
            centerX - 330, 108, 660, 38, brandFont, RGB(8, 47, 89)
        );

        HWND formTitle = label("ĐĂNG NHẬP", fieldX, 168, 470, 38);
        SendMessageW(formTitle, WM_SETFONT, (WPARAM)titleFont, TRUE);
        controlTextColors[formTitle] = THEME_TEXT;
        surfaceLabels[formTitle] = true;

        HWND subtitle = label("Hệ thống quản lý và thi trắc nghiệm", fieldX + 2, 210, 470, 24);
        SendMessageW(subtitle, WM_SETFONT, (WPARAM)smallFont, TRUE);
        controlTextColors[subtitle] = THEME_MUTED;
        surfaceLabels[subtitle] = true;

        HWND emailLabel = label("Tài khoản hoặc email", fieldX + 2, 245, 470, 22);
        SendMessageW(emailLabel, WM_SETFONT, (WPARAM)smallFont, TRUE);
        controlTextColors[emailLabel] = THEME_MUTED;
        surfaceLabels[emailLabel] = true;
        edit("", fieldX, 267, 470, 42, ID_FIELD_LOGIN_EMAIL);

        HWND passwordLabel = label("Mật khẩu", fieldX + 2, 320, 470, 22);
        SendMessageW(passwordLabel, WM_SETFONT, (WPARAM)smallFont, TRUE);
        controlTextColors[passwordLabel] = THEME_MUTED;
        surfaceLabels[passwordLabel] = true;
        edit("", fieldX, 342, 470, 42, ID_FIELD_LOGIN_PASSWORD, true);

        defaultButton("Đăng nhập", fieldX, 405, 470, 44, ID_LOGIN_SUBMIT);
        button("Đăng ký tài khoản học sinh", fieldX, 462, 470, 42, ID_MAIN_REGISTER);

        HWND passwordHelp = centeredLabel(
            "Quên mật khẩu? Vui lòng liên hệ giáo viên hoặc quản trị viên.",
            fieldX, 515, 470, 24, smallFont, THEME_MUTED
        );
        surfaceLabels[passwordHelp] = true;

        centeredLabel(
            "HCMUTE Quiz App • Học tập, kiểm tra và quản lý lớp học",
            centerX - 330, 555, 660, 24, smallFont, THEME_MUTED
        );

        RedrawWindow(
            window,
            nullptr,
            nullptr,
            RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN | RDW_UPDATENOW
        );
        setFocusTo(ID_FIELD_LOGIN_EMAIL);
    }

    void showRegisterScreen() {
        clearControls();
        currentScreen = SCREEN_REGISTER;
        title("Đăng ký học sinh", "Chỉ học sinh được tự đăng ký.");
        label("Họ tên", 300, 130, 130, 26);
        edit("", 450, 126, 280, 30, ID_FIELD_REGISTER_NAME);
        label("Email", 300, 175, 130, 26);
        edit("", 450, 171, 280, 30, ID_FIELD_REGISTER_EMAIL);
        label("Mật khẩu", 300, 220, 130, 26);
        edit("", 450, 216, 280, 30, ID_FIELD_REGISTER_PASSWORD, true);
        label("Lớp", 300, 265, 130, 26);
        edit("", 450, 261, 280, 30, ID_FIELD_REGISTER_CLASS);
        defaultButton("Đăng ký", 450, 320, 125, 38, ID_REGISTER_SUBMIT);
        button("Quay lại", 585, 320, 125, 38, ID_BACK_MAIN);
        setFocusTo(ID_FIELD_REGISTER_NAME);
    }

    void submitLogin() {
        string email = getText(ID_FIELD_LOGIN_EMAIL);
        string password = getText(ID_FIELD_LOGIN_PASSWORD);
        shared_ptr<User> user = data.findUserByEmail(email);
        if (user == nullptr || !user->checkPassword(password)) {
            data.writeAuditLog(email, "", "LOGIN_FAILED", "Sai email hoặc mật khẩu");
            error("Sai email hoặc mật khẩu.");
            return;
        }

        currentUser = user;
        data.writeAuditLog(user->getUserId(), "", "LOGIN_SUCCESS", "Đăng nhập thành công");
        if (user->getRole() == "admin") showAdminDashboard();
        else if (user->getRole() == "teacher") showTeacherDashboard();
        else showStudentDashboard();
    }

    void submitRegisterStudent() {
        string name = getText(ID_FIELD_REGISTER_NAME);
        string email = getText(ID_FIELD_REGISTER_EMAIL);
        string password = getText(ID_FIELD_REGISTER_PASSWORD);
        string className = normalizeClassName(getText(ID_FIELD_REGISTER_CLASS));

        if (name.empty() || email.empty() || password.empty() || className.empty()) {
            error("Vui lòng nhập đầy đủ thông tin.");
            return;
        }
        if (containsRoleKeyword(name) || containsRoleKeyword(password)) {
            error("Tên và mật khẩu không được chứa từ khóa: admin, teacher, student.");
            return;
        }

        if (data.findUserByEmail(email) != nullptr) {
            error("Email này đã tồn tại.");
            return;
        }

        if (!data.addStudent(name, email, password, className)) {
            error("Đăng ký thất bại. Không thể lưu tài khoản.\r\n"
                  "Vui lòng kiểm tra kết nối Internet rồi thử lại.");
            return;
        }

        message("Đăng ký thành công. Bạn có thể đăng nhập.");
        showLoginScreen();
    }

    void addNav(string roleTitle) {
        HWND role = label(roleTitle, 24, 68, 190, 28);
        SendMessageW(role, WM_SETFONT, (WPARAM)brandFont, TRUE);
        controlTextColors[role] = THEME_TEXT;
        sidebarLabels[role] = true;
        if (currentScreen != SCREEN_VIEW) {
            navButton("Hướng dẫn", 16, 590, 208, 38, ID_HELP);
        }
        navButton("Thông tin", 16, 638, 208, 38, ID_PROFILE);
        navButton("Đăng xuất", 16, 686, 208, 38, ID_LOGOUT);
    }

    void showUserGuide() {
        clearControls();
        currentScreen = SCREEN_VIEW;

        if (currentUser == nullptr) {
            showMainScreen();
            return;
        }

        string roleTitle = "Học sinh";
        if (currentUser->getRole() == "admin") roleTitle = "Quản trị viên";
        else if (currentUser->getRole() == "teacher") roleTitle = "Giáo viên";

        title("Hướng dẫn sử dụng", "Hướng dẫn nhanh cho " + roleTitle + ".");
        addNav(roleTitle);

        edit(guideTextForRole(currentUser->getRole()), 260, 95, 660, 430, 0, false, true, true);
    }

    string guideTextForRole(string role) {
        if (role == "admin") {
            return
                "HƯỚNG DẪN CHO QUẢN TRỊ VIÊN\r\n\r\n"
                "1. Quản lý học sinh\r\n"
                "- Chọn Học sinh để xem danh sách học sinh theo bảng.\r\n"
                "- Nhập ID học sinh và Lớp mới, sau đó bấm Chuyển lớp.\r\n"
                "- Nhập ID học sinh và Mật khẩu mới để đặt lại mật khẩu.\r\n"
                "- Nhập ID học sinh và bấm Xóa học sinh để xóa tài khoản.\r\n\r\n"
                "2. Quản lý giáo viên\r\n"
                "- Chọn Giáo viên để xem danh sách giáo viên.\r\n"
                "- Chọn Tạo giáo viên để tạo tài khoản mới.\r\n"
                "- Nhập ID và Phụ trách mới để đổi lớp/môn phụ trách.\r\n"
                "- Nhập ID và Mật khẩu mới để đặt lại mật khẩu.\r\n\r\n"
                "3. Quản lý đề thi và kết quả\r\n"
                "- Chọn Đề thi để xem danh sách đề.\r\n"
                "- Chọn Mở/đóng đề để thay đổi trạng thái đề thi.\r\n"
                "- Chọn Kết quả để xem điểm của tất cả học sinh.\r\n\r\n"
                "4. Thông tin cá nhân\r\n"
                "- Chọn Thông tin để đổi tên, email hoặc mật khẩu.\r\n"
                "- Sau khi lưu, app sẽ đăng xuất và yêu cầu đăng nhập lại.";
        }

        if (role == "teacher") {
            return
                "HƯỚNG DẪN CHO GIÁO VIÊN\r\n\r\n"
                "1. Quản lý câu hỏi\r\n"
                "- Chọn Câu hỏi để xem ngân hàng câu hỏi.\r\n"
                "- Chọn Thêm câu hỏi để thêm từng câu hỏi.\r\n"
                "- Chọn Nhập câu hỏi để nhập nhiều câu hỏi từ CSV.\r\n"
                "- Khi nhập CSV, chỉ cần nhập Môn học/Chủ đề một lần ở bên ngoài.\r\n"
                "- File CSV cần các cột: content, optionA, optionB, optionC, optionD, correctAnswer.\r\n"
                "- Có thể chọn file từ máy, tải URL hoặc mở Mẫu CSV.\r\n\r\n"
                "2. Tạo đề thi\r\n"
                "- Chọn Tải đề thi.\r\n"
                "- Nhập tên đề, thời gian làm bài, thời gian mở và đóng đề.\r\n"
                "- Với ngân hàng câu hỏi, nhập mã câu hỏi, ví dụ: q1 q2 q3.\r\n"
                "- Với file gốc, để trống mã câu hỏi và nhập đáp án, ví dụ: A C B B A.\r\n"
                "- Chọn file .docx/.pdf/.jpg/.png hoặc dán liên kết công khai.\r\n"
                "- Có thể bỏ trống mật khẩu nếu đề không cần bảo vệ.\r\n\r\n"
                "3. Theo dõi đề và kết quả\r\n"
                "- Chọn Đề thi để xem danh sách đề.\r\n"
                "- Chọn Mở/đóng đề để thay đổi trạng thái.\r\n"
                "- Chọn Kết quả để xem điểm học sinh.\r\n\r\n"
                "4. Thông tin cá nhân\r\n"
                "- Chọn Thông tin để đổi tên, email hoặc mật khẩu.\r\n"
                "- Sau khi lưu, app sẽ đăng xuất và yêu cầu đăng nhập lại.";
        }

        return
            "HƯỚNG DẪN CHO HỌC SINH\r\n\r\n"
            "1. Đăng ký và đăng nhập\r\n"
            "- Học sinh có thể tự đăng ký tại màn hình đăng nhập.\r\n"
            "- Sau khi đăng ký thành công, đăng nhập bằng email và mật khẩu đã tạo.\r\n\r\n"
            "2. Xem và làm bài thi\r\n"
            "- Chọn Đề đang mở để xem các đề có thể làm.\r\n"
            "- Chọn Làm bài, nhập mã đề và mật khẩu nếu có.\r\n"
            "- Chỉ có thể bắt đầu khi đã tới thời gian mở đề.\r\n"
            "- Khi vào bài, chọn đáp án A/B/C/D bằng chuột.\r\n"
            "- Nếu đề có file gốc, mở file để xem nội dung rồi chọn đáp án trong app.\r\n"
            "- App hiển thị thời gian còn lại và tự động nộp bài khi hết giờ.\r\n\r\n"
            "3. Xem kết quả\r\n"
            "- Chọn Lịch sử điểm để xem điểm các bài đã nộp.\r\n\r\n"
            "4. Thông tin cá nhân\r\n"
            "- Chọn Thông tin để đổi tên, email hoặc mật khẩu.\r\n"
            "- Sau khi lưu, app sẽ đăng xuất và yêu cầu đăng nhập lại.";
    }

    void showProfileScreen() {
        if (currentUser == nullptr) {
            showMainScreen();
            return;
        }

        clearControls();
        currentScreen = SCREEN_PROFILE;
        title("Thông tin cá nhân", "Sau khi cập nhật, bạn cần đăng nhập lại.");

        string roleTitle = "Học sinh";
        if (currentUser->getRole() == "admin") roleTitle = "Quản trị viên";
        else if (currentUser->getRole() == "teacher") roleTitle = "Giáo viên";
        addNav(roleTitle);

        label("Họ tên", 300, 130, 170, 26);
        edit(currentUser->getName(), 500, 126, 360, 34, 7001);
        label("Email/Gmail", 300, 180, 170, 26);
        edit(currentUser->getEmail(), 500, 176, 360, 34, 7002);
        label("Mật khẩu hiện tại", 300, 230, 170, 26);
        edit("", 500, 226, 360, 34, 7003, true);
        label("Mật khẩu mới", 300, 280, 170, 26);
        edit("", 500, 276, 360, 34, 7004, true);
        label("Xác nhận mật khẩu mới", 300, 330, 190, 26);
        edit("", 500, 326, 360, 34, 7005, true);
        label("Lớp/phụ trách", 300, 380, 170, 26);
        edit(currentUser->getClassName(), 500, 376, 360, 34, 0, false, false, true);

        defaultButton("Lưu thay đổi", 500, 445, 170, 42, ID_PROFILE_SAVE);
        button("Về bảng điều khiển", 690, 445, 170, 42, ID_DASHBOARD);
        setFocusTo(7001);
    }

    void submitProfileUpdate() {
        if (currentUser == nullptr) {
            showMainScreen();
            return;
        }

        string userId = currentUser->getUserId();
        string name = getText(7001);
        string email = getText(7002);
        string currentPassword = getText(7003);
        string newPassword = getText(7004);
        string confirmPassword = getText(7005);

        if (name.empty() || email.empty() || currentPassword.empty() ||
            newPassword.empty() || confirmPassword.empty()) {
            error("Vui lòng nhập đầy đủ họ tên, email và các ô mật khẩu.");
            return;
        }
        if (email.find('@') == string::npos || email.find('.') == string::npos) {
            error("Email/Gmail không hợp lệ.");
            return;
        }
        if (!currentUser->checkPassword(currentPassword)) {
            error("Mật khẩu hiện tại không đúng.");
            return;
        }
        if (newPassword != confirmPassword) {
            error("Hai lần nhập mật khẩu mới không trùng nhau.");
            return;
        }
        if (newPassword.size() < 6) {
            error("Mật khẩu mới phải có ít nhất 6 ký tự.");
            return;
        }
        if (containsRoleKeyword(name) || containsRoleKeyword(newPassword)) {
            error("Tên và mật khẩu không được chứa từ khóa: admin, teacher, student.");
            return;
        }

        shared_ptr<User> existingUser = data.findUserByEmail(email);
        if (existingUser != nullptr && existingUser->getUserId() != userId) {
            error("Email này đã được tài khoản khác sử dụng.");
            return;
        }

        if (!data.updateUserProfile(userId, name, email, newPassword)) {
            error("Cập nhật thông tin thất bại. Vui lòng thử lại.");
            return;
        }

        currentUser = nullptr;
        message("Đã cập nhật thông tin. Vui lòng đăng nhập lại bằng thông tin mới.");
        showLoginScreen();
    }
