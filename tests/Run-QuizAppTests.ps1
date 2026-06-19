param(
    [string]$AdminEmail = "admin@quiz.local",
    [Parameter(Mandatory = $true)]
    [string]$AdminPassword,
    [string]$TeacherEmail = "teacher@quiz.local",
    [Parameter(Mandatory = $true)]
    [string]$TeacherPassword,
    [string]$StudentEmail = "phamdinhphuocpk@gmail.com",
    [Parameter(Mandatory = $true)]
    [string]$StudentPassword,
    [string]$Executable = ""
)

$ErrorActionPreference = "Stop"
$projectRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
if ([string]::IsNullOrWhiteSpace($Executable)) {
    $Executable = Join-Path $projectRoot "build\QuizApp.exe"
}
$resultDirectory = Join-Path $projectRoot "test-results"
$fixtureCsv = Join-Path $PSScriptRoot "fixtures\questions_vi.csv"
$sampleCsv = Join-Path $projectRoot "sample_questions.csv"

New-Item -ItemType Directory -Force -Path $resultDirectory | Out-Null
$env:QUIZAPP_SKIP_FIREBASE = "1"

Add-Type -AssemblyName System.Windows.Forms
Add-Type -AssemblyName Microsoft.VisualBasic

$interopSource = @'
using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;

public static class QuizAppTestInterop
{
    public delegate bool EnumProc(IntPtr hwnd, IntPtr lParam);

    [DllImport("user32.dll")]
    public static extern bool EnumChildWindows(IntPtr hwnd, EnumProc callback, IntPtr lParam);

    [DllImport("user32.dll", CharSet = CharSet.Unicode)]
    public static extern int GetClassName(IntPtr hwnd, StringBuilder text, int max);

    [DllImport("user32.dll", CharSet = CharSet.Unicode)]
    public static extern int GetWindowText(IntPtr hwnd, StringBuilder text, int max);

    [DllImport("user32.dll")]
    public static extern int GetDlgCtrlID(IntPtr hwnd);

    [DllImport("user32.dll")]
    public static extern bool GetWindowRect(IntPtr hwnd, out RECT rect);

    [DllImport("user32.dll")]
    public static extern bool IsWindowVisible(IntPtr hwnd);

    [StructLayout(LayoutKind.Sequential)]
    public struct RECT
    {
        public int Left;
        public int Top;
        public int Right;
        public int Bottom;
    }

    [DllImport("user32.dll")]
    public static extern bool PostMessage(IntPtr hwnd, uint message, IntPtr wParam, IntPtr lParam);

    [DllImport("user32.dll", EntryPoint = "SendMessageW")]
    private static extern IntPtr SendMessage(IntPtr hwnd, uint message, IntPtr wParam, IntPtr lParam);

    public static string[] VisibleTexts(IntPtr parent)
    {
        var values = new List<string>();
        EnumChildWindows(parent, (hwnd, _) =>
        {
            var className = new StringBuilder(80);
            var text = new StringBuilder(1024);
            GetClassName(hwnd, className, className.Capacity);
            GetWindowText(hwnd, text, text.Capacity);
            if (!className.ToString().Equals("Edit", StringComparison.OrdinalIgnoreCase) &&
                text.Length > 0)
            {
                values.Add(text.ToString());
            }
            return true;
        }, IntPtr.Zero);
        return values.ToArray();
    }

    public static int ListViewRows(IntPtr parent)
    {
        int rows = 0;
        EnumChildWindows(parent, (hwnd, _) =>
        {
            var className = new StringBuilder(80);
            GetClassName(hwnd, className, className.Capacity);
            if (className.ToString().Equals("SysListView32", StringComparison.OrdinalIgnoreCase))
            {
                rows += (int)SendMessage(hwnd, 0x1004, IntPtr.Zero, IntPtr.Zero);
            }
            return true;
        }, IntPtr.Zero);
        return rows;
    }

