param(
    [string]$Email = "",
    [string]$Password = "",
    [int]$Command = 0,
    [Parameter(Mandatory = $true)]
    [string]$Output
)

$ErrorActionPreference = "Stop"
$projectRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
$executable = Join-Path $projectRoot "build\QuizApp.exe"

Add-Type -AssemblyName System.Windows.Forms
Add-Type -AssemblyName System.Drawing
Add-Type -AssemblyName Microsoft.VisualBasic

$interop = @'
using System;
using System.Runtime.InteropServices;

public static class UiScreenCaptureInterop
{
    [DllImport("user32.dll")]
    public static extern bool PostMessage(IntPtr hwnd, uint message, IntPtr wParam, IntPtr lParam);
    [DllImport("user32.dll")]
    public static extern bool GetWindowRect(IntPtr hwnd, out RECT rect);
    [DllImport("user32.dll")]
    public static extern bool PrintWindow(IntPtr hwnd, IntPtr hdc, uint flags);
    [StructLayout(LayoutKind.Sequential)]
    public struct RECT { public int Left, Top, Right, Bottom; }
}
'@
Add-Type -TypeDefinition $interop

$process = $null
try {
    $process = Start-Process -FilePath $executable -PassThru
    $deadline = (Get-Date).AddSeconds(25)
    do {
        Start-Sleep -Milliseconds 400
        $process.Refresh()
    } while (-not $process.HasExited -and
             $process.MainWindowHandle -eq [IntPtr]::Zero -and
             (Get-Date) -lt $deadline)

    if ($process.HasExited -or $process.MainWindowHandle -eq [IntPtr]::Zero) {
        throw "Quiz App không tạo được cửa sổ chính."
    }

    if (-not [string]::IsNullOrWhiteSpace($Email)) {
        [Microsoft.VisualBasic.Interaction]::AppActivate($process.Id) | Out-Null
        [System.Windows.Forms.SendKeys]::SendWait($Email)
        [System.Windows.Forms.SendKeys]::SendWait("{TAB}")
        [System.Windows.Forms.SendKeys]::SendWait($Password)
        [System.Windows.Forms.SendKeys]::SendWait("{ENTER}")
        Start-Sleep -Seconds 3
        $process.Refresh()
    }

    if ($Command -gt 0) {
        [UiScreenCaptureInterop]::PostMessage(
            $process.MainWindowHandle, 0x0111, [IntPtr]$Command, [IntPtr]0
        ) | Out-Null
        Start-Sleep -Seconds 2
        $process.Refresh()
    }

    $rect = New-Object UiScreenCaptureInterop+RECT
    [UiScreenCaptureInterop]::GetWindowRect($process.MainWindowHandle, [ref]$rect) | Out-Null
    $width = $rect.Right - $rect.Left
    $height = $rect.Bottom - $rect.Top
    $bitmap = New-Object System.Drawing.Bitmap $width, $height
    $graphics = [System.Drawing.Graphics]::FromImage($bitmap)
    $hdc = $graphics.GetHdc()
    [UiScreenCaptureInterop]::PrintWindow($process.MainWindowHandle, $hdc, 2) | Out-Null
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
