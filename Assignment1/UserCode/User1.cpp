/******************************************************************\
 * Author: Nicco Simone
 * Copyright 2015, DigiPen Institute of Technology
\******************************************************************/

#include <assert.h>
#include "../Drivers/Driver1.hpp"

//=========================================================
// Private vars and types
//

static const int NOT_ACCEPTING           = 0;
static const int MAX_STATES              = 2048;
static const int MAX_EDGES_PER_STATE     = 8;
static const char DEFAULT_EDGE_CONDITION = CHAR_MAX;

class DfaState;
struct DfaEdge;

static DfaState * s_states = nullptr;
static int s_stateIndex    = 0;

typedef bool (*EdgeCondition)(const DfaEdge * edge,  char c);

//=========================================================
struct DfaEdge {
    DfaState * m_state;
    EdgeCondition m_condition;
    char m_simpleCondition;
};

//=========================================================
// external code is forward declaring with class
class DfaState {
public:
    DfaEdge m_edges[MAX_EDGES_PER_STATE];

    // only one default edge per state
    DfaEdge m_defaultEdge;

    // does not include default edge
    int m_edgeCount;

    TokenType::Enum m_tokenType;
    bool m_accepting;
};

//=========================================================
struct DfaTokenReader {
    const char * m_stream;
    const char * m_lastAcceptingPosition;
    DfaState * m_lastAcceptingState;
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
static void InternalAddEdge(DfaState * from, DfaState * to, char c, EdgeCondition condition) {
    assert(from->m_edgeCount < MAX_EDGES_PER_STATE);
    DfaEdge * edge;
    if (c == DEFAULT_EDGE_CONDITION) {
        // make sure we haven't defined a default edge already
        assert(!from->m_defaultEdge.m_state);
        edge = &from->m_defaultEdge;
    }
    else {
        edge = &from->m_edges[from->m_edgeCount++];
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
}

//=========================================================
static void InternalDeleteStateAndChildren (DfaState * state, DfaState *& newRoot) {
    while (state->m_edgeCount > 0) {
        DfaEdge * edge = &state->m_edges[--state->m_edgeCount];
        if (edge->m_state != state) {
            InternalDeleteStateAndChildren(edge->m_state, newRoot);
        }
        InternalResetEdge(edge);
    }
    if (state->m_defaultEdge.m_state) {
        if (state->m_defaultEdge.m_state != state) {
            InternalDeleteStateAndChildren(state->m_defaultEdge.m_state, newRoot);
        }
        InternalResetEdge(&state->m_defaultEdge);
    }
    state->m_accepting = false;
    state->m_tokenType = (TokenType::Enum) 0;
    newRoot = state;
}

//=========================================================
static void InternalParseToken (DfaTokenReader * reader, DfaState * state, int streamOffset) {
    // save position if this state is accepting
    if (state->m_accepting) {
        reader->m_lastAcceptingState = state;
        reader->m_lastAcceptingPosition = reader->m_stream + streamOffset;
    }

    if (*(reader->m_stream + streamOffset) == '\0') {
        if (!reader->m_lastAcceptingPosition) {
            reader->m_lastAcceptingPosition = reader->m_stream + streamOffset;
        }
        return;
    }

    // look at outgoing edges to find next match
    for (int edgeIndex = 0; edgeIndex < state->m_edgeCount; ++edgeIndex) {
        DfaEdge * edge = &state->m_edges[edgeIndex];
        char c = *(reader->m_stream + streamOffset);
        if (edge->m_condition(edge, c)) {
            InternalParseToken(reader, edge->m_state, streamOffset + 1);
        }
    }

    // if we didn't find an edge, see if we have a default edge and check that
    if (!reader->m_lastAcceptingState && state->m_defaultEdge.m_state) {
        assert(state->m_defaultEdge.m_condition(&state->m_defaultEdge, DEFAULT_EDGE_CONDITION));
        InternalParseToken(reader, state->m_defaultEdge.m_state, streamOffset + 1);
    }

    if (!reader->m_lastAcceptingPosition) {
        reader->m_lastAcceptingPosition = reader->m_stream + streamOffset;
    }
}

//=========================================================
void InternalReadToken(DfaState * startingState, const char * stream, Token & outToken) {
    outToken.mTokenType = 0;

    DfaTokenReader reader;
    reader.m_stream                = stream;
    reader.m_lastAcceptingState    = nullptr;
    reader.m_lastAcceptingPosition = nullptr;

    InternalParseToken(&reader, startingState, 0);

    if (reader.m_lastAcceptingState) {
        outToken.mTokenType = reader.m_lastAcceptingState->m_tokenType;
    }
    outToken.mText   = reader.m_stream;
    outToken.mLength = reader.m_lastAcceptingPosition - reader.m_stream;
}

//=========================================================
// Public Methods
//

//=========================================================
DfaState * AddState (int acceptingToken) {
    assert(s_stateIndex < MAX_STATES);
    if (s_states == nullptr) {
        static DfaState state_pool[MAX_STATES];
        s_states = state_pool;
    }

    DfaState * state = &s_states[s_stateIndex++];
    state->m_accepting = acceptingToken != NOT_ACCEPTING;
    state->m_tokenType = (TokenType::Enum)acceptingToken;
    state->m_edgeCount = 0;

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
    DfaState * newRoot = nullptr;
    InternalDeleteStateAndChildren(root, newRoot);

    // see if we get back to the beginning of our pool
    assert(newRoot == s_states);
    s_stateIndex = 0;
}

//=========================================================
void ReadLanguageToken(DfaState * startingState, const char * stream, Token & outToken) {
    InternalReadToken(startingState, stream, outToken);
}

//=========================================================
DfaState* CreateLanguageDfa() {
    DfaState * root = AddState(NOT_ACCEPTING);
    DfaState * whiteSpace = AddState(TokenType::Whitespace);
    DfaState * identifier = AddState(TokenType::Identifier);

    // root to whitespace
    InternalAddEdge(root, whiteSpace, 0, WhiteSpaceEdgeCondition);

    // define whitespace edges
    InternalAddEdge(whiteSpace, whiteSpace, 0, WhiteSpaceEdgeCondition);

    // root to identifier
    InternalAddEdge(root, identifier, 0, AlphaEdgeCondition);
    InternalAddEdge(root, identifier, '_', nullptr);

    // define identifier edges
    InternalAddEdge(identifier, identifier, 0, AlphaEdgeCondition);
    InternalAddEdge(identifier, identifier, '_', nullptr);
    InternalAddEdge(identifier, identifier, 0, IntegerEdgeCondition);

    return root;
}
