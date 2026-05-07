#include "ai_core.h"
#include <iostream>
#include <chrono>
#include <cmath>
#include <algorithm>

using namespace std;

const int EMPTY = 0;
const long INF = 1000000000;
const long WIN_SCORE = 10000000;

struct Candidate {
    int score;
    int r;
    int c;
};

// Đóng gói trạng thái đệ quy để tuân thủ luật "No Global Mutable State"
struct SearchContext {
    long long start_time_ms;
    long time_limit_ms;
    long nodes;
    bool timeout_flag;
    int ai_player;
};

// Kiểm tra thắng nhanh cục bộ từ một ô vừa đánh
bool check_win_from(int board[BOARD_SIZE][BOARD_SIZE], int r, int c, int player) {
    int dr[] = {1, 0, 1, 1};
    int dc[] = {0, 1, 1, -1};
    for (int i = 0; i < 4; ++i) {
        int count = 1;
        int nr = r + dr[i], nc = c + dc[i];
        while (nr >= 0 && nr < BOARD_SIZE && nc >= 0 && nc < BOARD_SIZE && board[nr][nc] == player) {
            count++; 
            nr += dr[i]; 
            nc += dc[i];
        }
        nr = r - dr[i]; 
        nc = c - dc[i];
        while (nr >= 0 && nr < BOARD_SIZE && nc >= 0 && nc < BOARD_SIZE && board[nr][nc] == player) {
            count++; 
            nr -= dr[i]; 
            nc -= dc[i];
        }
        if (count >= 5) return true;
    }
    return false;
}

int check_win(int board[BOARD_SIZE][BOARD_SIZE]) {
    for (int r = 0; r < BOARD_SIZE; ++r) {
        for (int c = 0; c < BOARD_SIZE; ++c) {
            if (board[r][c] != EMPTY) {
                if (check_win_from(board, r, c, board[r][c])) return board[r][c];
            }
        }
    }
    return 0;
}

// Tối ưu hóa kiểm tra thời gian (chỉ gọi System Time sau mỗi 1024 nodes)
bool check_timeout(SearchContext& ctx) {
    if ((ctx.nodes & 1023) != 0) {
        return false;
    }
    auto now = chrono::steady_clock::now().time_since_epoch();
    long long current_ms = chrono::duration_cast<chrono::milliseconds>(now).count();
    
    if (current_ms - ctx.start_time_ms >= ctx.time_limit_ms) {
        ctx.timeout_flag = true;
        return true;
    }
    return false;
}

int count_line(int board[BOARD_SIZE][BOARD_SIZE], int r, int c, int dr, int dc, int player, int& blocks) {
    int count = 1;
    blocks = 0;
    int nr = r + dr, nc = c + dc;
    while (nr >= 0 && nr < BOARD_SIZE && nc >= 0 && nc < BOARD_SIZE && board[nr][nc] == player) {
        count++; nr += dr; nc += dc;
    }
    if (nr < 0 || nr >= BOARD_SIZE || nc < 0 || nc >= BOARD_SIZE || board[nr][nc] != EMPTY) blocks++;
    
    nr = r - dr; nc = c - dc;
    while (nr >= 0 && nr < BOARD_SIZE && nc >= 0 && nc < BOARD_SIZE && board[nr][nc] == player) {
        count++; nr -= dr; nc -= dc;
    }
    if (nr < 0 || nr >= BOARD_SIZE || nc < 0 || nc >= BOARD_SIZE || board[nr][nc] != EMPTY) blocks++;
    return count;
}

