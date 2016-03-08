/******************************************************************\
 * Author:
 * Copyright 2015, DigiPen Institute of Technology
\******************************************************************/
#include "../Drivers/Driver3.hpp"
#include <algorithm>
#include <functional>

class Visitor {
public:
    enum Result {
        Stop,
        Continue
    };
    virtual Result Visit(AbstractNode* node) { return Continue; }
    virtual Result Visit(ClassNode*      node) { return this->Visit((AbstractNode*)node); }
    virtual Result Visit(VariableNode*   node) { return this->Visit((MemberAccessNode*)node); }
    virtual Result Visit(TypeNode*       node) { return this->Visit((AbstractNode*)node); }
    virtual Result Visit(NamedTypeNode* node) { return this->Visit((TypeNode*)node); }
    virtual Result Visit(StatementNode*  node) { return this->Visit((AbstractNode*)node); }
    virtual Result Visit(ExpressionNode* node) { return this->Visit((StatementNode*)node); }
    virtual Result Visit(LiteralNode*    node) { return this->Visit((ExpressionNode*)node); }
    virtual Result Visit(BinaryOperatorNode* node) { return this->Visit((ExpressionNode*)node); }
    virtual Result Visit(MemberAccessNode*   node) { return this->Visit((ExpressionNode*)node); }
    virtual Result Visit(NameReferenceNode* node) { return this->Visit((ExpressionNode*)node); }
    virtual Result Visit(FunctionNode*   node) { return this->Visit((MemberAccessNode*)node); }
    virtual Result Visit(ParameterNode*  node) { return this->Visit((AbstractNode*)node); }
    virtual Result Visit(UnaryOperatorNode* node) { return this->Visit((ExpressionNode*)node); }
    virtual Result Visit(IndexNode* node) { return this->Visit((PostExpressionNode*)node); }
    virtual Result Visit(CallNode* node) { return this->Visit((PostExpressionNode*)node); }
    virtual Result Visit(CastNode* node) { return this->Visit((PostExpressionNode*)node); }
    virtual Result Visit(BlockNode* node) { return this->Visit((AbstractNode*)node); }
    virtual Result Visit(PointerTypeNode* node) { return this->Visit((TypeNode*)node); }
    virtual Result Visit(ScopeNode* node) { return this->Visit((StatementNode*)node); }
    virtual Result Visit(WhileNode* node) { return this->Visit((StatementNode*)node); }
    virtual Result Visit(ForNode* node) { return this->Visit((StatementNode*)node); }
    virtual Result Visit(IfNode* node) { return this->Visit((StatementNode*)node); }
    virtual Result Visit(FunctionTypeNode* node) { return this->Visit((StatementNode*)node); }
    virtual Result Visit(ReferenceTypeNode* node) { return this->Visit((StatementNode*)node); }

    virtual Result Visit(ReturnNode* node) { return this->Visit((StatementNode*)node); }
    virtual Result Visit(GotoNode* node) { return this->Visit((StatementNode*)node); }
    virtual Result Visit(LabelNode* node) { return this->Visit((StatementNode*)node); }
    virtual Result Visit(BreakNode* node) { return this->Visit((StatementNode*)node); }
    virtual Result Visit(ContinueNode* node) { return this->Visit((StatementNode*)node); }
};

class VisitorPrinter : public Visitor {
public:
    //=========================================================
    Visitor::Result Visit(ReturnNode* node) {
        NodePrinter printer;
        printer << "ReturnNode";
        node->mReturnValue->Walk(this, false);
        return Visitor::Stop;
    }

    //=========================================================
    Visitor::Result Visit(GotoNode* node) {
        NodePrinter printer;
        printer << "GotoNode(" << node->mName << ")";
        return Visitor::Stop;
    }

    //=========================================================
    Visitor::Result Visit(LabelNode* node) {
        NodePrinter printer;
        printer << "LabelNode(" << node->mName << ")";
        return Visitor::Stop;
    }

    //=========================================================
    Visitor::Result Visit(BreakNode* node) {
        NodePrinter printer;
        printer << "BreakNode";
        return Visitor::Stop;
    }

    //=========================================================
    Visitor::Result Visit(ContinueNode* node) {
        NodePrinter printer;
        printer << "ContinueNode";
        return Visitor::Stop;
    }

    //=========================================================
    Visitor::Result Visit(LiteralNode* node) {
        NodePrinter printer;
        printer << "LiteralNode(" << node->mToken.str() << ")";
        return Visitor::Stop;
    }

    //=========================================================
    Visitor::Result Visit(BinaryOperatorNode* node) {
        NodePrinter printer;
        printer << "BinaryOperatorNode(" << node->mOperator.str() << ")";
        node->mLeft->Walk(this, false);
        node->mRight->Walk(this, false);
        return Visitor::Stop;
    }

    //=========================================================
    Visitor::Result Visit(NameReferenceNode* node) {
        NodePrinter printer;
        printer << "NameReferenceNode(" << node->mName.str() << ")";
        return Visitor::Stop;
    }

    //=========================================================
    Visitor::Result Visit(UnaryOperatorNode* node) {
        NodePrinter printer;
        printer << "UnaryOperatorNode(" << node->mOperator << ")";
        node->mRight->Walk(this, false);
        return Visitor::Stop;
    }