    public static int ControlCount(IntPtr parent, string wantedClass)
    {
        int count = 0;
        EnumChildWindows(parent, (hwnd, _) =>
        {
            var className = new StringBuilder(80);
            GetClassName(hwnd, className, className.Capacity);
            if (className.ToString().Equals(wantedClass, StringComparison.OrdinalIgnoreCase))
            {
                count++;
            }
            return true;
        }, IntPtr.Zero);
        return count;
    }

    public static bool HasControlId(IntPtr parent, int id)
    {
        bool found = false;
        EnumChildWindows(parent, (hwnd, _) =>
        {
            if (GetDlgCtrlID(hwnd) == id)
            {
                found = true;
            }
            return true;
        }, IntPtr.Zero);
        return found;
    }

    public static IntPtr FindControl(IntPtr parent, int id)
    {
        IntPtr result = IntPtr.Zero;
        EnumChildWindows(parent, (hwnd, _) =>
        {
            if (GetDlgCtrlID(hwnd) == id)
            {
                result = hwnd;
                return false;
            }
            return true;
        }, IntPtr.Zero);
        return result;
    }

    public static bool SetControlText(IntPtr parent, int id, string value)
    {
        IntPtr control = FindControl(parent, id);
        if (control == IntPtr.Zero)
        {
            return false;
        }

        IntPtr text = Marshal.StringToHGlobalUni(value);
        try
        {
            SendMessage(control, 0x000C, IntPtr.Zero, text);
        }
        finally
        {
            Marshal.FreeHGlobal(text);
        }
        return true;
    }

    public static string[] OverlappingButtons(IntPtr parent)
    {
        var buttons = new List<Tuple<IntPtr, string, RECT>>();
        EnumChildWindows(parent, (hwnd, _) =>
        {
            var className = new StringBuilder(80);
            GetClassName(hwnd, className, className.Capacity);
            if (className.ToString().Equals("Button", StringComparison.OrdinalIgnoreCase) &&
                IsWindowVisible(hwnd))
            {
                var text = new StringBuilder(256);
                GetWindowText(hwnd, text, text.Capacity);
                RECT rect;
                if (GetWindowRect(hwnd, out rect))
                {
                    buttons.Add(Tuple.Create(hwnd, text.ToString(), rect));
                }
            }
            return true;
        }, IntPtr.Zero);

        var overlaps = new List<string>();
        for (int i = 0; i < buttons.Count; i++)
        {
            for (int j = i + 1; j < buttons.Count; j++)
            {
                RECT left = buttons[i].Item3;
                RECT right = buttons[j].Item3;
                bool intersects = left.Left < right.Right &&
                                  left.Right > right.Left &&
                                  left.Top < right.Bottom &&
                                  left.Bottom > right.Top;
                if (intersects)
                {
                    overlaps.Add("'" + buttons[i].Item2 + "' <> '" + buttons[j].Item2 + "'");
                }
            }
        }
        return overlaps.ToArray();
    }
}
'@

Add-Type -TypeDefinition $interopSource

$script:results = [System.Collections.Generic.List[object]]::new()
$script:activeProcess = $null

function Add-TestResult {
    param(
        [string]$Role,
        [string]$Feature,
        [bool]$Passed,
        [string]$Details
    )

    $script:results.Add([pscustomobject]@{
        Role = $Role
        Feature = $Feature
        Passed = $Passed
        Details = $Details
    })
}

function Stop-QuizApp {
    if ($script:activeProcess -and -not $script:activeProcess.HasExited) {
        Stop-Process -Id $script:activeProcess.Id -Force -ErrorAction SilentlyContinue
    }
    $script:activeProcess = $null
}

function Wait-AppWindow {
    param([int]$TimeoutSeconds = 120)

    $deadline = (Get-Date).AddSeconds($TimeoutSeconds)
    while ((Get-Date) -lt $deadline) {
        $script:activeProcess.Refresh()
        if ($script:activeProcess.HasExited) {
            throw "Quiz App đã dừng ngoài dự kiến với exit code $($script:activeProcess.ExitCode)."
        }
        if ($script:activeProcess.MainWindowHandle -ne [IntPtr]::Zero) {
            return
        }
        Start-Sleep -Milliseconds 500
    }

    throw "Quiz App không tạo cửa sổ chính sau $TimeoutSeconds giây."
}

