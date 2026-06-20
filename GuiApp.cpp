#include "GuiApp.h"
#include "src/data/QuizData.h"
#include "src/services/AIQuestionService.h"
#include "src/ui/ControlIds.h"
#include "src/ui/Layout.h"

class GuiApp {
private:
    static constexpr COLORREF THEME_BACKGROUND = RGB(245, 246, 247);
    static constexpr COLORREF THEME_SURFACE = RGB(255, 255, 255);
    static constexpr COLORREF THEME_SIDEBAR = RGB(255, 255, 255);
    static constexpr COLORREF THEME_BORDER = RGB(227, 229, 232);
    static constexpr COLORREF THEME_TEXT = RGB(26, 26, 26);
    static constexpr COLORREF THEME_MUTED = RGB(107, 113, 120);
    static constexpr COLORREF THEME_PRIMARY = RGB(37, 99, 235);
    static constexpr COLORREF THEME_PRIMARY_HOVER = RGB(29, 78, 216);
    static constexpr COLORREF THEME_PRESSED = RGB(241, 243, 245);
    static constexpr COLORREF THEME_DISABLED = RGB(227, 229, 232);

    HINSTANCE instance;
    HWND window = nullptr;
    HFONT font = nullptr;
    HFONT titleFont = nullptr;
    HFONT brandFont = nullptr;
    HFONT smallFont = nullptr;
    HBRUSH backgroundBrush = nullptr;
    HBRUSH surfaceBrush = nullptr;
    HBRUSH sidebarBrush = nullptr;
    HBRUSH inputBrush = nullptr;
    HBRUSH loginBackgroundBrush = nullptr;
    QuizData data;
    AIQuestionService aiQuestionService;
    shared_ptr<User> currentUser = nullptr;
    AIQuestionRequest aiQuestionRequest;
    vector<GeneratedQuestionDraft> generatedQuestionDrafts;
    string aiQuestionStatus;
    vector<HWND> controls;
    vector<HBITMAP> imageBitmaps;
    vector<HIMAGELIST> listImageLists;
    vector<IPreviewHandler*> previewHandlers;
    map<HWND, IPreviewHandler*> previewHandlerByHost;
    map<HWND, bool> primaryButtons;
    map<HWND, bool> navButtons;
    map<HWND, bool> answerChoiceButtons;
    map<HWND, COLORREF> controlTextColors;
    map<HWND, bool> surfaceLabels;
    map<HWND, bool> sidebarLabels;
    vector<array<HWND, 4>> answerOptions;
    vector<array<char, 4>> activeAnswerValues;
    vector<HWND> answerCombos;
    vector<string> activeQuestionIds;
    map<string, char> activeExamDraftAnswers;
    vector<vector<int>> activeExamQuestionPages;
    vector<string> activeExamQuestionOrder;
    map<string, array<string, 4>> activeExamDisplayedOptions;
    map<string, array<char, 4>> activeExamDisplayedValues;
    int activeExamQuestionPage = 0;
    string activeExamId;
    string activeSessionId;
    int examViolationCount = 0;
    chrono::steady_clock::time_point allowExamViewerFocusLossUntil{};
    bool antiCheatDialogOpen = false;
    bool examAppActive = true;
    bool pendingExamViolationNotice = false;
    bool pendingExamAutoSubmit = false;
    bool examSubmissionInProgress = false;
    HWND examTimerLabel = nullptr;
    chrono::system_clock::time_point activeExamEndTime;
    bool examTimerActive = false;
    AppScreen currentScreen = SCREEN_MAIN;
    ULONG_PTR gdiplusToken = 0;
    bool comInitialized = false;
    static constexpr UINT_PTR EXAM_TIMER_ID = 9001;

public:
    GuiApp(HINSTANCE instance) : instance(instance) {}

#include "src/ui/features/GuiCore.inl"
#include "src/ui/features/AuthScreens.inl"
#include "src/ui/features/DashboardScreens.inl"
#include "src/ui/features/QuestionScreens.inl"
#include "src/ui/features/ExamScreens.inl"
#include "src/ui/features/ResultScreens.inl"
#include "src/ui/features/GuiUtilities.inl"
};

int RunQuizApp(HINSTANCE hInstance) {
    GuiApp app(hInstance);
    return app.run();
}
