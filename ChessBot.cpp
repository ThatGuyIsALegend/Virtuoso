#include "ChessBot.h"

ChessBot::ChessBot() {
    for (int i = 0; i < 64; i++) {
        b_pawnSquareTable[i] = w_pawnSquareTable[63-i];
        b_knightSquareTable[i] = w_knightSquareTable[63-i];
        b_bishopSquareTable[i] = w_bishopSquareTable[63-i];
        b_rookSquareTable[i] = w_rookSquareTable[63-i];
        b_queenSquareTable[i] = w_queenSquareTable[63-i];
        b_kingMiddleGameSquareTable[i] = w_kingMiddleGameSquareTable[63-i];
        b_kingEndGameSquareTable[i] = w_kingEndGameSquareTable[63-i];
    }
    times = 0;
    usesBook = false;
}

float ChessBot::countMaterial(chess::Board* board) {
    float whiteMaterial = 0;
    float blackMaterial = 0;

    // Pawns
    whiteMaterial += board->pieces(chess::PieceType::PAWN, chess::Color::WHITE).count() * 1;
    blackMaterial += board->pieces(chess::PieceType::PAWN, chess::Color::BLACK).count() * 1;

    // Bishops
    whiteMaterial += board->pieces(chess::PieceType::BISHOP, chess::Color::WHITE).count() * 3;
    blackMaterial += board->pieces(chess::PieceType::BISHOP, chess::Color::BLACK).count() * 3;

    // Knights
    whiteMaterial += board->pieces(chess::PieceType::KNIGHT, chess::Color::WHITE).count() * 3;
    blackMaterial += board->pieces(chess::PieceType::KNIGHT, chess::Color::BLACK).count() * 3;

    // Rooks
    whiteMaterial += board->pieces(chess::PieceType::ROOK, chess::Color::WHITE).count() * 5;
    blackMaterial += board->pieces(chess::PieceType::ROOK, chess::Color::BLACK).count() * 5;

    // Queens
    whiteMaterial += board->pieces(chess::PieceType::QUEEN, chess::Color::WHITE).count() * 9;
    blackMaterial += board->pieces(chess::PieceType::QUEEN, chess::Color::BLACK).count() * 9;

    return whiteMaterial - blackMaterial;
}

float ChessBot::materialAdjustment(chess::Board* board) {
    float whiteVal = 0;
    float blackVal = 0;
    bool isEndgame = false;
    if (piecesLeft(board) < 10 || (piecesLeft(board) < 15 && board->pieces(chess::PieceType::QUEEN).count() == 0)) {
        isEndgame = true;
    }

    for (int i = 0; i < 64; i++) {
        chess::Piece piece = board->at(i);

        if (piece == chess::Piece::NONE) {
            continue;
        } else if (piece == chess::Piece::WHITEPAWN) {
            whiteVal += w_pawnSquareTable[i];
        } else if (piece == chess::Piece::BLACKPAWN) {
            blackVal += b_pawnSquareTable[i];
        } else if (piece == chess::Piece::WHITEBISHOP) {
            whiteVal += w_bishopSquareTable[i];
        } else if (piece == chess::Piece::BLACKBISHOP) {
            blackVal += b_bishopSquareTable[i];
        } else if (piece == chess::Piece::WHITEKNIGHT) {
            whiteVal += w_knightSquareTable[i];
        } else if (piece == chess::Piece::BLACKKNIGHT) {
            blackVal += b_knightSquareTable[i];
        } else if (piece == chess::Piece::WHITEROOK) {
            whiteVal += w_rookSquareTable[i];
        } else if (piece == chess::Piece::BLACKROOK) {
            blackVal += b_rookSquareTable[i];
        } else if (piece == chess::Piece::WHITEQUEEN) {
            whiteVal += w_queenSquareTable[i];
        } else if (piece == chess::Piece::BLACKQUEEN) {
            blackVal += b_queenSquareTable[i];
        } else if (piece == chess::Piece::WHITEKING) {
            if (!isEndgame)
                whiteVal += w_kingMiddleGameSquareTable[i];
            else
                whiteVal += w_kingEndGameSquareTable[i];
        } else if (piece == chess::Piece::BLACKKING) {
            if (!isEndgame)
                blackVal += b_kingMiddleGameSquareTable[i];
            else
                blackVal += b_kingEndGameSquareTable[i];
        }
    }


    return whiteVal - blackVal;
}