function Get-Texts {
    return [QuizAppTestInterop]::VisibleTexts($script:activeProcess.MainWindowHandle)
}

function Test-VisibleText {
    param([string]$Text)
    return [bool]((Get-Texts | Where-Object { $_.Contains($Text) } | Select-Object -First 1))
}

function Invoke-AppCommand {
    param([int]$CommandId, [int]$DelayMilliseconds = 700)
    [QuizAppTestInterop]::PostMessage(
        $script:activeProcess.MainWindowHandle,
        0x0111,
        [IntPtr]$CommandId,
        [IntPtr]0
    ) | Out-Null
    Start-Sleep -Milliseconds $DelayMilliseconds
    $script:activeProcess.Refresh()
}

function Set-Field {
    param([int]$Id, [string]$Value)

    if (-not [QuizAppTestInterop]::SetControlText(
        $script:activeProcess.MainWindowHandle,
        $Id,
        $Value
    )) {
        throw "Không tìm thấy ô nhập có ID $Id."
    }
}

function Start-LoggedInApp {
    param(
        [string]$Email,
        [string]$Password,
        [string]$ExpectedDashboard
    )

    for ($attempt = 1; $attempt -le 2; $attempt++) {
        Stop-QuizApp
        $script:activeProcess = Start-Process -FilePath $Executable -PassThru
        Wait-AppWindow

        Set-Field 2001 $Email
        Set-Field 2002 $Password
        Invoke-AppCommand 1004 2000
        $script:activeProcess.Refresh()

        if (Test-VisibleText $ExpectedDashboard) {
            return $true
        }
    }

    return $false
}

function Test-Screen {
    param(
        [string]$Role,
        [int]$CommandId,
        [string]$ExpectedTitle,
        [int[]]$ExpectedControlIds = @(),
        [int]$DelayMilliseconds = 700,
        [switch]$RecordRows
    )

    Invoke-AppCommand $CommandId $DelayMilliseconds
    $opened = Test-VisibleText $ExpectedTitle
    $missingIds = @(
        $ExpectedControlIds |
        Where-Object {
            -not [QuizAppTestInterop]::HasControlId(
                $script:activeProcess.MainWindowHandle,
                $_
            )
        }
    )
    $rows = if ($RecordRows) {
        [QuizAppTestInterop]::ListViewRows($script:activeProcess.MainWindowHandle)
    } else {
        0
    }
    $overlaps = [QuizAppTestInterop]::OverlappingButtons(
        $script:activeProcess.MainWindowHandle
    )
    $passed = $opened -and $missingIds.Count -eq 0 -and
              $overlaps.Count -eq 0 -and $script:activeProcess.Responding
    $details = "opened=$opened; responding=$($script:activeProcess.Responding)"
    if ($RecordRows) {
        $details += "; rows=$rows"
    }
    if ($missingIds.Count -gt 0) {
        $details += "; missingControlIds=$($missingIds -join ',')"
    }
    if ($overlaps.Count -gt 0) {
        $details += "; overlappingButtons=$($overlaps -join ' | ')"
    }
    Add-TestResult $Role $ExpectedTitle $passed $details
    Invoke-AppCommand 1050 450
}

function Test-ProfileValidation {
    param([string]$Role)

    Invoke-AppCommand 1006
    $requiredIds = 7001, 7002, 7003, 7004, 7005
    $missing = @(
        $requiredIds |
        Where-Object {
            -not [QuizAppTestInterop]::HasControlId(
                $script:activeProcess.MainWindowHandle,
                $_
            )
        }
    )
    $passed = (Test-VisibleText "Thông tin cá nhân") -and $missing.Count -eq 0
    $details = if ($missing.Count -eq 0) {
        "Đủ họ tên, email, mật khẩu hiện tại, mật khẩu mới và xác nhận."
    } else {
        "Thiếu control: $($missing -join ',')"
    }
    Add-TestResult $Role "Hồ sơ và ba lớp nhập mật khẩu" $passed $details
    Invoke-AppCommand 1050 450
}

