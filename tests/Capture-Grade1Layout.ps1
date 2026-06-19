param(
    [string]$Email = "hocsinh01.lop1@quiz.local",
    [Parameter(Mandatory = $true)]
    [string]$Password,
    [string]$ExamId = "e6",
    [string]$Output = "",
    [switch]$InterruptAfterStart
)

$ErrorActionPreference = "Stop"
$projectRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
$executable = Join-Path $projectRoot "build\QuizApp.exe"
if ([string]::IsNullOrWhiteSpace($Output)) {
    $Output = Join-Path $projectRoot "test-results\grade1-layout.png"
}

Add-Type -AssemblyName System.Windows.Forms
Add-Type -AssemblyName System.Drawing
Add-Type -AssemblyName Microsoft.VisualBasic

$interop = @'
using System;
using System.Runtime.InteropServices;
using System.Text;

public static class LayoutCaptureInterop
{
    public delegate bool EnumProc(IntPtr hwnd, IntPtr lParam);
    [DllImport("user32.dll")]
    public static extern bool EnumChildWindows(IntPtr hwnd, EnumProc callback, IntPtr lParam);
    [DllImport("user32.dll")]
    public static extern int GetDlgCtrlID(IntPtr hwnd);
    [DllImport("user32.dll")]
    public static extern bool PostMessage(IntPtr hwnd, uint message, IntPtr wParam, IntPtr lParam);
    [DllImport("user32.dll", EntryPoint = "SendMessageW")]
    public static extern IntPtr SendMessage(IntPtr hwnd, uint message, IntPtr wParam, IntPtr lParam);
    [DllImport("user32.dll")]
    public static extern bool GetWindowRect(IntPtr hwnd, out RECT rect);
    [DllImport("user32.dll")]
    public static extern bool PrintWindow(IntPtr hwnd, IntPtr hdc, uint flags);
    [StructLayout(LayoutKind.Sequential)]
    public struct RECT { public int Left, Top, Right, Bottom; }

    public static bool SetText(IntPtr parent, int id, string value)
    {
        IntPtr target = IntPtr.Zero;
        EnumChildWindows(parent, (hwnd, _) =>
        {
            if (GetDlgCtrlID(hwnd) == id) { target = hwnd; return false; }
            return true;
        }, IntPtr.Zero);
        if (target == IntPtr.Zero) return false;
        IntPtr text = Marshal.StringToHGlobalUni(value);
        try { SendMessage(target, 0x000C, IntPtr.Zero, text); }
        finally { Marshal.FreeHGlobal(text); }
        return true;
    }
}
'@
Add-Type -TypeDefinition $interop

$process = $null
try {
    $process = Start-Process -FilePath $executable -PassThru
    $deadline = (Get-Date).AddSeconds(25)
    do {
        Start-Sleep -Milliseconds 500
        $process.Refresh()
    } while (-not $process.HasExited -and
             $process.MainWindowHandle -eq [IntPtr]::Zero -and
             (Get-Date) -lt $deadline)
    if ($process.HasExited -or $process.MainWindowHandle -eq [IntPtr]::Zero) {
        throw "Quiz App không tạo được cửa sổ chính."
    }
    [Microsoft.VisualBasic.Interaction]::AppActivate($process.Id) | Out-Null
    [System.Windows.Forms.SendKeys]::SendWait($Email)
    [System.Windows.Forms.SendKeys]::SendWait("{TAB}")
    [System.Windows.Forms.SendKeys]::SendWait($Password)
    [System.Windows.Forms.SendKeys]::SendWait("{ENTER}")
    Start-Sleep -Seconds 3
    $process.Refresh()

    [LayoutCaptureInterop]::PostMessage($process.MainWindowHandle, 0x0111, [IntPtr]1026, [IntPtr]0) | Out-Null
    Start-Sleep -Seconds 1
    [LayoutCaptureInterop]::SetText($process.MainWindowHandle, 5201, $ExamId) | Out-Null
    [LayoutCaptureInterop]::PostMessage($process.MainWindowHandle, 0x0111, [IntPtr]1041, [IntPtr]0) | Out-Null
    Start-Sleep -Seconds 3
    $process.Refresh()

    if ($InterruptAfterStart) {
        [LayoutCaptureInterop]::PostMessage($process.MainWindowHandle, 0x0010, [IntPtr]0, [IntPtr]0) | Out-Null
        Start-Sleep -Seconds 3
        Write-Output "INTERRUPTED"
        return
    }

    $rect = New-Object LayoutCaptureInterop+RECT
    [LayoutCaptureInterop]::GetWindowRect($process.MainWindowHandle, [ref]$rect) | Out-Null
    $width = $rect.Right - $rect.Left
    $height = $rect.Bottom - $rect.Top
    $bitmap = New-Object System.Drawing.Bitmap $width, $height
    $graphics = [System.Drawing.Graphics]::FromImage($bitmap)
    $hdc = $graphics.GetHdc()
    [LayoutCaptureInterop]::PrintWindow($process.MainWindowHandle, $hdc, 2) | Out-Null
    $graphics.ReleaseHdc($hdc)
    $directory = Split-Path $Output -Parent
    New-Item -ItemType Directory -Force -Path $directory | Out-Null
    $bitmap.Save($Output, [System.Drawing.Imaging.ImageFormat]::Png)
    $graphics.Dispose()
    $bitmap.Dispose()
    Write-Output $Output
}
finally {
    if ($process -and -not $process.HasExited) {
        Stop-Process -Id $process.Id -Force -ErrorAction SilentlyContinue
    }
}
