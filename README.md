# 🚀 AI Caro Engine (15x15)

> Hệ thống trí tuệ nhân tạo chơi cờ Caro đỉnh cao, sử dụng thuật toán Minimax và Alpha-Beta Pruning biên dịch qua WebAssembly.

[![C++](https://img.shields.io/badge/Language-C%2B%2B-00599C?style=flat-square&logo=c%2B%2B)](https://isocpp.org/)
[![WASM](https://img.shields.io/badge/Powered%20by-WebAssembly-654FF0?style=flat-square&logo=webassembly)](https://webassembly.org/)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg?style=flat-square)](https://opensource.org/licenses/MIT)
[![Status](https://img.shields.io/badge/Status-Development-yellow?style=flat-square)]()

---

## 🧐 Mục lục
1. [Giới thiệu](#-giới-thiệu)
2. [Tính năng nổi bật](#-tính-năng-nổi-bật)
3. [Luật chơi (Core Rules)](#-luật-chơi-core-rules)
4. [Kiến trúc AI & Tối ưu hóa](#-kiến-trúc-ai--tối-ưu-hóa)
5. [Tech Stack](#-tech-stack)
6. [Phân công công việc (WBS)](#-phân-công-công-việc-wbs)
7. [Cài đặt & Triển khai](#-cài-đặt--triển-khai)
8. [Giao thức tích hợp](#-giao-thức-tích-hợp)

---

## 📖 Giới thiệu
Dự án tập trung vào việc xây dựng một **Core AI** mạnh mẽ cho trò chơi Caro 15x15. Trọng tâm của dự án là chứng minh khả năng tối ưu hóa thuật toán tìm kiếm (Minimax + Alpha-Beta), chất lượng hàm lượng giá Heuristic và khả năng thực thi hiệu năng cao trên trình duyệt thông qua công nghệ WebAssembly.

## ✨ Tính năng nổi bật
* **AI Thông minh:** Thuật toán Minimax kết hợp cắt tỉa Alpha-Beta mạnh mẽ.
* **Tối ưu hóa không gian:** *Candidate Move Generation* giới hạn vùng tìm kiếm trong bán kính Chebyshev <= 2.
* **Đa luồng (Multi-threading):** Chạy AI ngầm trên **Web Worker** để đảm bảo giao diện không bị treo (Non-blocking UI).
* **Benchmark Dashboard:** Hiển thị thời gian thực số lượng Node đã duyệt, thời gian xử lý và Heuristic score.
* **Local Win Check:** Kiểm tra thắng/hòa cục bộ từ nước đi mới nhất để đạt tốc độ xử lý tối ưu.

## ⚖️ Luật chơi (Core Rules)
* **Kích thước:** Bàn cờ cố định 15x15.
* **Điều kiện thắng:** Có từ 5 quân liên tiếp (ngang, dọc, chéo). Chấp nhận Overline, không xét chặn 2 đầu.
* **Điều kiện hòa:** Bàn cờ đầy (225 ô) mà không có bên thắng.
* **Đơn giản hóa:** Không áp dụng luật Renju hay luật Caro Việt Nam truyền thống.

## 🧠 Kiến trúc AI & Tối ưu hóa
* **Iterative Deepening:** Tìm kiếm theo độ sâu tăng dần. Nếu hết thời gian, AI trả về kết quả tốt nhất của Depth hoàn chỉnh gần nhất.
* **Timeout Control:** Giới hạn mỗi lượt suy nghĩ tối đa **2000ms**.
* **Alpha-Beta Pruning:** Mục tiêu cắt tỉa >= 60% số lượng Node so với Minimax thuần túy.

## 🛠 Tech Stack
* **Core AI:** C++ (Standard 17)
* **Bridge:** Emscripten (WebAssembly)
* **Frontend:** HTML5, CSS3, JavaScript (ES6)
* **Threading:** Web Workers API

## 👥 Phân công công việc (WBS)

| Thành viên | Vai trò | Trọng tâm công việc | KPI Hoàn thành |
| :--- | :--- | :--- | :--- |
| **Thành viên 1** | AI Architect | Minimax, Alpha-Beta, Iterative Deepening | Cắt tỉa >= 60% node; phản hồi < 2s |
| **Thành viên 2** | System Integration | Game Engine, Luật chơi, Tích hợp WASM | Pass 100% test luật; WASM ổn định |
| **Thành viên 3** | Strategy Analyst | Heuristic Weight Table, 15 Scenario Tests | AI pass >= 90% scenario tests |
| **Thành viên 4** | Data Engineer | Logger, Benchmark, Thu thập số liệu | Đủ số liệu Benchmark Depth 3, 4, 5 |
| **Thành viên 5** | UI/UX Developer | Vẽ Board, Dashboard Metrics, Control Panel | UI mượt mà; Reset xóa sạch metrics |

## 🚀 Cài đặt & Triển khai
 
1. **Clone dự án:**
   ```bash
   git clone [https://github.com/your-username/ai-caro-engine.git](https://github.com/your-username/ai-caro-engine.git)
   7. Cài đặt & Triển khai
   Biên dịch WASM (Yêu cầu Emscripten):
   emcc ai_core.cpp -s WASM=1 -o ai_core.js -O3
   Chạy Local Server:   
   python -m http.server 8000
   8. Giao thức tích hợp
   struct AIMove {
   int row;
   int col;
   long score;
   long nodes_evaluated;
   float time_ms;
   bool is_timeout;
   };
   AIMove get_best_move(int board[15][15], int player_turn, int max_depth, int timeout_ms, bool use_alpha_beta);

