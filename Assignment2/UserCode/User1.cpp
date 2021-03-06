/******************************************************************\
 * Author: Nicco Simone
 * Copyright 2015, DigiPen Institute of Technology
\******************************************************************/

#include <assert.h>
#include <map>
#include "../Drivers/Driver1.hpp"

//=========================================================
// Private vars and types
//

static const int NOT_ACCEPTING           = 0;
static const int MAX_STATES              = 512;
static const int MAX_EDGES               = 1024;
static const char DEFAULT_EDGE_CONDITION = CHAR_MAX;

class DfaState;
struct DfaEdge;

static DfaState * s_states = nullptr;
static int s_stateIndex    = 0;
static DfaEdge * s_edges = nullptr;
static int s_edgeIndex = 0;
static std::map<std::string, unsigned> s_keywordToId;

typedef bool (*EdgeCondition)(const DfaEdge * edge,  char c);

static const char * s_tokenSymbols[] = {
    #define TOKEN(Name, Value) Value,
    #include "../Drivers/TokenSymbols.inl"
    #undef TOKEN
};

static const char * s_tokenKeywords[] = {
    #define TOKEN(Name, Value) Value,
    #include "../Drivers/TokenKeywords.inl"
    #undef TOKEN
};


//=========================================================
struct DfaEdge {
    DfaState * m_state;
    DfaEdge * m_nextSibling;
    EdgeCondition m_condition;
    char m_simpleCondition;
};

//=========================================================
// external code is forward declaring with class
class DfaState {
public:
    DfaEdge * m_edge;

    // only one default edge per state
    DfaEdge * m_defaultEdge;

    unsigned int m_tokenType;
    bool m_accepting;
};

//=========================================================
struct DfaTokenReader {
    const char * m_stream;
    int m_tokenLength;
    DfaState * m_lastAcceptingState;
    DfaState * m_nextAcceptingState;
    bool m_error;
};

//=========================================================
// Private Methods
//

//=========================================================
static bool SimpleEdgeCondition (const DfaEdge * edge, char c) {
    return edge->m_simpleCondition == c;
}

//=========================================================
static bool WhiteSpaceEdgeCondition (const DfaEdge * edge, char c) {
    return  c == ' '  ||
            c == '\r' ||
            c == '\n' ||
            c == '\t';
}

//=========================================================
static bool AlphaEdgeCondition (const DfaEdge * edge, char c) {
    return c >= 'a' && c <= 'z' ||
           c >= 'A' && c <= 'Z';
}

//=========================================================
static bool IntegerEdgeCondition (const DfaEdge * edge, char c) {
    return c >= '0' && c <= '9';
}

//=========================================================
static bool EscapedCharEdgeCondition (const DfaEdge * edge, char c) {
    return  c == 'n' ||
            c == 'r' ||
            c == 't' ||
            c == '\"';
}

//=========================================================
static bool EndOfLineEdgeCondition(const DfaEdge * edge, char c) {
    return  c == '\n' ||
            c == '\r' ||
            c == '\0';
}

//=========================================================
static void InternalAddEdge (DfaState * from, DfaState * to, char c, EdgeCondition condition) {
    assert(s_edgeIndex < MAX_EDGES);
    if (!s_edges) {
        static DfaEdge edges[MAX_EDGES];
        s_edges = edges;
        memset(s_edges, 0, sizeof(s_edges) / sizeof(s_edges[0]));
    }

    // link new edge in state
    DfaEdge * edge = &s_edges[s_edgeIndex++];
    if (c == DEFAULT_EDGE_CONDITION) {
        assert(!from->m_defaultEdge);
        from->m_defaultEdge = edge;
    }
    else {
        DfaEdge ** curr = &from->m_edge;
        while (*curr) {
            curr = &(*curr)->m_nextSibling;
        }
        *curr = edge;
    }
    edge->m_simpleCondition = c;
    edge->m_state = to;
    edge->m_condition = condition ? condition : &SimpleEdgeCondition;
}

//=========================================================
static void InternalResetEdge (DfaEdge * edge) {
    edge->m_condition = 0;
    edge->m_state = nullptr;
    edge->m_simpleCondition = 0;
    edge->m_nextSibling = nullptr;
}