int evaluate_move(int board[BOARD_SIZE][BOARD_SIZE], int r, int c, int player) {
    int original_val = board[r][c];
    int opponent = -player; // Quy ước 1 và -1 cho player
    
    int my_win = 0, my_open4 = 0, my_open3 = 0, my_half4 = 0;
    int op_win = 0, op_open4 = 0, op_open3 = 0, op_half4 = 0;
    
    int dr[] = {1, 0, 1, 1};
    int dc[] = {0, 1, 1, -1};
    
    for (int i = 0; i < 4; ++i) {
        int blocks = 0, op_blocks = 0;
        
        board[r][c] = player;
        int count = count_line(board, r, c, dr[i], dc[i], player, blocks);
        if (count >= 5) my_win++;
        else if (count == 4 && blocks == 0) my_open4++;
        else if (count == 4 && blocks == 1) my_half4++;
        else if (count == 3 && blocks == 0) my_open3++;
        
        board[r][c] = opponent;
        int op_count = count_line(board, r, c, dr[i], dc[i], opponent, op_blocks);
        if (op_count >= 5) op_win++;
        else if (op_count == 4 && op_blocks == 0) op_open4++;
        else if (op_count == 4 && op_blocks == 1) op_half4++;
        else if (op_count == 3 && op_blocks == 0) op_open3++;
    }
    
    board[r][c] = original_val; // Hoàn trả state

    if (my_win > 0) return 10000000;
    if (op_win > 0) return 9000000;
    if (my_open4 > 0 || (my_open3 > 0 && op_open4 == 0)) return 8000000;
    if (op_open4 > 0 || op_half4 > 0) return 7000000;
    if (my_half4 > 0) return 6000000;
    if (op_open3 > 0) return 5000000;
    if (my_open3 > 0) return 4000000;

    return 1000 - (abs(r - 7) + abs(c - 7));
}

// Cấu hình linh hoạt Candidate Limit
int get_candidate_limit(int depth, bool is_root) {
    int limit = 14;
    if (depth <= 1) limit = 40;
    else if (depth == 2) limit = 30;
    else if (depth == 3) limit = 24;
    else if (depth == 4) limit = 18;
    
    if (is_root) limit += 6; // Nới lỏng nhẹ cho nhánh gốc
    return limit;
}

// Hàm sinh nước đi (Threat-preserving)
void get_candidates(int board[BOARD_SIZE][BOARD_SIZE], Candidate candidates[], int& candidate_count, int depth, int current_player, bool is_root) {
    candidate_count = 0;
    bool has_piece = false;
    Candidate temp_candidates[225];
    int count = 0;

    for (int r = 0; r < BOARD_SIZE; ++r) {
        for (int c = 0; c < BOARD_SIZE; ++c) {
            if (board[r][c] != EMPTY) {
                has_piece = true;
                continue;
            }
            
            int min_r = max(0, r - 2);
            int max_r = min(BOARD_SIZE - 1, r + 2);
            int min_c = max(0, c - 2);
            int max_c = min(BOARD_SIZE - 1, c + 2);
            
            bool found_near = false;
            for (int i = min_r; i <= max_r; ++i) {
                for (int j = min_c; j <= max_c; ++j) {
                    if (board[i][j] != EMPTY) {
                        found_near = true;
                        break;
                    }
                }
                if (found_near) break;
            }
            
            if (found_near) {
                int score = 0;
                
                // Ưu tiên 1: Nước thắng ngay
                board[r][c] = current_player;
                if (check_win_from(board, r, c, current_player)) {
                    score = 10000000; 
                }
                board[r][c] = EMPTY;
                
                // Ưu tiên 2: Chặn đối thủ thắng ngay
                if (score == 0) {
                    int opponent = -current_player;
                    board[r][c] = opponent;
                    if (check_win_from(board, r, c, opponent)) {
                        score = 9000000; 
                    }
                    board[r][c] = EMPTY;
                }
                
                // Điểm cơ bản
                if (score == 0) {
                    score += evaluate_move(board, r, c, current_player);
                    score += evaluate_move(board, r, c, -current_player) * 0.8; 
                }
                
                if (count < 225) {
                    temp_candidates[count++] = {score, r, c};
                }
            }
        }
    }
    
    if (!has_piece) {
        candidates[0] = {0, 7, 7};
        candidate_count = 1;
        return;
    }

    std::sort(temp_candidates, temp_candidates + count, [](const Candidate& a, const Candidate& b) {
        return a.score > b.score;
    });

    int limit = get_candidate_limit(depth, is_root);
    candidate_count = std::min(count, limit);

    for(int i = 0; i < candidate_count; i++) {
        candidates[i] = temp_candidates[i];
    }
}
long evaluate_board(int board[BOARD_SIZE][BOARD_SIZE], int ai_player) {
    int winner = check_win(board);
    if (winner == ai_player) return WIN_SCORE;
    if (winner != 0) return -WIN_SCORE;
    
    long score = 0;
    int opponent = -ai_player;
    for (int r = 0; r < BOARD_SIZE; r++) {
        for (int c = 0; c < BOARD_SIZE; c++) {
            if (board[r][c] == ai_player) score += evaluate_move(board, r, c, ai_player);
            if (board[r][c] == opponent) score -= evaluate_move(board, r, c, opponent);
        }
    }
    return score;
}

