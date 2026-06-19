param(
    [string]$TeacherEmail = "teacher@quiz.local",
    [Parameter(Mandatory = $true)]
    [string]$TeacherPassword,
    [string]$StudentPassword = "LopMot@2026",
    [string]$Executable = ""
)

$ErrorActionPreference = "Stop"
$projectRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
$csvPath = Join-Path $projectRoot "toan_lop_1_25_cau.csv"
$firebaseHelper = Join-Path $projectRoot "firebase-seed\grade1-e2e.js"

if ([string]::IsNullOrWhiteSpace($Executable)) {
    $Executable = Join-Path $projectRoot "build\QuizApp.exe"
}

Add-Type -AssemblyName System.Windows.Forms
Add-Type -AssemblyName Microsoft.VisualBasic

$interopSource = @'
using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;

public static class Grade1TestInterop
{
    public delegate bool EnumProc(IntPtr hwnd, IntPtr lParam);

    [DllImport("user32.dll")]
    public static extern bool EnumChildWindows(IntPtr hwnd, EnumProc callback, IntPtr lParam);

    [DllImport("user32.dll", CharSet = CharSet.Unicode)]
    public static extern int GetWindowText(IntPtr hwnd, StringBuilder text, int max);

    [DllImport("user32.dll")]
    public static extern int GetDlgCtrlID(IntPtr hwnd);

    [DllImport("user32.dll")]
    public static extern bool PostMessage(IntPtr hwnd, uint message, IntPtr wParam, IntPtr lParam);

    [DllImport("user32.dll", EntryPoint = "SendMessageW")]
    public static extern IntPtr SendMessage(IntPtr hwnd, uint message, IntPtr wParam, IntPtr lParam);

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

    public static bool HasControl(IntPtr parent, int id)
    {
        return FindControl(parent, id) != IntPtr.Zero;
    }

    public static bool ClickControl(IntPtr parent, int id)
    {
        IntPtr control = FindControl(parent, id);
        if (control == IntPtr.Zero)
        {
            return false;
        }
        SendMessage(control, 0x00F5, IntPtr.Zero, IntPtr.Zero);
        return true;
    }

    public static string GetControlText(IntPtr parent, int id)
    {
        IntPtr control = FindControl(parent, id);
        if (control == IntPtr.Zero)
        {
            return "";
        }
        var text = new StringBuilder(4096);
        GetWindowText(control, text, text.Capacity);
        return text.ToString();
    }

    public static bool HasVisibleText(IntPtr parent, string expected)
    {
        bool found = false;
        EnumChildWindows(parent, (hwnd, _) =>
        {
            var text = new StringBuilder(1024);
            GetWindowText(hwnd, text, text.Capacity);
            if (text.ToString().Equals(expected, StringComparison.Ordinal))
            {
                found = true;
                return false;
            }
            return true;
        }, IntPtr.Zero);
        return found;
    }
}
'@

Add-Type -TypeDefinition $interopSource

$script:process = $null

function Stop-App {
    if ($script:process -and -not $script:process.HasExited) {
        Stop-Process -Id $script:process.Id -Force -ErrorAction SilentlyContinue
    }
    $script:process = $null
}

function Refresh-App {
    $script:process.Refresh()
    if ($script:process.HasExited) {
        throw "Quiz App đã dừng ngoài dự kiến."
    }
}

function Invoke-Command {
    param([int]$Id, [int]$Delay = 700)
    Refresh-App
    [Grade1TestInterop]::PostMessage(
        $script:process.MainWindowHandle,
        0x0111,
        [IntPtr]$Id,
        [IntPtr]0
    ) | Out-Null
    Start-Sleep -Milliseconds $Delay
}

function Set-Field {
    param([int]$Id, [string]$Value)
    Refresh-App
    if (-not [Grade1TestInterop]::SetControlText(
        $script:process.MainWindowHandle,
        $Id,
        $Value
    )) {
        throw "Không tìm thấy ô nhập có ID $Id."
    }
}