//=========================================================
static void InternalParseToken (DfaTokenReader * reader, DfaState * state, int streamOffset) {
    // save position if this state is accepting
    if (state->m_accepting) {
        // this is a signal to previous callers
        reader->m_nextAcceptingState = state;

        reader->m_lastAcceptingState = state;
        reader->m_tokenLength = streamOffset;
    }

    if (*(reader->m_stream + streamOffset) == '\0') {
        if (!reader->m_tokenLength) {
            reader->m_tokenLength = streamOffset;
        }
        return;
    }

    // look at outgoing edges to find next match
    DfaEdge * edge = state->m_edge;
    bool foundEdge = false;
    while (edge) {
        char c = *(reader->m_stream + streamOffset);
        if (edge->m_condition(edge, c)) {
            reader->m_nextAcceptingState = nullptr;
            InternalParseToken(reader, edge->m_state, streamOffset + 1);
            if (!reader->m_nextAcceptingState) {
                reader->m_error = true;
            }
            foundEdge = true;
            break;
        }
        edge = edge->m_nextSibling;
    }

    // if we didn't find an edge, see if we have a default edge and check that
    if (state->m_defaultEdge && !reader->m_error && !foundEdge) {
        assert(state->m_defaultEdge->m_condition(state->m_defaultEdge, DEFAULT_EDGE_CONDITION));
        InternalParseToken(reader, state->m_defaultEdge->m_state, streamOffset + 1);
    }

    if (!reader->m_tokenLength) {
        reader->m_tokenLength = streamOffset;
    }
}

//=========================================================
void InternalReadToken(DfaState * startingState, const char * stream, Token & outToken) {
    outToken.mTokenType = 0;

    DfaTokenReader reader;
    reader.m_stream                = stream;
    reader.m_lastAcceptingState    = nullptr;
    reader.m_tokenLength = 0;
    reader.m_error = false;
    reader.m_nextAcceptingState = nullptr;

    InternalParseToken(&reader, startingState, 0);

    if (reader.m_lastAcceptingState) {
        outToken.mTokenType = reader.m_lastAcceptingState->m_tokenType;
        if (reader.m_lastAcceptingState->m_tokenType == TokenType::Identifier) {
            std::string id(reader.m_stream, reader.m_tokenLength);
            if (s_keywordToId.count(id)) {
                outToken.mTokenType = s_keywordToId[id];
            }
        }
    }
    outToken.mText   = reader.m_stream;
    outToken.mLength = reader.m_tokenLength;
}

//=========================================================
// Public Methods
//

//=========================================================
DfaState * AddState (int acceptingToken) {
    assert(s_stateIndex < MAX_STATES);
    if (s_states == nullptr) {
        static DfaState state_pool[MAX_STATES];
        memset(state_pool, 0, sizeof(state_pool) / sizeof(state_pool[0]));
        s_states = state_pool;
    }

    DfaState * state = &s_states[s_stateIndex++];
    state->m_accepting = acceptingToken != NOT_ACCEPTING;
    state->m_tokenType = acceptingToken;
    state->m_edge = nullptr;

    return state;
}

//=========================================================
void AddEdge (DfaState * from, DfaState * to, char c) {
    assert(from && to && c != DEFAULT_EDGE_CONDITION);
    InternalAddEdge(from, to, c, nullptr);
}

//=========================================================
void AddDefaultEdge (DfaState * from, DfaState * to) {
    assert(from && to);
    InternalAddEdge(from, to, DEFAULT_EDGE_CONDITION, nullptr);
}

//=========================================================
void ReadToken(DfaState * startingState, const char * stream, Token & outToken) {
    InternalReadToken(startingState, stream, outToken);
}

//=========================================================
void DeleteStateAndChildren (DfaState * root) {
    s_stateIndex = 0;
    s_keywordToId.clear();
    s_edgeIndex = 0;
    memset(s_states, 0, (MAX_STATES) * sizeof(DfaState));
    memset(s_edges, 0,  MAX_EDGES * sizeof(DfaEdge));
}

//=========================================================
void ReadLanguageToken(DfaState * startingState, const char * stream, Token & outToken) {
    InternalReadToken(startingState, stream, outToken);
}