function Test-Role {
    param(
        [string]$Role,
        [string]$Email,
        [string]$Password,
        [string]$Dashboard,
        [object[]]$Screens
    )

    $loggedIn = Start-LoggedInApp $Email $Password $Dashboard
    $loginDetails = "dashboard='$Dashboard'; responding=$($script:activeProcess.Responding)"
    if (-not $loggedIn) {
        $visible = (Get-Texts | Select-Object -First 8) -join " | "
        $loginDetails += "; visible=$visible"
    }
    Add-TestResult $Role "Đăng nhập và phân quyền" $loggedIn (
        $loginDetails
    )
    if (-not $loggedIn) {
        Stop-QuizApp
        return
    }

    foreach ($screen in $Screens) {
        Test-Screen `
            -Role $Role `
            -CommandId $screen.Command `
            -ExpectedTitle $screen.Title `
            -ExpectedControlIds $screen.Controls `
            -DelayMilliseconds $(if ($screen.Delay) { $screen.Delay } else { 700 }) `
            -RecordRows:$screen.Rows
    }

    Test-ProfileValidation $Role

    Invoke-AppCommand 1008
    Add-TestResult $Role "Hướng dẫn sử dụng" (
        Test-VisibleText "Hướng dẫn sử dụng"
    ) "Màn hình hướng dẫn mở được."
    Invoke-AppCommand 1050 450

    Invoke-AppCommand 1009
    Add-TestResult $Role "Đăng xuất" (
        Test-VisibleText "ĐĂNG NHẬP"
    ) "Quay về màn hình đăng nhập."

    Stop-QuizApp
}