    //=========================================================
    Visitor::Result Visit(NamedTypeNode* node) {
        NodePrinter printer;
        printer << "NamedTypeNode(" << node->mName << ")";
        return Visitor::Stop;
    }

    //=========================================================
    Visitor::Result Visit(IndexNode* node) {
        NodePrinter printer;
        printer << "IndexNode";
        node->mLeft->Walk(this, false);
        node->mIndex->Walk(this, false);
        return Visitor::Stop;
    }

    //=========================================================
    Visitor::Result Visit(CastNode* node) {
        NodePrinter printer;
        printer << "CastNode";
        node->mLeft->Walk(this, false);
        node->mType->Walk(this, false);
        return Visitor::Stop;
    }

    //=========================================================
    Visitor::Result Visit(CallNode* node) {
        NodePrinter printer;
        printer << "CallNode";
        node->mLeft->Walk(this, false);
        for (auto curr = node->mArguments.begin(); curr != node->mArguments.end(); ++curr) {
            curr->get()->Walk(this, false);
        }
        return Visitor::Stop;
    }

    //=========================================================
    Visitor::Result Visit(MemberAccessNode* node) {
        NodePrinter printer;
        printer << "MemberAccessNode(" << node->mOperator << ", " << node->mName << ")";
        node->mLeft->Walk(this, false);
        return Visitor::Stop;
    }

    //=========================================================
    Visitor::Result Visit(BlockNode* node) {
        NodePrinter printer;
        printer << "BlockNode";
        for (auto& global : node->mGlobals) {
            global->Walk(this);
        }
        return Visitor::Stop;
    }

    //=========================================================
    Visitor::Result Visit(FunctionNode* node) {
        NodePrinter printer;
        printer << "FunctionNode(" << node->mName << ")";
        for (auto& param : node->mParameters) {
            param->Walk(this, false);
        }
        if (node->mReturnType) {
            node->mReturnType->Walk(this, false);
        }
        if (node->mScope) {
            node->mScope->Walk(this, false);
        }
        return Visitor::Stop;
    }

    //=========================================================
    Visitor::Result Visit(ParameterNode* node) {
        NodePrinter printer;
        printer << "ParameterNode(" << node->mName << ")";
        if (node->mInitialValue) {
            node->mInitialValue->Walk(this, false);
        }
        node->mType->Walk(this, false);
        return Visitor::Stop;
    }

    //=========================================================
    Visitor::Result Visit(PointerTypeNode* node) {
        NodePrinter printer;
        printer << "PointerTypeNode";
        node->mPointerTo->Walk(this, false);
        return Visitor::Stop;
    }

    //=========================================================
    Visitor::Result Visit(ScopeNode* node) {
        NodePrinter printer;
        printer << "ScopeNode";
        for (auto& statement : node->mStatements) {
            statement->Walk(this, false);
        }
        return Visitor::Stop;
    }
    
    //=========================================================
    Visitor::Result Visit(ClassNode* node) {
        NodePrinter printer;
        printer << "ClassNode(" << node->mName << ")";
        for (auto& member : node->mMembers) {
            member->Walk(this, false);
        }
        return Visitor::Stop;
    }

    //=========================================================
    Visitor::Result Visit(WhileNode* node) {
        NodePrinter printer;
        printer << "WhileNode";
        node->mCondition->Walk(this, false);
        node->mScope->Walk(this, false);
        return Visitor::Stop;
    }

    //=========================================================
    Visitor::Result Visit(StatementNode* node) {
        NodePrinter printer;
        printer << "StatementNode";
        return Visitor::Stop;
    }

    //=========================================================
    Visitor::Result Visit(VariableNode* node) {
        NodePrinter printer;
        printer << "VariableNode(" << node->mName << ")";
        node->mType->Walk(this, false);
        if (node->mInitialValue) {
            node->mInitialValue->Walk(this, false);
        }
        return Visitor::Stop;
    }

    //=========================================================
    Visitor::Result Visit(ForNode* node) {
        NodePrinter printer;
        printer << "ForNode";
        if (node->mInitialVariable) {
            node->mInitialVariable->Walk(this, false);
        }
        if (node->mInitialExpression) {
            node->mInitialExpression->Walk(this, false);
        }
        if (node->mCondition) {
            node->mCondition->Walk(this, false);
        }

        if (node->mScope) {
            node->mScope->Walk(this, false);
        }
        
        if (node->mIterator) {
            node->mIterator->Walk(this, false);
        }
        return Visitor::Stop;
    }

    //=========================================================
    Visitor::Result Visit(IfNode* node) {
        NodePrinter printer;
        printer << "IfNode";
        if (node->mCondition) {
            node->mCondition->Walk(this, false);
        }
        if (node->mScope) {
            node->mScope->Walk(this, false);
        }
        if (node->mElse) {
            node->mElse->Walk(this, false);
        }
        return Visitor::Stop;
    }

    //=========================================================
    Visitor::Result Visit(FunctionTypeNode* node) {
        NodePrinter printer;
        printer << "FunctionTypeNode";
        for (auto& param : node->mParameters) {
            param->Walk(this, false);
        }
        node->mReturn->Walk(this, false);
        return Visitor::Stop;
    }

    //=========================================================
    Visitor::Result Visit(ReferenceTypeNode* node) {
        NodePrinter printer;
        printer << "ReferenceTypeNode";
        node->mReferenceTo->Walk(this, false);
        return Visitor::Stop;
    }
};

//=========================================================
void BreakNode::Walk(Visitor* visitor, bool visit) {
    visitor->Visit(this);
}

//=========================================================
void ContinueNode::Walk(Visitor* visitor, bool visit) {
    visitor->Visit(this);
}

//=========================================================
void LabelNode::Walk(Visitor* visitor, bool visit) {
    visitor->Visit(this);
}

//=========================================================
void ReturnNode::Walk(Visitor* visitor, bool visit) {
    visitor->Visit(this);
}

//=========================================================
void IfNode::Walk(Visitor* visitor, bool visit) {
    visitor->Visit(this);
}

//=========================================================
void ForNode::Walk(Visitor* visitor, bool visit) {
    visitor->Visit(this);
}

//=========================================================
void ClassNode::Walk(Visitor* visitor, bool visit) {
    visitor->Visit(this);
}

//=========================================================
void ExpressionNode::Walk(Visitor* visitor, bool visit) {
    visitor->Visit(this);
}

//=========================================================
void GotoNode::Walk(Visitor* visitor, bool visit) {
    visitor->Visit(this);
}

void MemberAccessNode::Walk(Visitor* visitor, bool visit) {
    visitor->Visit(this);
}

//=========================================================
void PostExpressionNode::Walk(Visitor* visitor, bool visit) {
    visitor->Visit(this);
}

//=========================================================
void TypeNode::Walk(Visitor* visitor, bool visit) {
    visitor->Visit(this);
}

//=========================================================
void VariableNode::Walk(Visitor* visitor, bool visit) {
    visitor->Visit(this);
}

//=========================================================
void FunctionNode::Walk(Visitor* visitor, bool visit) {
    visitor->Visit(this);
}

//=========================================================
void LiteralNode::Walk(Visitor* visitor, bool visit) {
    visitor->Visit(this);
}

//=========================================================
void BinaryOperatorNode::Walk(Visitor* visitor, bool visit) {
    if (visitor && visitor->Visit(this) == false) {
        return;
    }
    mLeft->Walk(visitor);
    mRight->Walk(visitor);
}

//=========================================================
void NameReferenceNode::Walk(Visitor* visitor, bool visit) {
    visitor->Visit(this);
}

//=========================================================
void UnaryOperatorNode::Walk(Visitor* visitor, bool visit) {
    if (visitor && visitor->Visit(this) == false) {
        return;
    }
    mRight->Walk(visitor);
}

//=========================================================
void NamedTypeNode::Walk(Visitor* visitor, bool visit) {
    visitor->Visit(this);
}

//=========================================================
void FunctionTypeNode::Walk(Visitor* visitor, bool visit) {
    visitor->Visit(this);
}

//=========================================================
void CastNode::Walk(Visitor* visitor, bool visit) {
    visitor->Visit(this);
}

//=========================================================
void CallNode::Walk(Visitor* visitor, bool visit) {
    visitor->Visit(this);
}

//=========================================================
void IndexNode::Walk(Visitor* visitor, bool visit) {
    visitor->Visit(this);
}

//=========================================================
void BlockNode::Walk(Visitor* visitor, bool visit) {
    visitor->Visit(this);
}

//=========================================================
void ParameterNode::Walk(Visitor* visitor, bool visit) {
    visitor->Visit(this);
}

//=========================================================
void PointerTypeNode::Walk(Visitor* visitor, bool visit) {
    visitor->Visit(this);
}

//=========================================================
void ReferenceTypeNode::Walk(Visitor* visitor, bool visit) {
    visitor->Visit(this);
}

//=========================================================
void StatementNode::Walk(Visitor* visitor, bool visit) {
    visitor->Visit(this);
}

//=========================================================
void ScopeNode::Walk(Visitor* visitor, bool visit) {
    visitor->Visit(this);
}

//=========================================================
void WhileNode::Walk(Visitor* visitor, bool visit) {
    visitor->Visit(this);
}

//=========================================================
class Parser {
public:
    Parser  (const std::vector<Token> & tokenStream);
    ~Parser ();
    void Parse ();

    std::unique_ptr<BlockNode> Block ();
    std::unique_ptr<ClassNode> Class ();
    std::unique_ptr<FunctionNode> Function ();
    std::unique_ptr<VariableNode> Var ();

    std::unique_ptr<ExpressionNode> GroupedExpression ();
    std::unique_ptr<LiteralNode> Literal ();
    std::unique_ptr<NameReferenceNode> NameReference ();
    std::unique_ptr<ExpressionNode> Value ();


    std::unique_ptr<ExpressionNode> ParseBinaryExpressionHelper(
        std::unique_ptr<ExpressionNode>&node,
        std::function<bool()> acceptPredicate,
        std::unique_ptr<ExpressionNode> (Parser::* expressionFunc)()
    );

    std::unique_ptr<ExpressionNode> Expression ();
    std::unique_ptr<ExpressionNode> Expression1 ();
    std::unique_ptr<ExpressionNode> Expression2 ();
    std::unique_ptr<ExpressionNode> Expression3 ();
    std::unique_ptr<ExpressionNode> Expression4 ();
    std::unique_ptr<ExpressionNode> Expression5 ();
    std::unique_ptr<ExpressionNode> Expression6 ();
    std::unique_ptr<ExpressionNode> Expression7 ();

    std::unique_ptr<TypeNode> SpecifiedType ();
    std::unique_ptr<TypeNode> Type ();
    std::unique_ptr<TypeNode> NamedType ();
    std::unique_ptr<TypeNode> FunctionType ();

    std::unique_ptr<ParameterNode> Parameter ();
    std::unique_ptr<ScopeNode> Scope ();

    std::unique_ptr<StatementNode> Statement ();
    std::unique_ptr<StatementNode> FreeStatement ();
    std::unique_ptr<StatementNode> DelimitedStatement ();

    std::unique_ptr<IfNode> If ();
    std::unique_ptr<IfNode> Else ();
    std::unique_ptr<WhileNode> While ();
    std::unique_ptr<ForNode> For ();

    std::unique_ptr<LabelNode> Label ();
    std::unique_ptr<GotoNode> Goto ();
    std::unique_ptr<ReturnNode> Return ();

    std::unique_ptr<MemberAccessNode> MemberAccess ();
    std::unique_ptr<CallNode> Call ();
    std::unique_ptr<CastNode> Cast ();
    std::unique_ptr<IndexNode> Index ();

    bool Accept (TokenType::Enum tokenType);
    bool Expect (TokenType::Enum tokenType);
    void GetLastAcceptedToken(Token* outToken);

    const std::vector<Token>& m_tokenStream;
    int m_streamCursor;
};

//=========================================================
Parser::Parser (const std::vector<Token> & tokenStream) :
    m_tokenStream(tokenStream),
    m_streamCursor(0)
{}

//=========================================================
Parser::~Parser ()
{}

//=========================================================
void Parser::Parse () {
    m_streamCursor = 0;
    Block ();
    // if we didn't consume the whole stream
    if (m_streamCursor != m_tokenStream.size()) {
        throw ParsingException("Stream has remaining tokens");
    }
}

//=========================================================
bool Parser::Accept (TokenType::Enum tokenType) {
    if (m_streamCursor >= (int)m_tokenStream.size()) {
        return false;
    }
    Token token = m_tokenStream[m_streamCursor];
    bool match = token.mEnumTokenType == tokenType;
    if (match) {
        PrintRule::AcceptedToken(token);
        ++m_streamCursor;
    }
    return match;
}

//=========================================================
bool Parser::Expect (TokenType::Enum tokenType) {
    if (!Accept(tokenType)) {
        throw ParsingException("failed parsing");
    }
    return true;
}

void Parser::GetLastAcceptedToken(Token* outToken)
{
    *outToken = m_tokenStream[m_streamCursor - 1];
}

//=========================================================
std::unique_ptr<BlockNode> Parser::Block () {
    PrintRule rule("Block");
    auto node = std::make_unique<BlockNode>();
    while (
       (node->mGlobals.push_back(Function())) || 
       (node->mGlobals.push_back(Class())) ||
       (node->mGlobals.push_back(Var()) && Expect(TokenType::Semicolon))
    );
    return rule.Accept(std::move(node));
}

//=========================================================
std::unique_ptr<ClassNode> Parser::Class () {
    PrintRule rule("Class");
    if (Accept(TokenType::Class) == false) {
        return false;
    }
    auto node = std::make_unique<ClassNode>();
    Expect(TokenType::Identifier);
    GetLastAcceptedToken(&node->mName);
    
    Expect(TokenType::OpenCurley);
    while (
        ((node->mMembers.push_back(Var()) && Expect(TokenType::Semicolon))) ||
        (node->mMembers.push_back(Function()))
    );
    Expect(TokenType::CloseCurley);

    return rule.Accept(std::move(node));
}

//=========================================================
std::unique_ptr<FunctionNode> Parser::Function () {
    PrintRule rule("Function");
    if (Accept(TokenType::Function) == false) {
        return false;
    }
    auto node = std::make_unique<FunctionNode>(); 
    Expect(TokenType::Identifier);
    GetLastAcceptedToken(&node->mName);
    
    Expect(TokenType::OpenParentheses);

    if (node->mParameters.push_back(Parameter())) {
        while (Accept(TokenType::Comma)) {
            if (!node->mParameters.push_back(Parameter())) {
                throw ParsingException("Expected parameter after comma in function sig");
            }
        }
    }

    Expect(TokenType::CloseParentheses);

    node->mReturnType = SpecifiedType();

    if (!(node->mScope = Scope())) {
        throw ParsingException("Expected scope after function");
    }

    return rule.Accept(std::move(node));
}

//=========================================================
std::unique_ptr<VariableNode> Parser::Var () {
    PrintRule rule("Var");

    if (Accept(TokenType::Var) == false) {
        return false;
    }
    Expect(TokenType::Identifier);
    auto node = std::make_unique<VariableNode>();
    GetLastAcceptedToken(&node->mName);
    if (!(node->mType = SpecifiedType())) {
        throw ParsingException("No specified type provided on variable");
    }

    if (Accept(TokenType::Assignment)) {
        if (!(node->mInitialValue = Expression())) {
            throw ParsingException(
                "expected expression on right side of var assignment");
        }
    }

    return rule.Accept(std::move(node));
}

//=========================================================
std::unique_ptr<TypeNode> Parser::SpecifiedType () {
    PrintRule rule("SpecifiedType");
    if (Accept(TokenType::Colon) == false) {
        return false;
    }
    auto node = Type();
    if (!node) {
        throw ParsingException("Expected type in SpecifiedType");
    }
    return rule.Accept(std::move(node));
}

//=========================================================
std::unique_ptr<TypeNode> Parser::Type () {
    PrintRule rule("Type");
    std::unique_ptr<TypeNode> node;
    if ((node = NamedType()) || (node = FunctionType())) {
        return rule.Accept(std::move(node));
    }
    return false;
}

//=========================================================
std::unique_ptr<TypeNode> Parser::NamedType () {
    PrintRule rule("NamedType");
    if (Accept(TokenType::Identifier) == false) {
        return false;
    }
    auto node = std::make_unique<NamedTypeNode>();
    GetLastAcceptedToken(&node->mName);

    std::unique_ptr<PointerTypeNode> pNode;
    PointerTypeNode* curr = nullptr;
    while (Accept(TokenType::Asterisk)) {
        if (curr) {
            curr->mPointerTo = std::make_unique<PointerTypeNode>();
            curr = (PointerTypeNode*)&curr->mPointerTo;
        }
        else {
            pNode = std::make_unique<PointerTypeNode>();
            curr = pNode.get();
        }
    }
    if (curr) {
        curr->mPointerTo = std::move(node);
    }
   
    if (Accept(TokenType::Ampersand)) {
        auto refNode = std::make_unique<ReferenceTypeNode>();
        if (pNode) {
            refNode->mReferenceTo = std::move(pNode);
        }
        else {
            refNode->mReferenceTo = std::move(node);
        }
        return rule.Accept(std::move(refNode));
    }

    if (pNode) {
        return rule.Accept(std::move(pNode));
    }

    return rule.Accept(std::move(node));
}

//=========================================================
std::unique_ptr<TypeNode> Parser::FunctionType () {
    PrintRule rule("FunctionType");
    if (Accept(TokenType::Function) == false) {
        return false;
    }
    Expect(TokenType::Asterisk);
    auto pRootNode = std::make_unique<PointerTypeNode>();
    std::unique_ptr<PointerTypeNode> pNode;
    PointerTypeNode* curr = nullptr;
    while (Accept(TokenType::Asterisk)) {
        if (curr) {
            curr->mPointerTo = std::make_unique<PointerTypeNode>();
            curr = (PointerTypeNode*)&curr->mPointerTo;
        }
        else {
            pNode = std::make_unique<PointerTypeNode>();
            curr = pNode.get();
        }
    }

    std::unique_ptr<ReferenceTypeNode> refNode;
    if (Accept(TokenType::Ampersand)) {
        refNode = std::make_unique<ReferenceTypeNode>();
    }

    Expect(TokenType::OpenParentheses);

    auto funcTypeNode = std::make_unique<FunctionTypeNode>();
    if (!funcTypeNode->mParameters.push_back(Type())) {
        throw ParsingException("Expected type in function type");
    }
    while (Accept(TokenType::Comma)) {
        if (!funcTypeNode->mParameters.push_back(Type())) {
            throw ParsingException(
                "Expected Type after comma in function type");
        }
    }
    Expect(TokenType::CloseParentheses);
    funcTypeNode->mReturn = SpecifiedType();
    if (pNode) {
        if (refNode) {
            refNode->mReferenceTo = std::move(funcTypeNode);
            curr->mPointerTo = std::move(refNode);
        }
        else {
            curr->mPointerTo = std::move(funcTypeNode);
        }
        pRootNode->mPointerTo = std::move(pNode);
    }
    else if (refNode) {
        refNode->mReferenceTo = std::move(funcTypeNode);
        pRootNode->mPointerTo = std::move(refNode);
    }
    else {
        pRootNode->mPointerTo = std::move(funcTypeNode);
    }
    return rule.Accept(std::move(pRootNode));
}

//=========================================================
std::unique_ptr<ParameterNode> Parser::Parameter () {
    PrintRule rule ("Parameter");
    if (Accept(TokenType::Identifier)) {
        auto node = std::make_unique<ParameterNode>();
        GetLastAcceptedToken(&node->mName);
        node->mType = SpecifiedType();
        node->mInitialValue = Expression();
        return rule.Accept(std::move(node));
    }
    return false;
}

//=========================================================
std::unique_ptr<ScopeNode> Parser::Scope () {
    PrintRule rule("Scope");
    if (Accept(TokenType::OpenCurley) == false) {
        return false;
    }
    auto node = std::make_unique<ScopeNode>();
    while (node->mStatements.push_back(Statement()));
    Expect(TokenType::CloseCurley);
    return rule.Accept(std::move(node));
}

//=========================================================
std::unique_ptr<StatementNode> Parser::Statement () {
    PrintRule rule("Statement");
    std::unique_ptr<StatementNode> node;
    if (
        (node = FreeStatement())
    ) {
        return rule.Accept(std::move(node));
    }
    if (((node = DelimitedStatement()) && Expect(TokenType::Semicolon))) {
        return rule.Accept(std::move(node));
    }
    return false;
}

//=========================================================
std::unique_ptr<StatementNode> Parser::FreeStatement () {
    PrintRule rule("FreeStatement");
    std::unique_ptr<StatementNode> node;
    if (
        (node = While()) ||
        (node = For()) ||
        (node = If())
    ) {
        return rule.Accept(std::move(node));
    }
    return false;
}

//=========================================================
std::unique_ptr<StatementNode> Parser::DelimitedStatement () {
    PrintRule rule("DelimitedStatement");
    std::unique_ptr<StatementNode> node;
    if ((node = Var()) ||
        (node = Expression()) ||
        (node = Label()) ||
        (node = Goto()) ||
        (node = Return()) ||
        Accept(TokenType::Break) ||
        Accept(TokenType::Continue)
    ) {
        if (!node) {
            Token t;
            GetLastAcceptedToken(&t);
            if (t.mEnumTokenType == TokenType::Break) {
                node = std::make_unique<BreakNode>();
            }
            else {
                node = std::make_unique<ContinueNode>();
            }
        }
        return rule.Accept(std::move(node));
    }
    return false;
}

//=========================================================
std::unique_ptr<IfNode> Parser::If () {
    PrintRule rule("If");
    if (Accept(TokenType::If) == false) {
        return false;
    }
    auto node = std::make_unique<IfNode>();
    if (!(node->mCondition = GroupedExpression()) || !(node->mScope = Scope())) {
        throw ParsingException("expected group expression for if");
    }
    node->mElse = Else ();
    return rule.Accept(std::move(node));
}

//=========================================================
std::unique_ptr<IfNode> Parser::Else () {
    PrintRule rule("Else");
    if (Accept(TokenType::Else) == false) {
        return false;
    }
    auto node = If();
    if (!node) {
        node = std::make_unique<IfNode>();
        node->mScope = Scope();
        if (!node->mScope) {
            throw ParsingException("Failed to parse Else");
        }
    }
    return rule.Accept(std::move(node));
}

//=========================================================
std::unique_ptr<WhileNode> Parser::While () {
    PrintRule rule("While");
    if (Accept(TokenType::While) == false) {
        return false;
    }
    auto node = std::make_unique<WhileNode>();
    if (!(node->mCondition = GroupedExpression())) {
        throw ParsingException("Expected grouped expression in while");
    }
    if (!(node->mScope = Scope())) {
        throw ParsingException("Expected scope in while");
    }
    return rule.Accept(std::move(node));
}

//=========================================================
std::unique_ptr<ForNode> Parser::For () {
    PrintRule rule("For");
    if (Accept(TokenType::For) == false) {
        return false;
    }
    auto node = std::make_unique<ForNode>();
    Expect(TokenType::OpenParentheses);
    node->mInitialVariable = Var();
    if (!node->mInitialVariable) {
        node->mInitialExpression = Expression();
    }
    Expect(TokenType::Semicolon);
    node->mCondition = Expression();
    Expect(TokenType::Semicolon);
    node->mIterator = Expression();
    Expect(TokenType::CloseParentheses);
    if (!(node->mScope = Scope())) {
        throw ParsingException("Expected scope for For block");
    }
    return rule.Accept(std::move(node));
}

//=========================================================
std::unique_ptr<LabelNode> Parser::Label () {
    PrintRule rule("Label");
    if (Accept(TokenType::Label) == false) {
        return false;
    }
    Expect(TokenType::Identifier);
    auto node = std::make_unique<LabelNode>();
    GetLastAcceptedToken(&node->mName);
    return rule.Accept(std::move(node));
}

//=========================================================
std::unique_ptr<GotoNode> Parser::Goto () {
    PrintRule rule("Goto");
    if (Accept(TokenType::Goto) == false) {
        return false;
    }
    Expect(TokenType::Identifier);
    auto node = std::make_unique<GotoNode>();
    GetLastAcceptedToken(&node->mName);
    return rule.Accept(std::move(node));
}

//=========================================================
std::unique_ptr<ReturnNode> Parser::Return () {
    PrintRule rule("Return");
    if (Accept(TokenType::Return) == false) {
        return false;
    }
    auto node = std::make_unique<ReturnNode>();
    node->mReturnValue = Expression();
    return rule.Accept(std::move(node));
}

//=========================================================
std::unique_ptr<MemberAccessNode> Parser::MemberAccess () {
    PrintRule rule("MemberAccess");
    if (!Accept(TokenType::Dot) && !Accept(TokenType::Arrow)) {
        return false;
    }
    auto node = std::make_unique<MemberAccessNode>();
    GetLastAcceptedToken(&node->mOperator);
    Expect(TokenType::Identifier);
    GetLastAcceptedToken(&node->mName);
    return rule.Accept(std::move(node));
}

//=========================================================
std::unique_ptr<CallNode> Parser::Call () {
    PrintRule rule("Call");
    if (Accept(TokenType::OpenParentheses) == false) {
        return false;
    }
    auto node = std::make_unique<CallNode>();
    if (node->mArguments.push_back(Expression())) {
        while (Accept(TokenType::Comma)) {
            if (!node->mArguments.push_back(Expression())) {
                throw ParsingException(
                    "Expected expression after comma in func call"
                );
            }
        }
    }
    Expect(TokenType::CloseParentheses);
    return rule.Accept(std::move(node));
}

//=========================================================
std::unique_ptr<CastNode> Parser::Cast () {
    PrintRule rule("Cast");
    if (Accept(TokenType::As) == false) {
        return false;
    }
    auto typeNode = Type();
    if (!typeNode) {
        throw ParsingException("Expected Type in Cast");
    }
    auto castNode = std::make_unique<CastNode>();
    castNode->mType = std::move(typeNode);
    return rule.Accept(std::move(castNode));
}

//=========================================================
std::unique_ptr<IndexNode> Parser::Index () {
    PrintRule rule("Index");
    if (Accept(TokenType::OpenBracket) == false) {
        return false;
    }
    auto node = std::make_unique<IndexNode>();
    if (!(node->mIndex = Expression())) {
        throw ParsingException("Expected expression in Indexing");
    }
    Expect(TokenType::CloseBracket);
    return rule.Accept(std::move(node));
}

//=========================================================
std::unique_ptr<ExpressionNode> Parser::ParseBinaryExpressionHelper(
    std::unique_ptr<ExpressionNode>&node,
    std::function<bool()> acceptPredicate,
    std::unique_ptr<ExpressionNode>(Parser::* expressionPredicate)()
) {
    std::unique_ptr<BinaryOperatorNode> binaryNode;
    std::unique_ptr<BinaryOperatorNode> prevBinaryNode;
    while (acceptPredicate()) {
        binaryNode = std::make_unique<BinaryOperatorNode>();
        GetLastAcceptedToken(&binaryNode->mOperator);
        if (prevBinaryNode) {
            binaryNode->mLeft = std::move(prevBinaryNode);
        }
        else {
            binaryNode->mLeft = std::move(node);
        }
        binaryNode->mRight = (this->*expressionPredicate)();
        if (!binaryNode->mRight) {
            throw ParsingException("Failed Parsing Expression 5");
        }
        prevBinaryNode = std::move(binaryNode);
    }
    if (prevBinaryNode) {
        return std::move(prevBinaryNode);
    }
    return std::move(node);
}

//=========================================================
std::unique_ptr<ExpressionNode> Parser::Expression () {
    PrintRule rule("Expression");
    auto node = Expression1();
    if (!node) {
        return nullptr;
    }

    if (
        Accept(TokenType::Assignment)         ||
        Accept(TokenType::AssignmentPlus)     ||
        Accept(TokenType::AssignmentMinus)    ||
        Accept(TokenType::AssignmentMultiply) ||
        Accept(TokenType::AssignmentDivide)   ||
        Accept(TokenType::AssignmentModulo)
    ) {
        auto binaryOperator = std::make_unique<BinaryOperatorNode>();
        GetLastAcceptedToken(&binaryOperator->mOperator);
        binaryOperator->mLeft = std::move(node);
        binaryOperator->mRight = Expression();
        if (!binaryOperator->mRight) {
            throw ParsingException("Failed parsing base expression");
        }
        return rule.Accept(std::move(binaryOperator));
    }
    return rule.Accept(std::move(node));
}

//=========================================================
std::unique_ptr<ExpressionNode> Parser::Expression1 () {
    PrintRule rule("Expression1");
    auto node = Expression2();
    if (!node) {
        return nullptr;
    }
    auto accept = [&]() {
        return Accept(TokenType::LogicalOr);
    };
    return rule.Accept(ParseBinaryExpressionHelper(node, accept, &Parser::Expression2));
}

//=========================================================
std::unique_ptr<ExpressionNode> Parser::Expression2 () {
    PrintRule rule("Expression2");
    auto node = Expression3();
    if (!node) {
        return nullptr;
    }
    auto accept = [&]() {
        return Accept(TokenType::LogicalAnd);
    };
    return rule.Accept(ParseBinaryExpressionHelper(node, accept, &Parser::Expression3));
}

//=========================================================
std::unique_ptr<ExpressionNode> Parser::Expression3 () {
    PrintRule rule("Expression3");
    auto node = Expression4();
    if (!node) {
        return nullptr;
    }
    auto accept = [&]() {
        return
            Accept(TokenType::LessThan) ||
            Accept(TokenType::GreaterThan) ||
            Accept(TokenType::LessThanOrEqualTo) ||
            Accept(TokenType::GreaterThanOrEqualTo) ||
            Accept(TokenType::Equality) ||
            Accept(TokenType::Inequality);
    };
    return rule.Accept(ParseBinaryExpressionHelper(node, accept, &Parser::Expression4));
}

//=========================================================
std::unique_ptr<ExpressionNode> Parser::Expression4() {
    PrintRule rule("Expression4");
    auto node = Expression5();
    if (!node) {
        return node;
    }

    auto accept = [&]() {
        return Accept(TokenType::Plus) || Accept(TokenType::Minus);
    };
    return rule.Accept(ParseBinaryExpressionHelper(node, accept, &Parser::Expression5));
}

//=========================================================
std::unique_ptr<ExpressionNode> Parser::Expression5() {
    PrintRule rule("Expression5");
    auto node = Expression6();
    if (!node) {
        return nullptr;
    }
    auto accept = [&]() {
        return  Accept(TokenType::Asterisk) ||
                Accept(TokenType::Divide) ||
                Accept(TokenType::Modulo);
    };
    return rule.Accept(ParseBinaryExpressionHelper(node, accept, &Parser::Expression6));
}

//=========================================================
std::unique_ptr<ExpressionNode> Parser::Expression6() {
    PrintRule rule("Expression6");
    unique_vector<UnaryOperatorNode> nodes;
    int count = 0;
    while (
        Accept(TokenType::Asterisk) ||
        Accept(TokenType::Ampersand) ||
        Accept(TokenType::Plus) ||
        Accept(TokenType::Minus) ||
        Accept(TokenType::LogicalNot) ||
        Accept(TokenType::Increment) ||
        Accept(TokenType::Decrement)
    ) {
        auto nextUnaryNode = std::make_unique<UnaryOperatorNode>();
        GetLastAcceptedToken(&nextUnaryNode->mOperator);
        nodes.push_back(std::move(nextUnaryNode));
        ++count;
    }
    if (count) {
        UnaryOperatorNode* curr = nodes.back().get();
        curr->mRight = Expression7();
        curr = nodes.front().get();
        for (int i = 1; i < count; ++i) {
            curr->mRight = std::move(nodes[i]);
            curr = (UnaryOperatorNode*)curr->mRight.get();
        }
        return rule.Accept(std::move(nodes.front()));
    }
    return rule.Accept(Expression7());
}

//=========================================================
std::unique_ptr<ExpressionNode> Parser::Expression7() {
    PrintRule rule("Expression7");
    std::unique_ptr<ExpressionNode> node = Value();
    if (node == nullptr) {
        return nullptr;
    }
    unique_vector<PostExpressionNode> nodes;
    std::unique_ptr<PostExpressionNode> postNode;
    int count = 0;
    while (
        (postNode = MemberAccess()) ||
        (postNode = Call()) ||
        (postNode = Cast()) ||
        (postNode = Index())
    ) {
        nodes.push_back(std::move(postNode));
        ++count;
    }
    if (count) {
        PostExpressionNode* curr = nodes.back().get();
        for (int i = count - 2; i >= 0; --i) {
            curr->mLeft = std::move(nodes[i]);
            curr = (PostExpressionNode*)curr->mLeft.get();
        }
        curr->mLeft = std::move(node);
        return rule.Accept(std::move(nodes.back()));
    }
    return rule.Accept(std::move(node));
}

//=========================================================
std::unique_ptr<ExpressionNode> Parser::GroupedExpression() {
    PrintRule rule("GroupedExpression");
    if (Accept(TokenType::OpenParentheses) == false) {
        return false;
    }

    auto node = Expression();
    Expect(TokenType::CloseParentheses);
    return rule.Accept(std::move(node));
}

//=========================================================
std::unique_ptr<LiteralNode> Parser::Literal() {
    PrintRule rule("Literal");
    if (Accept(TokenType::True) ||
        Accept(TokenType::False) ||
        Accept(TokenType::Null) ||
        Accept(TokenType::IntegerLiteral) ||
        Accept(TokenType::FloatLiteral) ||
        Accept(TokenType::StringLiteral) ||
        Accept(TokenType::CharacterLiteral)
    ) {
        auto node = std::make_unique<LiteralNode>();
        GetLastAcceptedToken(&node->mToken);
        return rule.Accept(std::move(node));
    }
    return rule.Accept(nullptr);
}

//=========================================================
std::unique_ptr<NameReferenceNode> Parser::NameReference () {
    PrintRule rule("NameReference");
    if (Accept(TokenType::Identifier))
    {
        auto node = std::make_unique<NameReferenceNode>();
        GetLastAcceptedToken(&node->mName);
        return rule.Accept(std::move(node));
    }
    return rule.Accept(nullptr);
}

//=========================================================
std::unique_ptr<ExpressionNode> Parser::Value () {
    PrintRule rule("Value");
    std::unique_ptr<ExpressionNode> node = nullptr;
    if (
        (node = Literal()) ||
        (node = NameReference()) ||
        (node = GroupedExpression())
    );

    return rule.Accept(std::move(node));
}

//=========================================================
void RemoveWhitespaceAndComments(std::vector<Token>& tokens) {
    if (tokens.size() == 0) {
        return;
    }

    tokens.erase(
        std::remove_if(
            tokens.begin(),
            tokens.end(),
            [](Token t) {
        return (t.mEnumTokenType == TokenType::Whitespace)
            || (t.mEnumTokenType == TokenType::SingleLineComment)
            || (t.mEnumTokenType == TokenType::MultiLineComment);
    }
            ),
        tokens.end()
        );
}

//=========================================================
void Recognize(std::vector<Token>& tokens) {
    Parser parser(tokens);
    parser.Parse();
}


//=========================================================
std::unique_ptr<ExpressionNode> ParseExpression(std::vector<Token>& tokens)
{
    Parser parser(tokens);
    auto node = parser.Expression();
    return std::move(node);
}

//=========================================================
std::unique_ptr<BlockNode> ParseBlock(std::vector<Token>& tokens)
{
    Parser parser(tokens);
    auto node = parser.Block();
    return std::move(node);
}

//=========================================================
void PrintTree(AbstractNode* node)
{
    VisitorPrinter vistorPrinter;
    node->Walk(&vistorPrinter);
}