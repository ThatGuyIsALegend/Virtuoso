#include "Polyglot.h"

template <typename T>
T swapEndianness(T value) {
    constexpr size_t size = sizeof(T);
    T result = 0;

    for (size_t i = 0; i < size; ++i) {
        result |= ((value >> (8 * (size - 1 - i))) & 0xFF) << (8 * i);
    }

    return result;
}

uint64_t PolyGlot::PolyKeyFromBoard(chess::Board* board) {
    uint64_t result = 0;

    // pieces
    int offset = 0;
    for (int i = 0; i < 64; i++) {
        chess::Square sq(i);
        chess::Piece chessPiece = board->at(sq);
        if (chessPiece == chess::Piece::NONE)
            continue;
        
        PolyGlotPiece polyPiece = toPolyGlotPiece(chessPiece);
        int rank = sq.rank();
        int file = sq.file();

        result ^= Random64[(64 * polyPiece) + (8 * rank) + file];
    }

    // catsling
    offset = 768;
    if (board->castlingRights().has(chess::Color::WHITE, chess::Board::CastlingRights::Side::KING_SIDE)) result ^= Random64[offset + 0];
    if (board->castlingRights().has(chess::Color::WHITE, chess::Board::CastlingRights::Side::QUEEN_SIDE)) result ^= Random64[offset + 1];
    if (board->castlingRights().has(chess::Color::BLACK, chess::Board::CastlingRights::Side::KING_SIDE)) result ^= Random64[offset + 2];
    if (board->castlingRights().has(chess::Color::BLACK, chess::Board::CastlingRights::Side::QUEEN_SIDE)) result ^= Random64[offset + 3];

    // enpassant
    offset = 772;
    chess::Square enPassantSq = board->enpassantSq();
    if (isEnpasantPossible(board)) {
        result ^= Random64[offset + enPassantSq.file()];
    }

    // turn
    offset = 780;
    if (board->sideToMove() == chess::Color::WHITE) {
        result ^= Random64[offset];
    }

    return result;
}

chess::Move PolyGlot::getMove(chess::Board* board) {
    uint64_t key = PolyKeyFromBoard(board);
    uint16_t bestMove = 0;
    uint16_t bestMoveWeight = 0;

    chess::Move result;

    for (int i = 0; i < numEntries; i++) {
        if (key == swapEndianness(entries[i].key)) {
            uint16_t move = swapEndianness(entries[i].move);
            if (bestMoveWeight < swapEndianness(entries[i].weight)) {
                bestMoveWeight = swapEndianness(entries[i].weight);
                bestMove = move;
            }
        }
    }

    if (!bestMove)
        return chess::Move::NULL_MOVE;

    chess::Square from(chess::File((bestMove >> 6) & 7), chess::Rank((bestMove >> 9) & 7));
    chess::Square to(chess::File((bestMove >> 0) & 7), chess::Rank((bestMove >> 3) & 7));

    result = chess::Move::make(from, to);

    return result;
}

bool PolyGlot::isEnpasantPossible(chess::Board* board) {
    chess::Square enPassantSq = board->enpassantSq();
    if (!enPassantSq.is_valid()) {
        return false;
    }

    if (board->sideToMove() == chess::Color::WHITE) {
        for (int i = 0; i < 64; i++) {
            chess::Piece enemyPiece = board->at(i);
            if (enemyPiece != chess::Piece::WHITEPAWN)
                continue;
            
            if (chess::attacks::pawn(chess::Color::WHITE, i).check(enPassantSq.index())) {
                return true;
            }
        }
    } else {
        for (int i = 0; i < 64; i++) {
            chess::Piece enemyPiece = board->at(i);
            if (enemyPiece != chess::Piece::BLACKPAWN)
                continue;
            
            if (chess::attacks::pawn(chess::Color::BLACK, i).check(enPassantSq.index())) {
                return true;
            }
        }
    }

    return false;
}

PolyGlotPiece PolyGlot::toPolyGlotPiece(chess::Piece piece) {
    PolyGlotPiece result;

    if (piece == chess::Piece::BLACKPAWN) {
        result = BLACK_PAWN;
    } else if (piece == chess::Piece::WHITEPAWN) {
        result = WHITE_PAWN;
    } else if (piece == chess::Piece::BLACKKNIGHT) {
        result = BLACK_KNIGHT;
    } else if (piece == chess::Piece::WHITEKNIGHT) {
        result = WHITE_KNIGHT;
    } else if (piece == chess::Piece::BLACKBISHOP) {
        result = BLACK_BISHOP;
    } else if (piece == chess::Piece::WHITEBISHOP) {
        result = WHITE_BISHOP;
    } else if (piece == chess::Piece::BLACKROOK) {
        result = BLACK_ROOK;
    } else if (piece == chess::Piece::WHITEROOK) {
        result = WHITE_ROOK;
    } else if (piece == chess::Piece::BLACKQUEEN) {
        result = BLACK_QUEEN;
    } else if (piece == chess::Piece::WHITEQUEEN) {
        result = WHITE_QUEEN;
    } else if (piece == chess::Piece::BLACKKING) {
        result = BLACK_KING;
    } else if (piece == chess::Piece::WHITEKING) {
        result = WHITE_KING;
    }

    return result;
}

void PolyGlot::InitBook(std::string name) {
    std::ifstream bookFile = std::ifstream(name, std::ios::binary);

    if (!bookFile.is_open()) {
        std::cout << "Error opening file" << std::endl;
        return;
    }

    bookFile.seekg(0, std::ios::end);
    int fileSize = bookFile.tellg();
    bookFile.seekg(0, std::ios::beg);

    if (fileSize < sizeof(BookEntry)) {
        std::cout << "No entries found" << std::endl;
    }

    numEntries = fileSize / sizeof(BookEntry);
    entries = (BookEntry*)malloc(sizeof(BookEntry) * numEntries);

    bookFile.read((char*)entries, sizeof(BookEntry) * numEntries);
    bookFile.close();
}

void PolyGlot::close() {
    free(entries);
    return;
}