try {
    if (-not (Test-Path $Executable)) {
        throw "Không tìm thấy executable: $Executable"
    }

    $sampleHeader = (Get-Content -Encoding UTF8 $sampleCsv -TotalCount 1)
    $fixtureHeader = (Get-Content -Encoding UTF8 $fixtureCsv -TotalCount 1)
    $expectedHeader = "content,optionA,optionB,optionC,optionD,correctAnswer,difficulty"
    $legacyHeader = "content,optionA,optionB,optionC,optionD,correctAnswer"
    Add-TestResult "Chung" "File CSV mẫu" ($sampleHeader -eq $expectedHeader) (
        "header='$sampleHeader'"
    )
    Add-TestResult "Chung" "CSV fixture tiếng Việt" ($fixtureHeader -eq $legacyHeader) (
        "header='$fixtureHeader'; rows=$((Import-Csv $fixtureCsv).Count)"
    )

    $adminScreens = @(
        @{ Command = 1010; Title = "Quản lý học sinh"; Controls = @(6101, 6102, 6103, 6104); Rows = $true; Delay = 5000 },
        @{ Command = 1011; Title = "Quản lý giáo viên"; Controls = @(6201, 6202, 6203, 6204); Rows = $true; Delay = 5000 },
        @{ Command = 1012; Title = "Tạo tài khoản giáo viên"; Controls = @(3001, 3002, 3003, 3004); Rows = $false },
        @{ Command = 1013; Title = "Ngân hàng câu hỏi"; Controls = @(6401, 6402, 6403); Rows = $false },
        @{ Command = 1014; Title = "Danh sách đề thi"; Controls = @(); Rows = $true },
        @{ Command = 1015; Title = "Mở/đóng đề thi"; Controls = @(5101, 5102); Rows = $true },
        @{ Command = 1016; Title = "Kết quả tất cả học sinh"; Controls = @(6501, 6502); Rows = $true },
        @{ Command = 1120; Title = "Giám sát chống gian lận"; Controls = @(6601); Rows = $false; Delay = 4000 }
    )

    $teacherScreens = @(
        @{ Command = 1017; Title = "Ngân hàng câu hỏi"; Controls = @(6401, 6402, 6403); Rows = $false },
        @{ Command = 1018; Title = "Thêm câu hỏi"; Controls = @(4001, 4002, 4003, 4004, 4005, 4006, 4007, 4008, 4009); Rows = $false },
        @{ Command = 1019; Title = "Nhập câu hỏi"; Controls = @(8001, 8002, 8003); Rows = $false },
        @{ Command = 1020; Title = "Xóa câu hỏi"; Controls = @(4101); Rows = $false },
        @{ Command = 1021; Title = "Danh sách đề thi"; Controls = @(); Rows = $true },
        @{ Command = 1022; Title = "Giáo viên tải đề thi"; Controls = @(5001, 5002, 5003, 5004, 5005, 5006, 5007, 5008, 5009, 5010, 5011, 5012, 5013); Rows = $false },
        @{ Command = 1023; Title = "Mở/đóng đề thi"; Controls = @(5101, 5102); Rows = $true },
        @{ Command = 1024; Title = "Kết quả tất cả học sinh"; Controls = @(6501, 6502); Rows = $true },
        @{ Command = 1100; Title = "Quản lý đề thi"; Controls = @(6301, 6302, 6303, 6304, 6305, 6306, 6307, 6308, 6309, 6310, 6311, 6312, 6313); Rows = $true }
    )

    $studentScreens = @(
        @{ Command = 1025; Title = "Đề thi đang mở"; Controls = @(); Rows = $true },
        @{ Command = 1026; Title = "Làm bài thi"; Controls = @(5201, 5202); Rows = $true },
        @{ Command = 1027; Title = "Lịch sử điểm"; Controls = @(); Rows = $true }
    )

    Test-Role "Giáo viên" $TeacherEmail $TeacherPassword "Bảng điều khiển Giáo viên" $teacherScreens
    Test-Role "Admin" $AdminEmail $AdminPassword "Bảng điều khiển Admin" $adminScreens
    Test-Role "Học sinh" $StudentEmail $StudentPassword "Bảng điều khiển Học sinh" $studentScreens
}
finally {
    Stop-QuizApp
}

$summary = [pscustomobject]@{
    GeneratedAt = (Get-Date).ToString("yyyy-MM-dd HH:mm:ss")
    Executable = (Resolve-Path $Executable).Path
    Total = $script:results.Count
    Passed = @($script:results | Where-Object Passed).Count
    Failed = @($script:results | Where-Object { -not $_.Passed }).Count
    Results = $script:results
}

$jsonPath = Join-Path $resultDirectory "quiz-app-test-report.json"
$markdownPath = Join-Path $resultDirectory "quiz-app-test-report.md"
$summary | ConvertTo-Json -Depth 6 | Set-Content -Encoding UTF8 $jsonPath

$markdown = [System.Collections.Generic.List[string]]::new()
$markdown.Add("# Báo cáo kiểm thử Quiz App")
$markdown.Add("")
$markdown.Add("- Thời gian: $($summary.GeneratedAt)")
$markdown.Add("- Tổng kiểm thử: $($summary.Total)")
$markdown.Add("- Đạt: $($summary.Passed)")
$markdown.Add("- Không đạt: $($summary.Failed)")
$markdown.Add("")
$markdown.Add("| Vai trò | Chức năng | Kết quả | Chi tiết |")
$markdown.Add("|---|---|---|---|")
foreach ($item in $script:results) {
    $status = if ($item.Passed) { "ĐẠT" } else { "LỖI" }
    $details = $item.Details.Replace("|", "\|").Replace("`r", " ").Replace("`n", " ")
    $markdown.Add("| $($item.Role) | $($item.Feature) | $status | $details |")
}
$markdown | Set-Content -Encoding UTF8 $markdownPath

$summary
if ($summary.Failed -gt 0) {
    exit 1
}