float ChessBot::centerControl(chess::Board* board) {
    float blackCenterControl = 0;
    float whiteCenterControl = 0;

    const int centerSquaresLength = 4;
    chess::Square centerSquares[centerSquaresLength];
    centerSquares[0] = chess::Square("d4");
    centerSquares[1] = chess::Square("d5");
    centerSquares[2] = chess::Square("e4");
    centerSquares[3] = chess::Square("e5");

    for (int i = 0; i < centerSquaresLength; i++) {
        if (board->isAttacked(centerSquares[i], chess::Color::WHITE)) {
            whiteCenterControl += 0.1f;
        }
        if (board->isAttacked(centerSquares[i], chess::Color::BLACK)) {
            blackCenterControl += 0.1f;
        }
    }

    for (int i = 0; i < 64; i++) { // Prefer pawn center control
        chess::Piece piece = board->at(i);

        if (piece.type() != chess::PieceType::PAWN) {
            continue;
        }

        if (piece == chess::Piece::WHITEPAWN) {
            chess::Bitboard b = chess::attacks::pawn(chess::Color::WHITE, i);

            for (int j = 0; j < centerSquaresLength; j++) {
                if (b.check(centerSquares[j].index())) {
                    whiteCenterControl += 1.0f;
                }
            }
        } else {
            chess::Bitboard b = chess::attacks::pawn(chess::Color::BLACK, i);

            for (int j = 0; j < centerSquaresLength; j++) {
                if (b.check(centerSquares[j].index())) {
                    blackCenterControl += 1.0f;
                }
            }
        }

    }

    return whiteCenterControl - blackCenterControl;
}

float ChessBot::evaluatePosition(chess::Board* board) {
    if (board->isGameOver().first == chess::GameResultReason::CHECKMATE) {
        if (board->sideToMove() == chess::Color::BLACK) {
            return 1000;
        } else {
            return -1000;
        }
    } else if (board->isGameOver().second == chess::GameResult::DRAW || board->isGameOver().first == chess::GameResultReason::THREEFOLD_REPETITION) {
        return 0;
    }

    float score = 0;
    score += countMaterial(board);
    score += materialAdjustment(board);
    score += centerControl(board);
    score += attackingSquares(board);

    return score;
}

float ChessBot::attackingSquares(chess::Board* board) {
    float whiteAttackingSquares = 0;
    float blackAttackingSquares = 0;

    for (int i = 0; i < 64; i++) {
        if (board->isAttacked(i, chess::Color::WHITE)) {
            whiteAttackingSquares += 0.05f;
        }
        if (board->isAttacked(i, chess::Color::BLACK)) {
            blackAttackingSquares += 0.05f;
        }
    }

    return whiteAttackingSquares - blackAttackingSquares;
}

float ChessBot::minimax(chess::Board* board, int depth, float alpha, float beta, bool maximizingPlayer) {
    times++;
    chess::Movelist moves;
    chess::movegen::legalmoves(moves, *board);

    if (depth == 0 || moves.size() == 0) {
        float eval = evaluatePosition(board);
        if (eval == 1000) {
            eval += depth;
        }
        if (eval == -1000) {
            eval -= depth;
        }

        return eval;
    }

    if (maximizingPlayer) {
        float maxEval = -1000;
        for (chess::Move move : moves) {
            //std::cout << "Evaluting: " << chess::uci::moveToSan(*board, move) << std::endl;
            board->makeMove(move);
            float eval = minimax(board, depth-1, alpha, beta, false);
            board->unmakeMove(move);
            //std::cout << "Eval: " << eval << std::endl;

            maxEval = std::max(eval, maxEval);
            alpha = std::max(alpha, eval);

            

            if (beta <= alpha) {
                break;
            }
        }
        return maxEval;
    } else {
        float minEval = 1000;
        for (chess::Move move : moves) {
            //std::cout << "Evaluting: " << chess::uci::moveToSan(*board, move) << std::endl;
            board->makeMove(move);
            float eval = minimax(board, depth-1, alpha, beta, true);
            board->unmakeMove(move);
            //std::cout << "Eval: " << eval << std::endl;

            minEval = std::min(eval, minEval);
            beta = std::min(beta, eval);

            

            if (beta <= alpha) {
                break;
            }
        }
        return minEval;
    }
}

