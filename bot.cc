#include <node_api.h>
#include <unistd.h>
#include "ChessBot.h"

chess::Board board;
ChessBot bot;

napi_value getBestMove(napi_env env, napi_callback_info info) {
    napi_value output;

    chess::Move bestMove;
    int depth = 4;
    int piecesLeft = bot.piecesLeft(&board);
    if (piecesLeft < 10) {
        depth = 5;
    } else if (piecesLeft < 5) {
        depth = 6;
    }

    bestMove = bot.getBestMove(&board, depth, (board.sideToMove() == chess::Color::WHITE));
    
    std::string moveSan = chess::uci::moveToSan(board, bestMove);
    board.makeMove(bestMove);

    napi_create_string_utf8(env, moveSan.c_str(), moveSan.length(), &output);

    return output;
}

napi_value doMove(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    size_t str_length;

    napi_value args[argc];
    std::string san;

    napi_get_cb_info(env, info, &argc, args, NULL, NULL);

    napi_get_value_string_utf8(env, args[0], nullptr, 0, &str_length);
    san = std::string(str_length, '\0');
    napi_get_value_string_utf8(env, args[0], &san[0], san.length() + 1, nullptr);

    chess::Move opponentMove;
    opponentMove = chess::uci::parseSan(board, san);
    //std::cout << "Move: " << chess::uci::moveToSan(board, opponentMove) << std::endl;

    board.makeMove(opponentMove);

    return NULL;
}

napi_value setFen(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[argc];
    size_t str_length;

    napi_get_cb_info(env, info, &argc, args, NULL, NULL);

    napi_get_value_string_utf8(env, args[0], nullptr, 0, &str_length);

    std::string fen(str_length, '\0');

    // Copy the napi_value string content to std::string
    size_t copied;
    napi_get_value_string_utf8(env, args[0], &fen[0], fen.length() + 1, &copied);

    board.setFen(fen);
    std::cout << board.getFen() << std::endl;

    bot = ChessBot();
    bot.useBook();
    return NULL;
}

napi_value init(napi_env env, napi_value exports) {
    napi_value getBestMoveFunc;
    napi_value setFenFunc;
    napi_value doMoveFunc;

    napi_create_function(env, "getBestMove", NAPI_AUTO_LENGTH, getBestMove, nullptr, &getBestMoveFunc);
    napi_set_named_property(env, exports, "getBestMove", getBestMoveFunc);

    napi_create_function(env, "setFen", NAPI_AUTO_LENGTH, setFen, nullptr, &setFenFunc);
    napi_set_named_property(env, exports, "setFen", setFenFunc);

    napi_create_function(env, "doMove", NAPI_AUTO_LENGTH, doMove, nullptr, &doMoveFunc);
    napi_set_named_property(env, exports, "doMove", doMoveFunc);

    return exports;
}

NAPI_MODULE(NODE_GYP_BOT, init);