long alpha_beta(int board[BOARD_SIZE][BOARD_SIZE], int depth,
     long alpha, long beta,
     bool is_maximizing, int current_player, SearchContext& ctx) {
    ctx.nodes++;
    if (check_timeout(ctx)) return 0;

    if (depth == 0) return evaluate_board(board, ctx.ai_player);

    Candidate candidates[64];
    int candidate_count = 0;
    get_candidates(board, candidates, candidate_count, depth,
         current_player, false);

    if (candidate_count == 0) return evaluate_board(board, ctx.ai_player);
    if (is_maximizing) {
        long best_score = -INF;
        for (int i = 0; i < candidate_count; i++) {
            auto& move = candidates[i];
            board[move.r][move.c] = current_player;
            
            long score;
            if (check_win_from(board, move.r, move.c, current_player)) {
                score = WIN_SCORE + depth;
            } else {
                score = alpha_beta(board, depth - 1, alpha, beta, false,
                     -current_player, ctx);
            }
            
            board[move.r][move.c] = EMPTY;
            
            if (ctx.timeout_flag) return 0;

            if (score > best_score) best_score = score;
            if (best_score > alpha) alpha = best_score;
            if (beta <= alpha) break;
        }
        return best_score;
    } else {
        long best_score = INF;
        for (int i = 0; i < candidate_count; i++) {
            auto& move = candidates[i];
            board[move.r][move.c] = current_player;
            
            long score;
            if (check_win_from(board, move.r, move.c, current_player)) {
                score = -WIN_SCORE - depth;
            } else {
                score = alpha_beta(board, depth - 1, alpha, beta, true, -current_player, ctx);
            }
            
            board[move.r][move.c] = EMPTY;
            
            if (ctx.timeout_flag) return 0;

            if (score < best_score) best_score = score;
            if (best_score < beta) beta = best_score;
            if (beta <= alpha) break;
        }
        return best_score;
    }
}

long minimax(int board[BOARD_SIZE][BOARD_SIZE], int depth,
     bool is_maximizing, int current_player, SearchContext& ctx) {
    ctx.nodes++;
    if (check_timeout(ctx)) return 0;
    if (depth == 0) return evaluate_board(board, ctx.ai_player);

    Candidate candidates[64];
    int candidate_count = 0;
    get_candidates(board, candidates, candidate_count, depth,
         current_player, false);

    if (candidate_count == 0) return evaluate_board(board, ctx.ai_player);

    if (is_maximizing) {
        long best_score = -INF;
        for (int i = 0; i < candidate_count; i++) {
            auto& move = candidates[i];
            board[move.r][move.c] = current_player;
            
            long score;
            if (check_win_from(board, move.r, move.c, current_player))
             score = WIN_SCORE + depth;
            else score = minimax(board, depth - 1, false, -current_player, ctx);
            
            board[move.r][move.c] = EMPTY;
            if (ctx.timeout_flag) return 0;
            if (score > best_score) best_score = score;
        }
        return best_score;
    } else {
        long best_score = INF;
        for (int i = 0; i < candidate_count; i++) {
            auto& move = candidates[i];
            board[move.r][move.c] = current_player;
            
            long score;
            if (check_win_from(board, move.r, move.c, current_player))
             score = -WIN_SCORE - depth;
            else score = minimax(board, depth - 1, true, -current_player, ctx);
            
            board[move.r][move.c] = EMPTY;
            if (ctx.timeout_flag) return 0;
            if (score < best_score) best_score = score;
        }
        return best_score;
    }
}

