param(
    [int]$Port = 8085
)

$ErrorActionPreference = "Stop"

$workspaceRoot = $PSScriptRoot
$dashboardRoot = Join-Path $workspaceRoot "dashboard"
$liverRoot = Join-Path $workspaceRoot "liver"
$lungsRoot = Join-Path $workspaceRoot "lungs"

$gpp = "D:\Mingw\mingw64\bin\g++.exe"
$freeglutInclude = "D:\Mingw\free-mingw-master\include"
$freeglutLib = "D:\Mingw\free-mingw-master\lib\x64"
$freeglutDll = "D:\Mingw\free-mingw-master\bin\x64\freeglut.dll"
$gladInclude = "D:\Mingw\Glad\include"
$glfwInclude = "D:\Mingw\Glfw\include"
$glfwLib = "D:\Mingw\Glfw\lib-mingw-w64"
$glfwDll = "D:\Mingw\Glfw\lib-mingw-w64\glfw3.dll"

if (-not (Test-Path $dashboardRoot)) { throw "Dashboard directory not found: $dashboardRoot" }
if (-not (Test-Path $liverRoot)) { throw "Liver project directory not found: $liverRoot" }
if (-not (Test-Path $lungsRoot)) { throw "Lungs project directory not found: $lungsRoot" }
if (-not (Test-Path $gpp)) { throw "Compiler not found: $gpp" }

function Invoke-Build {
    param(
        [string]$WorkingDirectory,
        [string[]]$Arguments
    )

    Push-Location $WorkingDirectory
    try {
        & $gpp @Arguments
        if ($LASTEXITCODE -ne 0) {
            throw "Build failed in $WorkingDirectory"
        }
    }
    finally {
        Pop-Location
    }
}

function Ensure-Dll {
    param([string]$TargetDirectory)

    if (Test-Path $freeglutDll) {
        Copy-Item $freeglutDll (Join-Path $TargetDirectory "freeglut.dll") -Force
    }
}

function Ensure-LungsDll {
    param([string]$TargetDirectory)

    if (Test-Path $glfwDll) {
        Copy-Item $glfwDll (Join-Path $TargetDirectory "glfw3.dll") -Force
    }
}

function Start-Liver {
    $args = @(
        "-std=c++11", "-O2", "-Wall",
        "liver_main.cpp", "liver.cpp", "animation.cpp", "ui.cpp", "side_bar.cpp", "bottom_bar.cpp", "utils.cpp",
        "-I$freeglutInclude",
        "-L$freeglutLib",
        "-o", "LiverVisualizer.exe",
        "-lfreeglut", "-lopengl32", "-lglu32", "-lmingw32"
    )

    Invoke-Build -WorkingDirectory $liverRoot -Arguments $args
    Ensure-Dll -TargetDirectory $liverRoot

    Start-Process -FilePath (Join-Path $liverRoot "LiverVisualizer.exe") -WorkingDirectory $liverRoot | Out-Null
    return "Liver simulation launched."
}

function Start-Lungs {
    $args = @(
        "-std=c++11", "-O2", "-Wall",
        "lungs_main.cpp", "lungs.cpp", "particles.cpp", "renderer.cpp", "geometry.cpp", "lighting.cpp", "glad.c",
        "-I$gladInclude",
        "-I$glfwInclude",
        "-L$glfwLib",
        "-o", "LungsVisualizer.exe",
        "-lglfw3", "-lopengl32", "-lgdi32", "-luser32", "-lkernel32"
    )

    Invoke-Build -WorkingDirectory $lungsRoot -Arguments $args
    Ensure-LungsDll -TargetDirectory $lungsRoot

    Start-Process -FilePath (Join-Path $lungsRoot "LungsVisualizer.exe") -WorkingDirectory $lungsRoot | Out-Null
    return "Lungs simulation launched."
}

function Write-JsonResponse {
    param(
        [System.Net.HttpListenerResponse]$Response,
        [int]$StatusCode,
        [hashtable]$Payload
    )

    $json = ($Payload | ConvertTo-Json -Compress)
    $bytes = [System.Text.Encoding]::UTF8.GetBytes($json)
    $Response.StatusCode = $StatusCode
    $Response.ContentType = "application/json; charset=utf-8"
    $Response.ContentLength64 = $bytes.Length
    $Response.OutputStream.Write($bytes, 0, $bytes.Length)
    $Response.OutputStream.Close()
}

function Write-FileResponse {
    param(
        [System.Net.HttpListenerResponse]$Response,
        [string]$FilePath
    )

    if (-not (Test-Path $FilePath)) {
        $Response.StatusCode = 404
        $Response.OutputStream.Close()
        return
    }

    $extension = [IO.Path]::GetExtension($FilePath).ToLowerInvariant()
    $mimeTypes = @{
        ".html" = "text/html; charset=utf-8"
        ".css"  = "text/css; charset=utf-8"
        ".js"   = "application/javascript; charset=utf-8"
        ".json" = "application/json; charset=utf-8"
    }

    $mimeType = if ($mimeTypes.ContainsKey($extension)) { $mimeTypes[$extension] } else { "application/octet-stream" }
    $bytes = [IO.File]::ReadAllBytes($FilePath)

    $Response.StatusCode = 200
    $Response.ContentType = $mimeType
    $Response.ContentLength64 = $bytes.Length
    $Response.OutputStream.Write($bytes, 0, $bytes.Length)
    $Response.OutputStream.Close()
}

$listener = New-Object System.Net.HttpListener
$prefixes = @(
    "http://localhost:$Port/",
    "http://127.0.0.1:$Port/"
)

foreach ($p in $prefixes) {
    $listener.Prefixes.Add($p)
}

try {
    $listener.Start()
}
catch {
    Write-Host "Failed to start dashboard listener on port $Port."
    Write-Host "Try a different port, for example: .\\start-dashboard.ps1 -Port 8090"
    throw
}

Write-Host "Dashboard running at:"
foreach ($p in $prefixes) {
    Write-Host "  $p"
}
Write-Host "Press Ctrl+C to stop."

try {
    while ($listener.IsListening) {
        $context = $listener.GetContext()
        $request = $context.Request
        $response = $context.Response
        $path = $request.Url.AbsolutePath.Trim("/")

        try {
            if ($request.HttpMethod -eq "POST" -and $path -eq "launch/liver") {
                $message = Start-Liver
                Write-JsonResponse -Response $response -StatusCode 200 -Payload @{ ok = $true; message = $message }
                continue
            }

            if ($request.HttpMethod -eq "POST" -and $path -eq "launch/lungs") {
                $message = Start-Lungs
                Write-JsonResponse -Response $response -StatusCode 200 -Payload @{ ok = $true; message = $message }
                continue
            }

            if ([string]::IsNullOrWhiteSpace($path)) {
                $path = "index.html"
            }

            $localPath = [IO.Path]::GetFullPath((Join-Path $dashboardRoot $path))
            $rootPath = [IO.Path]::GetFullPath($dashboardRoot + [IO.Path]::DirectorySeparatorChar)
            if (-not $localPath.StartsWith($rootPath, [System.StringComparison]::OrdinalIgnoreCase)) {
                Write-JsonResponse -Response $response -StatusCode 403 -Payload @{ ok = $false; message = "Forbidden" }
                continue
            }

            Write-FileResponse -Response $response -FilePath $localPath
        }
        catch {
            Write-JsonResponse -Response $response -StatusCode 500 -Payload @{ ok = $false; message = $_.Exception.Message }
        }
    }
}
finally {
    $listener.Stop()
    $listener.Close()
}


#Compile: .\start-dashboard.ps1