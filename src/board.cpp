/**
 * Constructs a new Board object to the starting chess position.
 */

#include "board.hpp"

Board::Board() {
    // initialize pieces
    pieceBB[0] = Rank1 | Rank2;
    pieceBB[1] = Rank7 | Rank8;
    pieceBB[2] = Rank2 | Rank7;
    pieceBB[3] = sqToBB[B1] | sqToBB[G1] | sqToBB[B8] | sqToBB[G8];
    pieceBB[4] = sqToBB[C1] | sqToBB[F1] | sqToBB[C8] | sqToBB[F8];
    pieceBB[5] = sqToBB[A1] | sqToBB[H1] | sqToBB[A8] | sqToBB[H8];
    pieceBB[6] = sqToBB[D1] | sqToBB[D8];
    pieceBB[7] = sqToBB[E1] | sqToBB[E8];

    occupiedBB = (pieceBB[0] | pieceBB[1]);
    emptyBB = ~occupiedBB;

    enPassant = SQ_NONE;
    castling = 0b0000;
    toMove = nWhite;
    fiftyCounter = 0;
}

/**
 * Constructs a new Board object to the given position
 *
 * @param FEN the desired starting position in FEN form
 */
Board::Board(std::string FEN) {
    
}

// Returns pieces of the given piece type
Bitboard Board::getPieces(Piece pt) const {
    return pieceBB[2 + pt];
}

// Returns pieces of the given color
Bitboard Board::getPieces(Color ct) const {
    return pieceBB[ct];
}

// Returns pieces of the given piece and color
Bitboard Board::getPieces(Piece pt, Color ct) const {
    return pieceBB[2 + pt] & pieceBB[ct];
}

// Returns bitboard of all the white pawns
Bitboard Board::getWhitePawns() const {
    return pieceBB[0] & pieceBB[2];
}

// Returns bitboard of all the white knights
Bitboard Board::getWhiteKnights() const {
    return pieceBB[0] & pieceBB[3];
}

// Returns bitboard of all the white bishops
Bitboard Board::getWhiteBishops() const {
    return pieceBB[0] & pieceBB[4];
}

// Returns bitboard of all the white rooks
Bitboard Board::getWhiteRooks() const {
    return pieceBB[0] & pieceBB[5];
}

// Returns bitboard of all the white queens
Bitboard Board::getWhiteQueens() const {
    return pieceBB[0] & pieceBB[6];
}

// Returns bitboard of the white king
Bitboard Board::getWhiteKing() const {
    return pieceBB[0] & pieceBB[7];
}

// Returns bitboard of all the black pawns
Bitboard Board::getBlackPawns() const {
    return pieceBB[1] & pieceBB[2];
}

// Returns bitboard of all the black knights
Bitboard Board::getBlackKnights() const {
    return pieceBB[1] & pieceBB[3];
}

// Returns bitboard of all the black bishops
Bitboard Board::getBlackBishops() const {
    return pieceBB[1] & pieceBB[4];
}

// Returns bitboard of all the black rooks
Bitboard Board::getBlackRooks() const {
    return pieceBB[1] & pieceBB[5];
}

// Returns bitboard of all the black queens
Bitboard Board::getBlackQueens() const {
    return pieceBB[1] & pieceBB[6];
}

// Returns bitboard of the black king
Bitboard Board::getBlackKing() const {
    return pieceBB[1] & pieceBB[7];
}

Bitboard Board::getOccupied() const {
    return occupiedBB;
}

Bitboard Board::getEmpty() const {
    return emptyBB;
}

Square Board::enPassantTarget() const {
    return enPassant;
}

short Board::getCastlingRights() const {
    return castling;
}

Color Board::getToMove() const {
    return toMove;
}

bool Board::attacked(int square, Color side) const {
    Bitboard pawns = getPieces(nPawn, side);
    if (pawnAttacks[side^1][square] & pawns) {
        return true;
    }
    Bitboard knights = getPieces(nKnight, side);
    if (knightAttacks[square] & knights) {
        return true;
    }
    Bitboard king = getPieces(nKing, side);
    if (kingAttacks[square] & king) {
        return true;
    }
    Bitboard bishopsQueens = getPieces(nQueen, side) | getPieces(nBishop, side);
    if (slidingAttacksBB<nBishop>(square, occupiedBB) & bishopsQueens) {
        return true;
    }
    Bitboard rooksQueens = getPieces(nQueen, side) | getPieces(nRook, side);
    if (slidingAttacksBB<nRook>(square, occupiedBB) & rooksQueens) {
        return true;
    }
}