// Hàm dự phòng bắt buộc phải có
void find_best_fallback_move(int board[BOARD_SIZE][BOARD_SIZE], int player, int& fallback_r, int& fallback_c) {
    Candidate candidates[64];
    int count = 0;
    get_candidates(board, candidates, count, 1, player, true);
    
    if (count > 0 && board[candidates[0].r][candidates[0].c] == EMPTY) {
        fallback_r = candidates[0].r;
        fallback_c = candidates[0].c;
        return;
    }
    // Scan thô nếu danh sách bị rỗng bất thường
    for (int r = 0; r < BOARD_SIZE; ++r) {
        for (int c = 0; c < BOARD_SIZE; ++c) {
            if (board[r][c] == EMPTY) {
                fallback_r = r; fallback_c = c; return;
            }
        }
    }
}

// Xử lý riêng nhánh gốc (Root)
long search_root(int board[BOARD_SIZE][BOARD_SIZE], int depth, bool use_alpha_beta, SearchContext& ctx, int& best_r, int& best_c) {
    Candidate candidates[64];
    int candidate_count = 0;
    get_candidates(board, candidates, candidate_count, depth, ctx.ai_player, true);

    long best_score = -INF;
    best_r = -1;
    best_c = -1;

    for (int i = 0; i < candidate_count; i++) {
        auto& move = candidates[i];
        if (board[move.r][move.c] != EMPTY) continue;

        board[move.r][move.c] = ctx.ai_player;
        long score;
        
        if (check_win_from(board, move.r, move.c, ctx.ai_player)) {
            score = WIN_SCORE + depth;
        } else {
            if (use_alpha_beta) {
                score = alpha_beta(board, depth - 1, -INF, INF, false, -ctx.ai_player, ctx);
            } else {
                score = minimax(board, depth - 1, false, -ctx.ai_player, ctx);
            }
        }
        
        board[move.r][move.c] = EMPTY;

        if (ctx.timeout_flag) break;

        if (score > best_score) {
            best_score = score;
            best_r = move.r;
            best_c = move.c;
        }
    }
    return best_score;
}

// Wrapper chính điều phối Iterative Deepening
AIMove run_ai_search(int board[BOARD_SIZE][BOARD_SIZE], int player_turn, int max_depth, int timeout_ms, bool use_alpha_beta) {
    AIMove result;
    SearchContext ctx;
    ctx.start_time_ms = chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now().time_since_epoch()).count();
    ctx.time_limit_ms = timeout_ms - 50; // Trừ hao 50ms an toàn bridge
    ctx.nodes = 0;
    ctx.timeout_flag = false;
    ctx.ai_player = player_turn;

    int final_best_r = -1;
    int final_best_c = -1;
    long final_best_score = -INF;
    int global_depth_reached = 0;

    // Gán ngay lớp lưới dự phòng
    find_best_fallback_move(board, player_turn, final_best_r, final_best_c);

    // Iterative Deepening
    for (int depth = 1; depth <= max_depth; depth++) {
        ctx.timeout_flag = false;
        int depth_best_r = -1, depth_best_c = -1;
        
        long depth_score = search_root(board, depth, use_alpha_beta, ctx, depth_best_r, depth_best_c);

        if (ctx.timeout_flag) break; // Bỏ kết quả depth hiện tại, giữ kết quả depth trước

        if (depth_best_r != -1 && depth_best_c != -1 && board[depth_best_r][depth_best_c] == EMPTY) {
            final_best_r = depth_best_r;
            final_best_c = depth_best_c;
            final_best_score = depth_score;
            global_depth_reached = depth;
            
            if (depth_score >= WIN_SCORE - 100) break; // Đã tìm thấy nước chiếu bí, ngắt sớm
        }
    }

    // Xác minh Fallback cuối cùng
    if (final_best_r == -1 || final_best_c == -1 || board[final_best_r][final_best_c] != EMPTY) {
        find_best_fallback_move(board, player_turn, final_best_r, final_best_c);
    }

    long long end_time_ms = chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now().time_since_epoch()).count();

    result.row = final_best_r;
    result.col = final_best_c;
    result.score = final_best_score;
    result.nodes_evaluated = ctx.nodes;
    result.time_ms = (float)(end_time_ms - ctx.start_time_ms);
    result.is_timeout = ctx.timeout_flag;
    result.depth_reached = global_depth_reached;

    return result;
}