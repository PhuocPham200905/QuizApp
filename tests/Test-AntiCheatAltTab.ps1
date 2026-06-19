param(
    [string]$Executable = "",
    [string]$StudentEmail = "an@quiz.local",
    [string]$StudentPassword = "123456",
    [string]$ExamId = "e1",
    [string]$ExamPassword = "oop123",
    [int]$Cycles = 2
)

$ErrorActionPreference = "Stop"
$projectRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
if ([string]::IsNullOrWhiteSpace($Executable)) {
    $Executable = Join-Path $projectRoot "build\QuizApp.exe"
} elseif (-not [System.IO.Path]::IsPathRooted($Executable)) {
    $Executable = Join-Path $projectRoot $Executable
}

$env:QUIZAPP_SKIP_FIREBASE = "1"
Add-Type -AssemblyName Microsoft.VisualBasic

$interopSource = @'
using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;

public static class AntiCheatTestInterop
{
    public delegate bool EnumProc(IntPtr hwnd, IntPtr lParam);

    [DllImport("user32.dll")]
    public static extern bool EnumWindows(EnumProc callback, IntPtr lParam);

    [DllImport("user32.dll")]
    public static extern bool EnumChildWindows(IntPtr hwnd, EnumProc callback, IntPtr lParam);

    [DllImport("user32.dll", CharSet = CharSet.Unicode)]
    public static extern int GetWindowText(IntPtr hwnd, StringBuilder text, int max);

    [DllImport("user32.dll")]
    public static extern uint GetWindowThreadProcessId(IntPtr hwnd, out uint processId);

    [DllImport("user32.dll")]
    public static extern int GetDlgCtrlID(IntPtr hwnd);

    [DllImport("user32.dll", EntryPoint = "SendMessageW")]
    public static extern IntPtr SendMessage(IntPtr hwnd, uint message, IntPtr wParam, IntPtr lParam);

    [DllImport("user32.dll")]
    public static extern bool PostMessage(IntPtr hwnd, uint message, IntPtr wParam, IntPtr lParam);

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
        if (control == IntPtr.Zero) return false;
        IntPtr text = Marshal.StringToHGlobalUni(value);
        try
        {
            SendMessage(control, 0x000C, IntPtr.Zero, text);
            return true;
        }
        finally
        {
            Marshal.FreeHGlobal(text);
        }
    }

    public static IntPtr FindDialog(uint processId, string title)
    {
        IntPtr result = IntPtr.Zero;
        EnumWindows((hwnd, _) =>
        {
            uint owner;
            GetWindowThreadProcessId(hwnd, out owner);
            if (owner != processId) return true;
            var text = new StringBuilder(256);
            GetWindowText(hwnd, text, text.Capacity);
            if (text.ToString().Equals(title, StringComparison.Ordinal))
            {
                result = hwnd;
                return false;
            }
            return true;
        }, IntPtr.Zero);
        return result;
    }

    public static string ChildText(IntPtr parent)
    {
        var values = new List<string>();
        EnumChildWindows(parent, (hwnd, _) =>
        {
            var text = new StringBuilder(1024);
            GetWindowText(hwnd, text, text.Capacity);
            if (text.Length > 0) values.Add(text.ToString());
            return true;
        }, IntPtr.Zero);
        return String.Join(" | ", values.ToArray());
    }
}
'@

Add-Type -TypeDefinition $interopSource

function Wait-MainWindow {
    param($Process, [int]$Seconds = 20)
    Start-Sleep -Seconds 2
    $deadline = (Get-Date).AddSeconds($Seconds)
    do {
        Start-Sleep -Milliseconds 250
        $Process.Refresh()
        if ($Process.HasExited) {
            throw "Quiz App đã thoát ngoài dự kiến (exit code $($Process.ExitCode), executable '$Executable')."
        }
    } while ($Process.MainWindowHandle -eq [IntPtr]::Zero -and (Get-Date) -lt $deadline)
    if ($Process.MainWindowHandle -eq [IntPtr]::Zero) {
        throw "Không tìm thấy cửa sổ Quiz App."
    }
}

function Wait-Dialog {
    param($Process, [int]$Seconds = 5)
    $deadline = (Get-Date).AddSeconds($Seconds)
    do {
        $dialog = [AntiCheatTestInterop]::FindDialog(
            [uint32]$Process.Id,
            "Cảnh báo chống gian lận"
        )
        if ($dialog -ne [IntPtr]::Zero) {
            return $dialog
        }
        Start-Sleep -Milliseconds 100
    } while ((Get-Date) -lt $deadline)
    return [IntPtr]::Zero
}

$quiz = $null
$other = $null
try {
    $quiz = Start-Process -FilePath $Executable -PassThru
    Wait-MainWindow $quiz

    [AntiCheatTestInterop]::SetControlText($quiz.MainWindowHandle, 2001, $StudentEmail) | Out-Null
    [AntiCheatTestInterop]::SetControlText($quiz.MainWindowHandle, 2002, $StudentPassword) | Out-Null
    [AntiCheatTestInterop]::PostMessage($quiz.MainWindowHandle, 0x0111, [IntPtr]1004, [IntPtr]0) | Out-Null
    Start-Sleep -Seconds 1

    [AntiCheatTestInterop]::PostMessage($quiz.MainWindowHandle, 0x0111, [IntPtr]1026, [IntPtr]0) | Out-Null
    Start-Sleep -Milliseconds 700
    [AntiCheatTestInterop]::SetControlText($quiz.MainWindowHandle, 5201, $ExamId) | Out-Null
    [AntiCheatTestInterop]::SetControlText($quiz.MainWindowHandle, 5202, $ExamPassword) | Out-Null
    [AntiCheatTestInterop]::PostMessage($quiz.MainWindowHandle, 0x0111, [IntPtr]1041, [IntPtr]0) | Out-Null
    Start-Sleep -Seconds 1

    $other = Start-Process notepad.exe -PassThru
    Wait-MainWindow $other
    $results = @()

    for ($cycle = 1; $cycle -le $Cycles; $cycle++) {
        [Microsoft.VisualBasic.Interaction]::AppActivate($quiz.Id) | Out-Null
        Start-Sleep -Milliseconds 400
        [Microsoft.VisualBasic.Interaction]::AppActivate($other.Id) | Out-Null
        Start-Sleep -Milliseconds 500
        [Microsoft.VisualBasic.Interaction]::AppActivate($quiz.Id) | Out-Null

        $dialog = Wait-Dialog $quiz 5
        if ($dialog -eq [IntPtr]::Zero) {
            $results += [pscustomobject]@{
                Cycle = $cycle
                WarningShown = $false
                Text = ""
            }
            continue
        }

        $text = [AntiCheatTestInterop]::ChildText($dialog)
        $results += [pscustomobject]@{
            Cycle = $cycle
            WarningShown = $true
            Text = $text
        }
        [AntiCheatTestInterop]::PostMessage($dialog, 0x0010, [IntPtr]0, [IntPtr]0) | Out-Null
        Start-Sleep -Milliseconds 500
    }

    $results
    if (@($results | Where-Object { -not $_.WarningShown }).Count -gt 0) {
        exit 1
    }
}
finally {
    if ($other -and -not $other.HasExited) {
        Stop-Process -Id $other.Id -Force -ErrorAction SilentlyContinue
    }
    if ($quiz -and -not $quiz.HasExited) {
        Stop-Process -Id $quiz.Id -Force -ErrorAction SilentlyContinue
    }
}