Piece Board::getPiece(int sq) const {
    Bitboard sqBB = sqToBB[sq];
    if (sqBB & pieceBB[2 + nPawn]) {
        return nPawn;
    } else if (sqBB & pieceBB[2 + nKnight]) {
        return nKnight;
    } else if (sqBB & pieceBB[2 + nBishop]) {
        return nBishop;
    } else if (sqBB & pieceBB[2 + nRook]) {
        return nRook;
    } else if (sqBB & pieceBB[2 + nQueen]) {
        return nQueen;
    } else if (sqBB & pieceBB[2 + nKing]) {
        return nKing;
    } else {
        return PIECE_NONE;
    }
}

Color Board::getColor(int sq) const {
    Bitboard sqBB = sqToBB[sq];
    if (sqBB & pieceBB[nWhite]) {
        return nWhite;
    } else if (sqBB & pieceBB[nBlack]) {
        return nBlack;
    } else {
        return COLOR_NONE;
    }
}

bool Board::isLegal(Move m) const {
    int start = m.getFrom();
    int end = m.getTo();
    Color startColor = getColor((Square)start);
    Color endColor = getColor((Square)end);
    Piece startPiece = getPiece((Square)start);
    Color otherColor = (startColor == nWhite ? nBlack : nWhite);

    int pawnSq = end - 8 * (2 * startColor - 1);

    if (toMove != startColor || toMove == endColor) {
        return false;
    }

    if (m.isCapture() && endColor == startColor) {
        return false;
    }
    
    if (m.isCapture() && (endColor == COLOR_NONE) && (m.getFlags() != 5)) {
        return false;
    }
    if (m.getFlags() == 5) { // en passant
        if (end != enPassant) {
            return false;
        }
        if (endColor != COLOR_NONE || startPiece != nPawn) {
            return false;
        }
        // check if there actually is an opposing pawn in the right place
        if (getPiece((Square)(pawnSq)) != nPawn) {
            return false;
        }
    }

    Bitboard bishopsQueens = (pieceBB[nBishop + 2] | pieceBB[nQueen + 2]) &
        pieceBB[otherColor];
    Bitboard rooksQueens = (pieceBB[nRook + 2] | pieceBB[nQueen + 2]) &
        pieceBB[otherColor];
    Bitboard kingLoc = pieceBB[nKing + 2] & pieceBB[startColor];
    Bitboard newOccupied = occupiedBB ^ sqToBB[start] ^ sqToBB[end] ^ sqToBB[pawnSq];
    if ((allSlidingAttacks<nBishop>(bishopsQueens, newOccupied) & kingLoc) ||
            (allSlidingAttacks<nRook>(rooksQueens, newOccupied) & kingLoc)) {
        return false;
    }

    if (startPiece == nKing) {
        return m.getFlags() == 2 || m.getFlags() == 3 || !attacked((Square)m.getTo(),
                otherColor);
    }
    if (attacked((Square)bitScanForward(kingLoc), otherColor)) {
        return false;
    }
    return true;
}

void Board::makeMove(Move m) {
    if (!isLegal(m)) { return;}

    fiftyCounter++;
    toMove = (toMove == nWhite ? nBlack : nWhite);

    // Get all the information from the move.
    int start = m.getFrom();
    int end = m.getTo();
    int flags = m.getFlags();
    bool capture = m.isCapture();
    bool prom = m.isPromotion();
    // Bitboards corresponding to start and end square.
    Bitboard startBB = sqToBB[start];
    Bitboard endBB = sqToBB[end];
    Bitboard startEndBB = startBB | endBB;
    // Get the start and end pieces.
    Piece startP = getPiece(start);
    Color startC = getColor(start);
    Piece endP = getPiece(end);
    Color endC = getColor(end);

    pieceBB[(int)startP + 2] ^= startEndBB; 
    pieceBB[(int)startC] ^= startEndBB;

    if (flags == 1) {
        enPassant = (Square)(startC == nWhite ? end - 8 : end + 8);
    }

    if (flags == 5) { // en passant
        endP = nPawn;
        if (startC == nWhite) {
            pieceBB[2] ^= sqToBB[end - 8];    
            pieceBB[1] ^= sqToBB[end - 8];
        } else {
            pieceBB[2] ^= sqToBB[end + 8];    
            pieceBB[0] ^= sqToBB[end + 8];
        }
    } else if (capture) {
        pieceBB[(int) endP + 2] ^= endBB;
        pieceBB[(int) endC] ^= endBB;
    }

    if (prom) {
        int promPiece = 1 + (flags & 3);
        pieceBB[promPiece + 2] ^= endBB;
        pieceBB[2] ^= endBB;
    } 

    if (((flags >> 1) & 1) == 1) { // castling
        if ((flags & 1) == 0) { // kingside
            if (startC == nWhite) {
                pieceBB[nRook + 2] ^= (sqToBB[F1] | sqToBB[H1]);
                pieceBB[startC] ^= (sqToBB[F1] | sqToBB[H1]);
                castling &= 0b1100;
            } else { 
                pieceBB[nRook + 2] ^= (sqToBB[F8] | sqToBB[H8]);
                pieceBB[startC] ^= (sqToBB[F8] | sqToBB[H8]);
                castling &= 0b0011;
            }
        } else { // queenside
            if (startC == nWhite) {
                pieceBB[nRook + 2] ^= (sqToBB[A1] | sqToBB[C1]);
                pieceBB[startC] ^= (sqToBB[A1] | sqToBB[C1]);
                castling &= 0b1100;
            } else { 
                pieceBB[nRook + 2] ^= (sqToBB[A8] | sqToBB[C8]);
                pieceBB[startC] ^= (sqToBB[A8] | sqToBB[C8]);
                castling &= 0b0011;
            }
        } 
    } 

    if (startP == nRook) {
        if (start == A1) {
            castling &= 0b1011;
        } else if (start == H1) {
            castling &= 0b0111;
        } else if (start == A8) {
            castling &= 0b1110;
        } else if (start == H8) {
            castling &= 0b1101;
        }
    }

    if (endP == nRook) {
        if (start == A1) {
            castling &= 0b1011;
        } else if (start == H1) {
            castling &= 0b0111;
        } else if (start == A8) {
            castling &= 0b1110;
        } else if (start == H8) {
            castling &= 0b1101;
        }
    }

    if (startP == nKing) {
        if (startC == nWhite) {
            castling &= 0b0011;
        } else {
            castling &= 0b1100;
        }
    }
}