void ChessBot::useBook() {
    usesBook = true;
    book.InitBook("Titans.bin");
}

chess::Move ChessBot::getBestMove(chess::Board* board, int depth, bool maximizingPlayer) {
    if (usesBook) {
        chess::Move result = book.getMove(board);
        if (result == chess::Move::NULL_MOVE)
            usesBook = false;
        else
            return result;
    }


    chess::Movelist moves;
    moves = orderMoves(board);

    float maxScore;
    chess::Move bestMove = moves[0];

    float alpha = -1000 - depth;
    float beta = 1000 + depth;

    //std::cout << "Best moves are: " << chess::uci::moveToSan(*board, moves[0]) << ", " << chess::uci::moveToSan(*board, moves[1]) << ", " << chess::uci::moveToSan(*board, moves[2]) << std::endl;

    if (maximizingPlayer)
    {
        maxScore = -1000;
        for (chess::Move move : moves) {
            board->makeMove(move);
            float currentScore = minimax(board, depth, alpha, beta, false);
            board->unmakeMove(move);

            alpha = std::max(alpha, currentScore);

            if (currentScore > maxScore)
            {
                maxScore = currentScore;
                bestMove = move;
            }
            if (beta <= alpha) {
                break;
            }
        }
    } else {
        maxScore = 1000;
        for (chess::Move move : moves) {
            board->makeMove(move);
            float currentScore = minimax(board, depth, alpha, beta, true);
            board->unmakeMove(move);

            beta = std::min(beta, currentScore);

            if (currentScore < maxScore)
            {
                maxScore = currentScore;
                bestMove = move;
            }
            if (beta <= alpha) {
                break;
            }
        }
    }

    std::cout << "Best move: " << chess::uci::moveToSan(*board, bestMove) << std::endl;
    return bestMove;
}

int ChessBot::piecesLeft(chess::Board* board) {
    return board->occ().count();
}

bool compareMoves(const chess::Move& move1, const chess::Move& move2) {
    return move1.score() > move2.score();  // Change the logic as needed
}

chess::Movelist ChessBot::orderMoves(chess::Board* board) {
    chess::Movelist moves;

    chess::movegen::legalmoves(moves, *board);
    int moveSize = moves.size();
    
    for (int i = 0; i < moveSize; i++) {
        int score = 0;
        chess::Piece movePiece = board->at(moves[i].from());
        chess::Piece capturePiece = board->at(moves[i].to());

        if (capturePiece != chess::Piece::NONE) { // If it's a capture
            score += 1;

            score += getPieceValue(capturePiece.type()) - getPieceValue(movePiece.type());
        }

        if (moves[i].typeOf() == chess::Move::PROMOTION) { // If it's a promotion
            score += 1;
        }

        board->makeMove(moves[i]);
        if (board->inCheck()) { // If it's a check
            score += 1;
        }

        //if (board->us(board->sideToMove()) & chess::attacks::attackers(*board, ))
        board->unmakeMove(moves[i]);

        moves[i].setScore(score);
    }
    std::sort(moves.begin(), moves.end(), compareMoves);

    return moves;
}

int ChessBot::getPieceValue(chess::PieceType piece) {
    if (piece == chess::PieceType::PAWN) {
        return 1;
    } else if (piece == chess::PieceType::BISHOP) {
        return 3;
    } else if (piece == chess::PieceType::KNIGHT) {
        return 3;
    } else if (piece == chess::PieceType::ROOK) {
        return 5;
    } else if (piece == chess::PieceType::QUEEN) {
        return 9;
    }

    return 0;
}