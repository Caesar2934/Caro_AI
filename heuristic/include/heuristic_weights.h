/**
 * Heuristic Weight Table — AI Caro Engine (15x15)
 * Branch: heuristic-qa | Owner: Thành viên 3 (Strategy Analyst)
 *
 * Quy ước board: 1 = quân đi trước, -1 = quân đi sau, 0 = trống.
 * Pattern áp dụng cho đánh giá thế cờ (tấn công / phòng thủ đối xứng).
 *
 * Thứ bậc bắt buộc (tài liệu dự án):
 *   Open Four > Half-Open Four > Open Three > Half-Open Three > Open Two
 *
 * Nguyên tắc phòng thủ: chặn nguy cơ thua ngay (đối thủ có 4 liên tiếp mở một
 * đầu hoặc tạo năm) phải được ưu tiên hơn tấn công “trung bình” (ví dụ tạo Open
 * Three). Các hằng BLOCK_* dùng khi phát hiện mối đe dọa tương ứng của đối thủ
 * — TV1 cộng/trừ đối xứng trong hàm evaluate() để không hardcode phe.
 */
#ifndef AI_CARO_HEURISTIC_WEIGHTS_H
#define AI_CARO_HEURISTIC_WEIGHTS_H

#include <cstdint>

namespace caro::heuristic {

// --- Tấn công / cấu trúc tự thân (điểm dương theo hướng “ưu thế bàn cờ”) ---
inline constexpr std::int64_t SCORE_OPEN_FOUR = 100000LL;
inline constexpr std::int64_t SCORE_HALF_OPEN_FOUR = 45000LL;
inline constexpr std::int64_t SCORE_OPEN_THREE = 8000LL;
inline constexpr std::int64_t SCORE_HALF_OPEN_THREE = 900LL;
inline constexpr std::int64_t SCORE_OPEN_TWO = 120LL;

// --- Phòng thủ: đe dọa trực tiếp từ đối thủ (phải >= tấn công trung bình) ---
// Chặn “bốn mở một đầu” của địch: quan trọng hơn tạo Open Three cho mình
// (8000).
inline constexpr std::int64_t BLOCK_HALF_OPEN_FOUR = 52000LL;
inline constexpr std::int64_t BLOCK_OPEN_FOUR_OR_WIN = 2000000LL;
// Ba mở / nửa mở đối thủ — vẫn xếp dưới chặn tứ nhưng trên Open Two tấn công.
inline constexpr std::int64_t BLOCK_OPEN_THREE = 7500LL;
inline constexpr std::int64_t BLOCK_HALF_OPEN_THREE = 1100LL;

// Kiểm tra thứ bậc tại compile-time (TV1 có thể static_assert trong module
// evaluate).
static_assert(SCORE_OPEN_FOUR > SCORE_HALF_OPEN_FOUR, "ordering");
static_assert(SCORE_HALF_OPEN_FOUR > SCORE_OPEN_THREE, "ordering");
static_assert(SCORE_OPEN_THREE > SCORE_HALF_OPEN_THREE, "ordering");
static_assert(SCORE_HALF_OPEN_THREE > SCORE_OPEN_TWO, "ordering");

static_assert(BLOCK_OPEN_FOUR_OR_WIN >= SCORE_OPEN_FOUR, "defense immediate");
static_assert(BLOCK_HALF_OPEN_FOUR > SCORE_OPEN_THREE, "defense vs avg attack");

} // namespace caro::heuristic

#endif