void Board::unmakeMove(Move m) {
    fiftyCounter++;
    toMove = (toMove == nWhite ? nBlack : nWhite);

    // Get all the information from the move.
    int start = m.getFrom();
    int end = m.getTo();
    int flags = m.getFlags();
    bool capture = m.isCapture();
    bool prom = m.isPromotion();
    // Bitboards corresponding to start and end square.
    Bitboard startBB = sqToBB[start];
    Bitboard endBB = sqToBB[end];
    Bitboard startEndBB = startBB | endBB;
    // Get the start and end pieces.
    Piece startP = getPiece(start);
    Color startC = getColor(start);
    Piece endP = getPiece(end);
    Color endC = getColor(end);

    pieceBB[(int)startP + 2] ^= startEndBB; 
    pieceBB[(int)startC] ^= startEndBB;

    if (flags == 1) {
        enPassant = (Square)(startC == nWhite ? end - 8 : end + 8);
    }

    if (flags == 5) { // en passant
        endP = nPawn;
        if (startC == nWhite) {
            pieceBB[2] ^= sqToBB[end - 8];    
            pieceBB[1] ^= sqToBB[end - 8];
        } else {
            pieceBB[2] ^= sqToBB[end + 8];    
            pieceBB[0] ^= sqToBB[end + 8];
        }
    } else if (capture) {
        pieceBB[(int) endP + 2] ^= endBB;
        pieceBB[(int) endC] ^= endBB;
    }

    if (prom) {
        int promPiece = 1 + (flags & 3);
        pieceBB[promPiece + 2] ^= endBB;
        pieceBB[2] ^= endBB;
    } 

    if (((flags >> 1) & 1) == 1) { // castling
        if ((flags & 1) == 0) { // kingside
            if (startC == nWhite) {
                pieceBB[nRook + 2] ^= (sqToBB[F1] | sqToBB[H1]);
                pieceBB[startC] ^= (sqToBB[F1] | sqToBB[H1]);
                castling &= 0b1100;
            } else { 
                pieceBB[nRook + 2] ^= (sqToBB[F8] | sqToBB[H8]);
                pieceBB[startC] ^= (sqToBB[F8] | sqToBB[H8]);
                castling &= 0b0011;
            }
        } else { // queenside
            if (startC == nWhite) {
                pieceBB[nRook + 2] ^= (sqToBB[A1] | sqToBB[C1]);
                pieceBB[startC] ^= (sqToBB[A1] | sqToBB[C1]);
                castling &= 0b1100;
            } else { 
                pieceBB[nRook + 2] ^= (sqToBB[A8] | sqToBB[C8]);
                pieceBB[startC] ^= (sqToBB[A8] | sqToBB[C8]);
                castling &= 0b0011;
            }
        } 
    } 

    if (startP == nRook) {
        if (start == A1) {
            castling &= 0b1011;
        } else if (start == H1) {
            castling &= 0b0111;
        } else if (start == A8) {
            castling &= 0b1110;
        } else if (start == H8) {
            castling &= 0b1101;
        }
    }

    if (endP == nRook) {
        if (start == A1) {
            castling &= 0b1011;
        } else if (start == H1) {
            castling &= 0b0111;
        } else if (start == A8) {
            castling &= 0b1110;
        } else if (start == H8) {
            castling &= 0b1101;
        }
    }

    if (startP == nKing) {
        if (startC == nWhite) {
            castling &= 0b0011;
        } else {
            castling &= 0b1100;
        }
    }
}
