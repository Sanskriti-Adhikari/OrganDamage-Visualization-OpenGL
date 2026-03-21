# Final Graphics Project

This folder contains only the dashboard plus liver and lungs simulation projects.

## Structure

- liver: standalone liver simulation source and executable output
- lungs: standalone lungs simulation source and executable output
- dashboard: simple HTML/CSS dashboard
- start-dashboard.ps1: local launcher server for dashboard buttons

## Run Dashboard

Open PowerShell:

cd "D:\Mingw\Final Graphics project"
.\start-dashboard.ps1

Open browser:

http://localhost:8085/

## Dashboard Tech

- HTML: dashboard/index.html
- CSS: dashboard/styles.css

## Button Actions

- Launch Liver Simulation: builds and runs LiverVisualizer.exe from liver
- Launch Lungs Simulation: builds and runs LungsVisualizer.exe from lungs

## Notes

- Compiler: D:\Mingw\mingw64\bin\g++.exe
- FreeGLUT path: D:\Mingw\free-mingw-master (liver)
- GLAD path: D:\Mingw\Glad\include (lungs)
- GLFW path: D:\Mingw\Glfw (lungs)