function Click-Control {
    param([int]$Id, [int]$Delay = 700)
    Refresh-App
    if (-not [Grade1TestInterop]::ClickControl(
        $script:process.MainWindowHandle,
        $Id
    )) {
        throw "Không tìm thấy nút có ID $Id."
    }
    Start-Sleep -Milliseconds $Delay
}

function Wait-Text {
    param([string]$Text, [int]$Seconds = 15)
    $deadline = (Get-Date).AddSeconds($Seconds)
    do {
        Refresh-App
        if ([Grade1TestInterop]::HasVisibleText(
            $script:process.MainWindowHandle,
            $Text
        )) {
            return
        }
        Start-Sleep -Milliseconds 300
    } while ((Get-Date) -lt $deadline)
    throw "Không thấy nội dung '$Text' trong thời gian chờ."
}

function Close-MessageBox {
    param([int]$WaitSeconds = 3)
    Start-Sleep -Seconds $WaitSeconds
    [Microsoft.VisualBasic.Interaction]::AppActivate($script:process.Id) | Out-Null
    Start-Sleep -Milliseconds 300
    [System.Windows.Forms.SendKeys]::SendWait("{ENTER}")
    Start-Sleep -Milliseconds 900
}

function Start-LoggedInApp {
    param(
        [string]$Email,
        [string]$Password,
        [string]$DashboardTitle
    )

    Stop-App
    $script:process = Start-Process -FilePath $Executable -PassThru
    Start-Sleep -Seconds 8
    Refresh-App
    [Microsoft.VisualBasic.Interaction]::AppActivate($script:process.Id) | Out-Null
    Start-Sleep -Milliseconds 400
    [System.Windows.Forms.SendKeys]::SendWait($Email)
    [System.Windows.Forms.SendKeys]::SendWait("{TAB}")
    [System.Windows.Forms.SendKeys]::SendWait($Password)
    [System.Windows.Forms.SendKeys]::SendWait("{ENTER}")
    Wait-Text $DashboardTitle 12
}

function Invoke-FirebaseHelper {
    param([string]$Action)
    $output = & node $firebaseHelper $Action
    if ($LASTEXITCODE -ne 0) {
        throw "Firebase helper thất bại ở bước $Action."
    }
    return ($output | Select-Object -Last 1 | ConvertFrom-Json)
}

