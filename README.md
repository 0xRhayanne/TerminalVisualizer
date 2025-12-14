# Terminal Visualizer: A real-time, cross-platform system dashboard rendered directly in your terminal.


<p align="center"> <img src="https://img.shields.io/badge/Language-C-blue" /> <img src="https://img.shields.io/badge/Build-CMake-lightgrey" /> <img src="https://img.shields.io/badge/IDE-CLion-green" /> <img src="https://img.shields.io/badge/Platform-Windows%20%7C%20Linux-orange" /> </p>

# 📌 Overview

Terminal Visualizer is a sleek and interactive terminal dashboard built in C that brings your system stats to life with vibrant colors and dynamic bars. Whether you're a developer, system admin, or just a tech enthusiast, it offers a fun way to monitor your system's health and performance in real time.

It simulates (and on Linux partially retrieves real) system information such as:

- CPU Usage (per-core + total);
- Memory usage;
- Load average;
- Swap usage;
- Disk usage + I/O activity;
- Network upload/download;
- GPU usage;
- Top process;
- Battery status;
- CPU temperature;
- Historical sparklines;
- And more…
---

# 🚀 Features

🌍 Cross-Platform: Runs on both Windows and Linux (with some Linux-specific capabilities for real stats);

🎨 Colorful & Interactive: Visualize your stats with vibrant colored bars, sparklines, and dynamic updates;

⚡ Real-Time Monitoring: Watch as system stats are updated live, offering a fresh dashboard every second;

🖥️ Terminal-Friendly: All data is displayed directly in your terminal, without the need for a GUI;

💪 Lightweight: Designed to be efficient, it uses minimal resources while running;

🛠️ Customizable: Adjust history size, bar length, and update interval to suit your preferences.

---

# ⚙️ Requirements

- C Compiler: ny compatible C compiler;
- CMake: Version 4.0 or higher (for building the project);
- Platform-Specific Libraries:
  - On Windows: Links against pdh.lib to fetch system performance data;
  - On Linux: Uses standard libraries for system info, such as sysinfo.h, unistd.h, etc.

---

# 📥 Installation

Getting up and running is a breeze! Here’s how:

1. Clone the repository
- Clone this repo to your local machine and navigate into the project folder:
```
git clone https://github.com/0xRhayanne/TerminalVisualizer.git
cd TerminalVisualizer
```
---
2. Build the project
- Use CMake to build the project:
