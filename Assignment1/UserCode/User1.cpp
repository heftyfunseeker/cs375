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
static DfaState * s_states = nullptr;
static int s_stateIndex    = 0;

//=========================================================
struct DfaEdge {
    DfaState * m_state;
    char m_condition;
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
static void InternalAddEdge(DfaState * from, DfaState * to, char c) {
    assert(from->m_edgeCount < MAX_EDGES_PER_STATE);
    DfaEdge * edge; 
    if (c == DEFAULT_EDGE_CONDITION) {
        edge = &from->m_defaultEdge;
    }
    else {
        edge = &from->m_edges[from->m_edgeCount++];
    }
    edge->m_condition = c;
    edge->m_state = to;
}

//=========================================================
static void InternalResetEdge (DfaEdge * edge) {
    edge->m_condition = 0;
    edge->m_state = nullptr;
}

//=========================================================
static void InternalDeleteStateAndChildren (DfaState * state, DfaState *& newRoot) {
    if (state->m_edgeCount == 0) {
        state->m_accepting = false;
        state->m_tokenType = (TokenType::Enum) 0;
    }
    else {
        while (state->m_edgeCount > 0) {
            DfaEdge * edge = &state->m_edges[--state->m_edgeCount];
            if (edge->m_state != state) {
                InternalDeleteStateAndChildren(edge->m_state, newRoot);
            }
            InternalResetEdge(edge);
        }
    }
    if (state->m_defaultEdge.m_state) {
        if (state->m_defaultEdge.m_state != state) {
            InternalDeleteStateAndChildren(state->m_defaultEdge.m_state, newRoot);
        }
        InternalResetEdge(&state->m_defaultEdge);
    }
    newRoot = state;
}

//=========================================================
static void InternalReadToken (DfaTokenReader * reader, DfaState * state, int streamOffset) {
    if (*(reader->m_stream + streamOffset) == '\0') {
        if (!reader->m_lastAcceptingPosition) {
            reader->m_lastAcceptingPosition = reader->m_stream + streamOffset;
        }
        return;
    }

    // save position if this state is accepting
    if (state->m_accepting) {
        reader->m_lastAcceptingState = state;
        reader->m_lastAcceptingPosition = reader->m_stream + streamOffset;
    }

    // look at outgoing edges to find next match
    for (int edgeIndex = 0; edgeIndex < state->m_edgeCount; ++edgeIndex) {
        DfaEdge * edge = &state->m_edges[edgeIndex];
        if (edge->m_condition == *(reader->m_stream + streamOffset)) {
            InternalReadToken(reader, edge->m_state, streamOffset + 1);
        }
    }

    // if we didn't find an edge, see if we have a default edge and check that
    if (!reader->m_lastAcceptingState && state->m_defaultEdge.m_state) {
        assert(state->m_defaultEdge.m_condition == DEFAULT_EDGE_CONDITION);
        InternalReadToken(reader, state->m_defaultEdge.m_state, streamOffset + 1);
    }

    if (!reader->m_lastAcceptingPosition) {
        reader->m_lastAcceptingPosition = reader->m_stream + streamOffset;
    }
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
    InternalAddEdge(from, to, c);
}

//=========================================================
void AddDefaultEdge (DfaState * from, DfaState * to) {
    assert(from && to);
    InternalAddEdge(from, to, DEFAULT_EDGE_CONDITION);
}

//=========================================================
void ReadToken(DfaState * startingState, const char * stream, Token & outToken) {
    outToken.mTokenType = 0;

    DfaTokenReader reader;
    reader.m_stream                = stream;
    reader.m_lastAcceptingState    = nullptr;
    reader.m_lastAcceptingPosition = nullptr;

    InternalReadToken(&reader, startingState, 0);

    if (reader.m_lastAcceptingState) {
        outToken.mTokenType = reader.m_lastAcceptingState->m_tokenType;
    }
    outToken.mText   = reader.m_stream;
    outToken.mLength = reader.m_lastAcceptingPosition - reader.m_stream;
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

}

//=========================================================
DfaState* CreateLanguageDfa()
{
  return nullptr;
}