try {
    $csvText = Get-Content $csvPath -Raw -Encoding UTF8
    $csvRows = @(Import-Csv $csvPath -Encoding UTF8)
    if ($csvRows.Count -ne 25) {
        throw "CSV phải có đúng 25 câu, hiện có $($csvRows.Count)."
    }

    Invoke-FirebaseHelper "reset-content" | Out-Null

    Start-LoggedInApp $TeacherEmail $TeacherPassword "Bảng điều khiển Giáo viên"
    Invoke-Command 1019
    Wait-Text "Nhập câu hỏi"
    Set-Field 8003 "Toán lớp 1 - Bộ 25 câu"
    Set-Field 8002 $csvText
    Invoke-Command 1031 30000
    Close-MessageBox 1
    Wait-Text "Bảng điều khiển Giáo viên" 10

    $questionInfo = Invoke-FirebaseHelper "get-question-ids"
    if ($questionInfo.count -ne 25) {
        throw "Firestore chỉ ghi nhận $($questionInfo.count)/25 câu."
    }

    Invoke-Command 1022
    Wait-Text "Giáo viên tải đề thi"
    Set-Field 5001 "Kiểm tra Toán lớp 1 - 25 câu"
    Set-Field 5002 "30"
    Set-Field 5010 "1"
    Set-Field 5006 (Get-Date).AddMinutes(-5).ToString("yyyy-MM-dd HH:mm")
    Set-Field 5004 (Get-Date).AddDays(30).ToString("yyyy-MM-dd HH:mm")
    Set-Field 5003 ($questionInfo.ids -join " ")
    Set-Field 5005 ""
    Set-Field 5007 ""
    Set-Field 5008 ""
    Set-Field 5009 ""
    Invoke-Command 1037 12000
    Close-MessageBox 1
    Wait-Text "Bảng điều khiển Giáo viên" 10

    $examInfo = Invoke-FirebaseHelper "get-exam"
    if ($examInfo.count -ne 1) {
        throw "Không xác định được duy nhất một đề thi vừa tạo."
    }
    $examId = $examInfo.exams[0].id

    Invoke-Command 1100
    Wait-Text "Quản lý đề thi"
    Set-Field 6301 $examId
    Click-Control 1101 1800
    Click-Control 1102 10000
    Close-MessageBox 1
    $updatedExamInfo = Invoke-FirebaseHelper "get-exam"
    if ($updatedExamInfo.count -ne 1 -or
        $updatedExamInfo.exams[0].durationMinutes -ne 30 -or
        $updatedExamInfo.exams[0].questionIds.Count -ne 25) {
        throw "Dữ liệu đề sau khi lưu thay đổi không đúng trên Firestore."
    }

    $disposable = Invoke-FirebaseHelper "create-disposable-exam"
    Stop-App
    Start-LoggedInApp $TeacherEmail $TeacherPassword "Bảng điều khiển Giáo viên"
    Invoke-Command 1100
    Wait-Text "Quản lý đề thi"
    Set-Field 6301 $disposable.id
    Click-Control 1101
    Click-Control 1103 600
    [Microsoft.VisualBasic.Interaction]::AppActivate($script:process.Id) | Out-Null
    [System.Windows.Forms.SendKeys]::SendWait("{LEFT}{ENTER}")
    Close-MessageBox 2
    $disposableCheck = Invoke-FirebaseHelper "check-disposable-exam"
    if ($disposableCheck.exists) {
        Invoke-FirebaseHelper "delete-disposable-exam" | Out-Null
        Write-Warning "Hộp thoại xác nhận xóa không nhận phím tự động; đã dọn đề kiểm thử bằng Firebase helper."
    }
    Stop-App

    $answerIndexes = @{
        A = 0
        B = 1
        C = 2
        D = 3
    }

    for ($studentNumber = 1; $studentNumber -le 10; $studentNumber++) {
        $number = $studentNumber.ToString("00")
        $email = "hocsinh$number.lop1@quiz.local"
        Start-LoggedInApp $email $StudentPassword "Bảng điều khiển Học sinh"
        Invoke-Command 1026
        Wait-Text "Làm bài thi"
        Set-Field 5201 $examId
        Set-Field 5202 ""
        Invoke-Command 1041 2500
        Wait-Text "Nội dung câu hỏi" 10

        for ($index = 0; $index -lt $csvRows.Count; $index++) {
            $answer = $csvRows[$index].correctAnswer.Trim().ToUpperInvariant()
            $commandId = 5300 + ($index * 10) + $answerIndexes[$answer]
            if (-not [Grade1TestInterop]::HasControl(
                $script:process.MainWindowHandle,
                $commandId
            )) {
                throw "Thiếu nút đáp án câu $($index + 1), ID $commandId."
            }
            Invoke-Command $commandId 20
        }

        Invoke-Command 1041 4000
        Close-MessageBox 1
        Wait-Text "Bảng điều khiển Học sinh" 10
        Stop-App
        Write-Output "Đã hoàn thành: $email"
    }

    $verification = Invoke-FirebaseHelper "verify"
    $report = [pscustomobject]@{
        CsvQuestions = $csvRows.Count
        FirestoreQuestions = $questionInfo.count
        Students = $verification.students
        ExamId = $examId
        Exams = $verification.exams
        Results = $verification.results
        PerfectScores = @(
            $verification.scores |
            Where-Object {
                $_.score -eq 25 -and $_.totalQuestions -eq 25
            }
        ).Count
    }
    $report | ConvertTo-Json -Depth 5
}
finally {
    Stop-App
}
