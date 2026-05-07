const BOARD_SIZE = 15;
const EMPTY = 0;

// Heuristic Weights
const SCORE_OPEN_FOUR = 100000;
const SCORE_HALF_OPEN_FOUR = 45000;
const SCORE_OPEN_THREE = 8000;
const SCORE_HALF_OPEN_THREE = 900;
const SCORE_OPEN_TWO = 120;

const BLOCK_HALF_OPEN_FOUR = 52000;
const BLOCK_OPEN_FOUR_OR_WIN = 2000000;
const BLOCK_OPEN_THREE = 7500;
const BLOCK_HALF_OPEN_THREE = 1100;

function evaluate_chain(count, open_ends, is_ai) {
    if (count >= 5) return is_ai ? 10000000 : -10000000;
    if (count === 4) {
        if (open_ends === 2) return is_ai ? SCORE_OPEN_FOUR : -BLOCK_OPEN_FOUR_OR_WIN;
        if (open_ends === 1) return is_ai ? SCORE_HALF_OPEN_FOUR : -BLOCK_HALF_OPEN_FOUR;
    }
    if (count === 3) {
        if (open_ends === 2) return is_ai ? SCORE_OPEN_THREE : -BLOCK_OPEN_THREE;
        if (open_ends === 1) return is_ai ? SCORE_HALF_OPEN_THREE : -BLOCK_HALF_OPEN_THREE;
    }
    if (count === 2) {
        if (open_ends === 2) return is_ai ? SCORE_OPEN_TWO : -10; 
        if (open_ends === 1) return is_ai ? 10 : -5;
    }
    return 0;
}

function evaluate_board_heuristic(board, ai_player) {
    let opponent = (ai_player === 1) ? -1 : 1;
    let total_score = 0;
    
    const dr = [0, 1, 1, 1];
    const dc = [1, 0, 1, -1];

    for (let r = 0; r < BOARD_SIZE; ++r) {
        for (let c = 0; c < BOARD_SIZE; ++c) {
            let player = board[r * BOARD_SIZE + c];
            if (player === EMPTY) continue;

            let is_ai = (player === ai_player);

            for (let d = 0; d < 4; ++d) {
                let back_r = r - dr[d];
                let back_c = c - dc[d];
                // Skip if this is not the start of the chain
                if (back_r >= 0 && back_r < BOARD_SIZE && back_c >= 0 && back_c < BOARD_SIZE && board[back_r * BOARD_SIZE + back_c] === player) {
                    continue;
                }

                let count = 1;
                let open_ends = 0;

                if (back_r >= 0 && back_r < BOARD_SIZE && back_c >= 0 && back_c < BOARD_SIZE && board[back_r * BOARD_SIZE + back_c] === EMPTY) {
                    open_ends++;
                }

                let curr_r = r + dr[d];
                let curr_c = c + dc[d];
                while (curr_r >= 0 && curr_r < BOARD_SIZE && curr_c >= 0 && curr_c < BOARD_SIZE && board[curr_r * BOARD_SIZE + curr_c] === player) {
                    count++;
                    curr_r += dr[d];
                    curr_c += dc[d];
                }

                if (curr_r >= 0 && curr_r < BOARD_SIZE && curr_c >= 0 && curr_c < BOARD_SIZE && board[curr_r * BOARD_SIZE + curr_c] === EMPTY) {
                    open_ends++;
                }

                let chain_score = evaluate_chain(count, open_ends, is_ai);
                if (chain_score >= 10000000) return 10000000;
                if (chain_score <= -10000000) return -10000000;
                total_score += chain_score;
            }
        }
    }
    return total_score;
}

function get_candidates(board, ai_player) {
    let empty_cells = [];
    let has_piece = false;

    for (let r = 0; r < BOARD_SIZE; ++r) {
        for (let c = 0; c < BOARD_SIZE; ++c) {
            if (board[r * BOARD_SIZE + c] === EMPTY) {
                empty_cells.push({r, c});
            } else {
                has_piece = true;
            }
        }
    }

    if (!has_piece) {
        return [{score: 0, r: 7, c: 7}]; 
    }

    let candidates = [];
    for (let cell of empty_cells) {
        let r = cell.r;
        let c = cell.c;
        
        let min_r = Math.max(0, r - 2);
        let max_r = Math.min(BOARD_SIZE - 1, r + 2);
        let min_c = Math.max(0, c - 2);
        let max_c = Math.min(BOARD_SIZE - 1, c + 2);
        
        let found_near = false;
        for (let i = min_r; i <= max_r; ++i) {
            for (let j = min_c; j <= max_c; ++j) {
                if (board[i * BOARD_SIZE + j] !== EMPTY) {
                    found_near = true;
                    break;
                }
            }
            if (found_near) break;
        }
        
        if (found_near) {
            // Move ordering: evaluate the board with this move
            board[r * BOARD_SIZE + c] = ai_player; 
            let score = evaluate_board_heuristic(board, ai_player);
            board[r * BOARD_SIZE + c] = EMPTY;
            candidates.push({score, r, c});
        }
    }

    candidates.sort((a, b) => b.score - a.score);
    return candidates;
}

function minimax(board, depth, alpha, beta, is_maximizing, ai_player) {
    let base_eval = evaluate_board_heuristic(board, ai_player);
    if (base_eval >= 9000000) return {score: base_eval + depth, r: -1, c: -1};
    if (base_eval <= -9000000) return {score: base_eval - depth, r: -1, c: -1};

    if (depth === 0) return {score: base_eval, r: -1, c: -1};

    let candidates = get_candidates(board, ai_player);
    if (candidates.length === 0) return {score: base_eval, r: -1, c: -1};

    let opponent = (ai_player === 1) ? -1 : 1;
    let best_r = -1;
    let best_c = -1;

    if (is_maximizing) {
        let max_eval = -999999999;
        for (let move of candidates) {
            board[move.r * BOARD_SIZE + move.c] = ai_player;
            let result = minimax(board, depth - 1, alpha, beta, false, ai_player);
            board[move.r * BOARD_SIZE + move.c] = EMPTY;

            if (result.score > max_eval) {
                max_eval = result.score;
                best_r = move.r;
                best_c = move.c;
            }
            alpha = Math.max(alpha, result.score);
            if (beta <= alpha) break; 
        }
        return {score: max_eval, r: best_r, c: best_c};
    } else {
        let min_eval = 999999999;
        for (let move of candidates) {
            board[move.r * BOARD_SIZE + move.c] = opponent;
            let result = minimax(board, depth - 1, alpha, beta, true, ai_player);
            board[move.r * BOARD_SIZE + move.c] = EMPTY;

            if (result.score < min_eval) {
                min_eval = result.score;
                best_r = move.r;
                best_c = move.c;
            }
            beta = Math.min(beta, result.score);
            if (beta <= alpha) break;
        }
        return {score: min_eval, r: best_r, c: best_c};
    }
}

function get_best_move(board, player_turn, max_depth) {
    // Just run top-level minimax
    let result = minimax(board, max_depth, -999999999, 999999999, true, player_turn);
    if (result.r === -1) {
        // Fallback
        let cands = get_candidates(board, player_turn);
        return cands[0].r * 15 + cands[0].c;
    }
    return result.r * 15 + result.c;
}

module.exports = {
    get_best_move
};