//=========================================================
DfaState* CreateLanguageDfa() {
    DfaState * root = AddState(NOT_ACCEPTING);

    // store out symbol states
    std::map<std::string, DfaState *> symbolStates;

    // find max symbol size
    int maxLength = 0;
    for (int i = 0; i < sizeof(s_tokenSymbols) / sizeof(s_tokenSymbols[0]); ++i) {
        const char * symbol = s_tokenSymbols[i];
        int l = strlen(symbol);
        if (l > maxLength) {
            maxLength = l;
        }
    }

    // create symbol dfa
    for (int tokenLength = 1; tokenLength <= maxLength; ++tokenLength) {
        for (int i = 0; i < sizeof(s_tokenSymbols) / sizeof(s_tokenSymbols[0]); ++i) {
            const char * symbol = s_tokenSymbols[i];
            if (strlen(symbol) == tokenLength) {
                DfaState * newState = AddState(TokenType::SymbolStart + 1 + i);
                std::string tokenBuffer(symbol);
                if (tokenBuffer.size() > 1) {
                    tokenBuffer.pop_back();
                }
                auto parentState = symbolStates.find(tokenBuffer);
                char edgeChar = *(symbol + tokenLength - 1);
                if (parentState != symbolStates.end()) {
                    AddEdge(parentState->second, newState, edgeChar);
                }
                else {
                    AddEdge(root, newState, *symbol);
                }
                symbolStates[symbol] = newState;
            }
        }
    }

    DfaState * whiteSpace = AddState(TokenType::Whitespace);
    DfaState * identifier = AddState(TokenType::Identifier);
    DfaState * integerLiteral = AddState (TokenType::IntegerLiteral);
    DfaState * floatLiteral = AddState(TokenType::FloatLiteral);
    DfaState * stringLiteral = AddState(TokenType::StringLiteral);
    DfaState * charLiteral = AddState(TokenType::CharacterLiteral);
    DfaState * multiComment = AddState(TokenType::MultiLineComment);
    DfaState * singleComment = AddState(TokenType::SingleLineComment);

    // integer
    InternalAddEdge(root, integerLiteral, 0, IntegerEdgeCondition);
    InternalAddEdge(integerLiteral, integerLiteral, 0, IntegerEdgeCondition);

    // float
    DfaState * floatDot = AddState(NOT_ACCEPTING);
    InternalAddEdge(integerLiteral, floatDot, '.', nullptr);
    InternalAddEdge(floatDot, floatLiteral, 0, IntegerEdgeCondition);
    InternalAddEdge(floatLiteral, floatLiteral, 0, IntegerEdgeCondition);
    InternalAddEdge(floatLiteral, floatLiteral, 'e', nullptr);
    InternalAddEdge(floatLiteral, floatLiteral, '+', nullptr);
    InternalAddEdge(floatLiteral, floatLiteral, '-', nullptr);
    DfaState * floatLiteralEnd = AddState(TokenType::FloatLiteral);
    InternalAddEdge(floatLiteral, floatLiteralEnd, 'f', nullptr);

    // string literal
    DfaState * stringLiteralStart = AddState(NOT_ACCEPTING);
    InternalAddEdge(root, stringLiteralStart, '"', nullptr);
    InternalAddEdge(stringLiteralStart, stringLiteral, '"', nullptr);
    DfaState * slEscaped = AddState(NOT_ACCEPTING);
    InternalAddEdge(stringLiteralStart, slEscaped, '\\', nullptr);
    InternalAddEdge(slEscaped, stringLiteralStart, 0, EscapedCharEdgeCondition);
    AddDefaultEdge(stringLiteralStart, stringLiteralStart);

    // char literal
    DfaState * charLiteralStart = AddState(NOT_ACCEPTING);
    InternalAddEdge(root, charLiteralStart, '\'', nullptr);
    InternalAddEdge(charLiteralStart, charLiteral, '\'', nullptr);
    DfaState * clEscaped = AddState(NOT_ACCEPTING);
    InternalAddEdge(charLiteralStart, clEscaped, '\\', nullptr);
    InternalAddEdge(clEscaped, charLiteralStart, 0, EscapedCharEdgeCondition);
    AddDefaultEdge(charLiteralStart, charLiteralStart);

    // multiline comment
    DfaState * divisionState = symbolStates[s_tokenSymbols[TokenType::Divide - TokenType::SymbolStart - 1]];
    DfaState * mlcStart = AddState(NOT_ACCEPTING);
    DfaState * mlcStarFinder = AddState(NOT_ACCEPTING);
    InternalAddEdge(divisionState, mlcStart, '*', nullptr);
    InternalAddEdge(mlcStart, mlcStarFinder, '*', nullptr);
    InternalAddEdge(mlcStarFinder, multiComment, '/', nullptr);
    AddDefaultEdge(mlcStarFinder, mlcStart);
    AddDefaultEdge(mlcStart, mlcStart);

    // singleline comment
    DfaState * slcStart = AddState(NOT_ACCEPTING);
    InternalAddEdge(divisionState, slcStart, '/', nullptr);
    InternalAddEdge(slcStart, singleComment, 0, EndOfLineEdgeCondition);
    AddDefaultEdge(slcStart, slcStart);

    // whitespace
    InternalAddEdge(root, whiteSpace, 0, WhiteSpaceEdgeCondition);
    InternalAddEdge(whiteSpace, whiteSpace, 0, WhiteSpaceEdgeCondition);

    // identifier
    InternalAddEdge(root, identifier, 0, AlphaEdgeCondition);
    InternalAddEdge(root, identifier, '_', nullptr);

    InternalAddEdge(identifier, identifier, 0, AlphaEdgeCondition);
    InternalAddEdge(identifier, identifier, '_', nullptr);
    InternalAddEdge(identifier, identifier, 0, IntegerEdgeCondition);

    // parse keywords
    for (unsigned i = 0; i < sizeof(s_tokenKeywords) / sizeof(s_tokenKeywords[0]); ++i) {
        s_keywordToId[s_tokenKeywords[i]] = TokenType::KeywordStart + 1 + i;
    }

    return root;
}
