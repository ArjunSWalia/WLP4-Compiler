#include <cassert>
#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <map>
#include <algorithm>
#include <sstream>
#include <memory>
using namespace std;

struct TreeNode
{
    // nonterminal nodes ONLY
    string rule;
    string lhs;

    // terminal nodes ONLY
    string kind;
    string lexeme;

    vector<string> rhs;

    string type; // will either be int, int* or "undefined"

    vector<TreeNode *> children;
    ~TreeNode()
    {
        for (auto &c : children)
        {
            delete c;
        }
    }

    void print(string indent = "")
    {
        if (kind != "")
        {
            cout << indent << kind << " " << lexeme;
            if (type != "")
            {
                cout << " : " << type;
            }
            cout << endl;
        }
        else
        {
            cout << indent << rule;
            if (type != "")
            {
                cout << " : " << type;
            }
            cout << endl;
            for (auto i : children)
            {
                i->print(indent + ">");
            }
        }
    }
    TreeNode *getChild(string str, int n)
    {
        // nth child of the tree?
        int childcount = 0;
        for (int i = 0; i < children.size(); i++)
        {
            if (children.at(i)->lhs == str || children.at(i)->kind == str)
            {
                childcount++;
                if (childcount == n)
                {
                    return children.at(i);
                }
            }
        }
        return nullptr;
    }

    TreeNode() : rule(""), lhs(""), kind(""), lexeme(""), type("") {}
};

int LoopCounter = 1;
int IfCounter = 1;
int deleteCount = 1;
int newCount = 1;

vector<string> splitter(string str)
{
    string temp;
    istringstream stream(str);
    vector<string> RHS;
    while (getline(stream, temp, ' '))
    {
        RHS.push_back(temp);
    }
    return RHS;
}

TreeNode *populateTreeNode()
{
    string prodrule; // start BOF procedures EOF
    getline(cin, prodrule);
    TreeNode *t = new TreeNode();
    vector<string> tempvec = splitter(prodrule);
    t->lhs = tempvec.at(0);
    t->rule += t->lhs;
    for (int i = 1; i < tempvec.size(); i++)
    {
        if (tempvec.at(1) == ".EMPTY")
        {
            // cout << "HAS RUN" << endl;
            t->rhs = vector<string>();
            break;
        }
        if (tempvec.at(i) != ":")
        {
            t->rhs.push_back(tempvec.at(i));
            t->rule += " ";
            t->rule += tempvec.at(i);
        }
        else if (tempvec.at(i) == ":")
        {
            t->type = tempvec.at(i + 1);
            break;
        }
    }
    if (tempvec.at(1) != ".EMPTY")
    {
        for (int i = 0; i < t->rhs.size(); i++)
        {
            // cout << "YOU SHOULD NEVER RUN" << endl;
            if (islower(t->rhs.at(i).at(0))) // therefore it is nonterminal
            {
                t->children.push_back(populateTreeNode());
            }
            else // if it is a terminal BOF, then the next line should be BOF BOF or like INT int
            {
                string str;
                getline(cin, str);
                TreeNode *terminalleaf = new TreeNode();
                vector<string> kindlexemepair = splitter(str); // vector should be of size 2, with kind being first and lexeme being second elem
                terminalleaf->kind = kindlexemepair.at(0);
                terminalleaf->lexeme = kindlexemepair.at(1);
                t->children.push_back(terminalleaf);
                // terminal node type information is not populated as it is not needed during code generation
            }
        }
    }

    return t;
}

void add(int d, int s, int t)
{
    cout << "add $" << d << ", $" << s << ", $" << t << endl;
}
void SUB(int d, int s, int t)
{
    cout << "sub $" << d << ", $" << s << ", $" << t << endl;
}
void MULT(int s, int t)
{
    cout << "mult $" << s << ", $" << t << endl;
}
void MULTU(int s, int t)
{
    cout << "multu $" << s << ", $" << t << endl;
}
void DIV(int s, int t)
{
    cout << "div $" << s << ", $" << t << endl;
}
void DIVU(int s, int t)
{
    cout << "divu $" << s << ", $" << t << endl;
}
void MFHI(int d)
{
    cout << "mfhi $" << d << endl;
}
void MFLO(int d)
{
    cout << "mflo $" << d << endl;
}
void LIS(int d)
{
    cout << "lis $" << d << endl;
}
void SLT(int d, int s, int t)
{
    cout << "slt $" << d << ", $" << s << ", $" << t << endl;
}
void SLTU(int d, int s, int t)
{
    cout << "sltu $" << d << ", $" << s << ", $" << t << endl;
}
void JR(int s)
{
    cout << "jr $" << s << endl;
}
void JALR(int s)
{
    cout << "jalr $" << s << endl;
}
void BEQ(int s, int t, string label)
{
    cout << "beq $" << s << ", $" << t << ", " << label << endl;
}
void BNE(int s, int t, string label)
{
    cout << "bne $" << s << ", $" << t << ", " << label << endl;
}
void LW(int t, int i, int s)
{
    cout << "lw $" << t << ", " << i << "($" << s << ")" << endl;
}
void SW(int t, int i, int s)
{
    cout << "sw $" << t << ", " << i << "($" << s << ")" << endl;
}
void word(int i)
{
    cout << ".word " << i << endl;
}
void word(string label)
{
    cout << ".word " + label << endl;
}
void LABEL(string label)
{
    cout << label << ":" << endl;
}
void push(int s)
{
    SW(s, -4, 30);
    SUB(30, 30, 4);
    // for SUB we must ensure that $4 holds the value of 4, which is why we will declare lis the value of 4 into $4
}
void pop(int s)
{
    add(30, 30, 4);
    LW(s, -4, 30);
}
void pop()
{
    add(30, 30, 4);
}
void comment(string s)
{
    cout << ";" << s << endl;
}
void constant(int s, int n)
{
    LIS(s);
    word(n);
}

void constant(int s, string str)
{
    LIS(s);
    word(str);
}

void traversedcls(TreeNode *t, map<string, int> &offsetTable, int &currOffset)
{
    if (t->rhs.size() == 0)
    {
        return;
    }
    traversedcls(t->getChild("dcls", 1), offsetTable, currOffset);
    offsetTable.insert({t->getChild("dcl", 1)->getChild("ID", 1)->lexeme, currOffset});
    // code gen
    if (t->getChild("NULL", 1))
    {
        constant(3, 1);
    }
    else
    {
        constant(3, stoi(t->getChild("NUM", 1)->lexeme));
    }
    push(3);
    currOffset -= 4;
    return;
}

void code(TreeNode *t, map<string, int> &offsetTable)
{
    // the objective of code is to lw a value from the stack which is precomputed from the offsettable into register 3
    // i.e lw $3, 8($29)
    if (t->rule == "expr expr PLUS term")
    {
        if (t->getChild("expr", 1)->type == "int" && t->getChild("term", 1)->type == "int")
        {
            code(t->getChild("expr", 1), offsetTable);
            push(3);
            code(t->getChild("term", 1), offsetTable);
            pop(5);
            add(3, 5, 3);
        }
        else if (t->getChild("expr", 1)->type == "int*" && t->getChild("term", 1)->type == "int")
        {
            code(t->getChild("expr", 1), offsetTable);
            push(3);
            code(t->getChild("term", 1), offsetTable);
            MULT(3, 4);
            MFLO(3);
            pop(5);
            add(3, 5, 3); // computes expr + (term*4)
        }
        else if (t->getChild("expr", 1)->type == "int" && t->getChild("term", 1)->type == "int*")
        {
            code(t->getChild("expr", 1), offsetTable);
            push(3);
            code(t->getChild("term", 1), offsetTable);
            pop(5); //$5 = expr
            MULT(5, 4);
            MFLO(5);
            add(3, 5, 3); // computes (expr*4) + term
        }
    }
    else if (t->rule == "expr expr MINUS term")
    {
        if (t->getChild("expr", 1)->type == "int" && t->getChild("term", 1)->type == "int")
        {
            code(t->getChild("expr", 1), offsetTable);
            push(3);
            code(t->getChild("term", 1), offsetTable);
            pop(5);
            SUB(3, 5, 3);
        }
        else if (t->getChild("expr", 1)->type == "int*" && t->getChild("term", 1)->type == "int")
        {
            code(t->getChild("expr", 1), offsetTable);
            push(3);
            code(t->getChild("term", 1), offsetTable);
            MULT(3, 4);
            MFLO(3);
            pop(5);
            SUB(3, 5, 3); // computes expr - (term*4)
        }
        else if (t->getChild("expr", 1)->type == "int*" && t->getChild("term", 1)->type == "int*")
        {
            code(t->getChild("expr", 1), offsetTable);
            push(3);
            code(t->getChild("term", 1), offsetTable);
            pop(5);
            SUB(3, 5, 3);
            DIV(3, 4);
            MFLO(3);
        }
    }
    else if (t->rule == "expr term")
    {
        // cout<<t->rule<<endl;
        code(t->getChild("term", 1), offsetTable);
    }
    else if (t->rule == "term factor")
    {
        // cout<<t->rule<<endl;
        code(t->getChild("factor", 1), offsetTable);
    }
    else if (t->rule == "term term STAR factor")
    {
        code(t->getChild("term", 1), offsetTable);
        push(3);
        code(t->getChild("factor", 1), offsetTable);
        pop(5);
        MULT(5, 3);
        MFLO(3);
    }
    else if (t->rule == "term term PCT factor")
    {
        code(t->getChild("term", 1), offsetTable);
        push(3);
        code(t->getChild("factor", 1), offsetTable);
        pop(5);
        DIV(5, 3);
        MFHI(3);
    }
    else if (t->rule == "term term SLASH factor")
    {
        code(t->getChild("term", 1), offsetTable);
        push(3);
        code(t->getChild("factor", 1), offsetTable);
        pop(5);
        DIV(5, 3);
        MFLO(3);
    }
    else if (t->rule == "factor ID")
    {
        LW(3, offsetTable.find(t->getChild("ID", 1)->lexeme)->second, 29);
    }
    else if (t->rule == "factor NUM")
    {
        constant(3, stoi(t->getChild("NUM", 1)->lexeme));
    }
    else if (t->rule == "factor NULL")
    {
        constant(3, 1);
    }
    else if (t->rule == "factor LPAREN expr RPAREN")
    {
        code(t->getChild("expr", 1), offsetTable);
    }

    else if (t->rule == "statements statements statement")
    {
        code(t->getChild("statements", 1), offsetTable);
        code(t->getChild("statement", 1), offsetTable);
    }

    else if (t->rule == "statement lvalue BECOMES expr SEMI")
    {
        TreeNode *lvalue = t->getChild("lvalue", 1);
        // go through parentheses
        while (lvalue->getChild("lvalue", 1))
        {
            lvalue = lvalue->getChild("lvalue", 1);
        }
        if (lvalue->rule == "lvalue ID")
        {
            code(t->getChild("expr", 1), offsetTable);
            SW(3, offsetTable.find(lvalue->getChild("ID", 1)->lexeme)->second, 29);
        }
        else if (lvalue->rule == "lvalue STAR factor")
        {
            code(t->getChild("expr", 1), offsetTable);
            push(3);
            code(lvalue->getChild("factor", 1), offsetTable); //$3 = computed mem address
            pop(5);                                           //$5 = expression
            SW(5, 0, 3);                                      // mem[computed mem address] = expr
        }
    }
    else if (t->rule == "statement PRINTLN LPAREN expr RPAREN SEMI")
    {
        push(31);
        constant(15, "print");
        code(t->getChild("expr", 1), offsetTable); // this will store the value of expression in register 3
        add(1, 3, 0);                              // sets the value of 3 to be the value of the expression
        JALR(15);
        pop(31);
    }

    else if (t->rule == "statments")
    {
        return;
    }
    else if (t->rule == "factor STAR factor")
    {
        // cout<<t->rule<<endl;
        code(t->getChild("factor", 1), offsetTable);
        LW(3, 0, 3);
    }
    else if (t->rule == "factor AMP lvalue")
    {
        // cout<<t->rule<<endl;
        TreeNode *lvalue = t->getChild("lvalue", 1);
        // go through parentheses
        while (lvalue->getChild("lvalue", 1))
        {
            lvalue = lvalue->getChild("lvalue", 1);
        }
        if (lvalue->rule == "lvalue ID")
        {
            constant(3, offsetTable.find(lvalue->getChild("ID", 1)->lexeme)->second);
            add(3, 29, 3);
        }
        else if (lvalue->rule == "lvalue STAR factor")
        {
            // cout<<lvalue->rule<<endl;
            code(lvalue->getChild("factor", 1), offsetTable);
        }
    }

    else if (t->rule == "test expr LT expr")
    {
        code(t->getChild("expr", 1), offsetTable);
        push(3);
        code(t->getChild("expr", 2), offsetTable);
        pop(5);
        if (t->getChild("expr", 1)->type == "int*" || t->getChild("expr", 2)->type == "int*")
        {
            SLTU(3, 5, 3);
        }
        else
        {
            SLT(3, 5, 3);
        }
    }
    else if (t->rule == "test expr GE expr")
    {
        code(t->getChild("expr", 1), offsetTable);
        push(3);
        code(t->getChild("expr", 2), offsetTable);
        pop(5);
        constant(17, 1);
        if (t->getChild("expr", 1)->type == "int*" || t->getChild("expr", 2)->type == "int*")
        {
            SLTU(3, 5, 3);
        }
        else
        {
            SLT(3, 5, 3);
        }
        SUB(3, 17, 3);
    }
    else if (t->rule == "test expr NE expr")
    {

        code(t->getChild("expr", 1), offsetTable);
        push(3);
        code(t->getChild("expr", 2), offsetTable);
        pop(5);
        constant(17, 1); //$17 = 1
        if (t->getChild("expr", 1)->type == "int*" || t->getChild("expr", 2)->type == "int*")
        {
            SLTU(7, 5, 3); //$7 <- expr2 < expr1
            SLTU(8, 3, 5); //$8 <- expr1 <expr2
        }
        else
        {
            SLT(7, 5, 3);
            SLT(8, 3, 5);
        }
        add(3, 7, 8); // 3 = 18 + 19
    }
    else if (t->rule == "test expr EQ expr")
    {
        code(t->getChild("expr", 1), offsetTable);
        push(3);
        code(t->getChild("expr", 2), offsetTable);
        pop(5);
        constant(17, 1); //$17 = 1
        if (t->getChild("expr", 1)->type == "int*" || t->getChild("expr", 2)->type == "int*")
        {
            SLTU(7, 5, 3); //$3 = (5<3)
            SLTU(8, 3, 5);
        }
        else
        {
            SLT(7, 5, 3); //$3 = (5<3)
            SLT(8, 3, 5);
        }
        add(3, 7, 8);  // 3 = 18 + 19
        SUB(3, 17, 3); // 3 = 1-(prev value of 3)
    }
    else if (t->rule == "test expr LE expr")
    {
        code(t->getChild("expr", 1), offsetTable);
        push(3);
        code(t->getChild("expr", 2), offsetTable);
        pop(5);
        constant(17, 1);
        if (t->getChild("expr", 1)->type == "int*" || t->getChild("expr", 2)->type == "int*")
        {
            SLTU(3, 3, 5);
        }
        else
        {
            SLT(3, 3, 5);
        }
        SUB(3, 17, 3);
    }
    else if (t->rule == "test expr GT expr")
    {
        code(t->getChild("expr", 1), offsetTable);
        push(3);
        code(t->getChild("expr", 2), offsetTable);
        pop(5);
        if (t->getChild("expr", 1)->type == "int*" || t->getChild("expr", 2)->type == "int*")
        {
            SLTU(3, 3, 5);
        }
        else
        {
            SLT(3, 3, 5);
        }
    }
    else if (t->rule == "statement IF LPAREN test RPAREN LBRACE statements RBRACE ELSE LBRACE statements RBRACE")
    {
        // if handling
        string else_count = "ELSE"+to_string(IfCounter);
        string end_if = "END" + to_string(IfCounter);
        IfCounter++;
        code(t->getChild("test", 1), offsetTable);
        BEQ(3, 0,else_count);
        code(t->getChild("statements", 1), offsetTable);
        BEQ(0, 0, end_if);
        LABEL(else_count);
        code(t->getChild("statements", 2), offsetTable);
        LABEL(end_if);
    }
    else if (t->rule == "statement WHILE LPAREN test RPAREN LBRACE statements RBRACE")
    {
        // while handling
        string while_count = "WHILE" + to_string(LoopCounter);
        string end_while = "ENDWHILE" + to_string(LoopCounter);
        LoopCounter++;
        LABEL(while_count);
        code(t->getChild("test", 1), offsetTable);
        BEQ(3, 0, end_while);
        code(t->getChild("statements", 1), offsetTable);
        BEQ(0, 0, while_count);
        LABEL(end_while);
    }
    else if (t->rule == "factor NEW INT LBRACK expr RBRACK")
    {
        code(t->getChild("expr", 1), offsetTable); //$3 <- size of array to allocate
        add(1, 3, 0);
        push(31);
        constant(15, "new");
        JALR(15);
        BNE(3, 0, "notNULL" + to_string(newCount));
        constant(3, 1);
        LABEL("notNULL" + to_string(newCount));
        pop(31);
        newCount++;
        // this is part of an assignment statment
        //$3 <-address of allocated array or 0 if failed allocation
    }

    else if (t->rule == "statement DELETE LBRACK RBRACK expr SEMI")
    {
        // the expr must be the address of a currently allocated array as returned by new
        code(t->getChild("expr", 1), offsetTable); //$3 = memory address to be deleted, if it's value
        add(15, 0, 3);                              // 15 = value of mem[ID];
        constant(11, 1);
        BEQ(15, 11, "DELETEEND" + to_string(deleteCount)); // if it's a 1 don't call delete
        push(31);
        constant(15, "delete");
        add(1, 0, 3); // expects the memory address of the array to delete, which should be in $3 as expr will be factor->ID
        JALR(15);
        pop(31);
        LABEL("DELETEEND" + to_string(deleteCount));
        deleteCount++;
    }
    else if (t->rule == "factor ID LPAREN RPAREN")
    {
        push(29);
        push(31);
        constant(15, "P" + t->getChild("ID", 1)->lexeme);
        JALR(15);
        pop(31);
        pop(29);
    }

    else if (t->rule == "factor ID LPAREN arglist RPAREN")
    {
        // calling a function
        push(29);
        push(31);
        int signature = 0;
        TreeNode *arglist = t->getChild("arglist", 1); // arglist is either expr or expr COMMA arglist
        while (arglist)
        {
            code(arglist->getChild("expr", 1), offsetTable);
            push(3);
            arglist = arglist->getChild("arglist", 1);
            signature++;
        }
        constant(15, "P" + t->getChild("ID", 1)->lexeme);
        JALR(15);
        for (int i = 0; i < signature; i++)
        {
            pop();
        }
        pop(31);
        pop(29);
    }

    return;
}

void WainCodeGen(TreeNode *main)
{
    constant(4, 4);
    cout << ".import print" << endl;
    cout << ".import init" << endl;
    cout << ".import new" << endl;
    cout << ".import delete" << endl;


    push(31);
    push(2);
    if (main->getChild("dcl", 1)->getChild("type",1)->rhs.size() == 1)
    {
        add(2, 0, 0);
    }
    constant(15, "init");
    JALR(15);
    pop(2);
    pop(31);
    map<string, int> offsetTable;

    int offset = 4 * 2; // 4(n), where n = size of params, in wain, n = 2;
    TreeNode *firstparam = main->getChild("dcl", 1);
    TreeNode *secondparam = main->getChild("dcl", 2);
    offsetTable.insert({firstparam->getChild("ID", 1)->lexeme, offset});
    push(1);
    offset -= 4;
    offsetTable.insert({secondparam->getChild("ID", 1)->lexeme, offset});
    push(2);
    offset -= 4;
    // set frame pointer
    SUB(29, 30, 4);
    traversedcls(main->getChild("dcls", 1), offsetTable, offset); // this is always first in main;

    // for statements
    code(main->getChild("statements", 1), offsetTable);

    // for return expr
    code(main->getChild("expr", 1), offsetTable);

    // call pop for each local variable that you pushed
    int popCounter = -1 * (offset / 4);

    // call pop for each local variable that you pushed
    for (int i = 0; i < popCounter; i++)
    {
        pop();
    }
    JR(31);
}

void ProceduresCodeGen(TreeNode *t)
{
    if (t->rhs.at(0) == "main")
    {
        return;
    }
    TreeNode *procedure = t->getChild("procedure", 1);
    // codegen for procedure
    // generate offsettable

    // these all need to be for procedure

    map<string, int> offsetTable;
    int offset = 0;
    LABEL("P" + procedure->getChild("ID", 1)->lexeme);
    SUB(29, 30, 4);

    TreeNode *signaturecounter = procedure->getChild("params", 1)->getChild("paramlist", 1);
    while (signaturecounter)
    {
        offset += 4;
        signaturecounter = signaturecounter->getChild("paramlist", 1);
    }
    TreeNode *paramlist = procedure->getChild("params", 1)->getChild("paramlist", 1);
    while (paramlist)
    {
        offsetTable.insert({paramlist->getChild("dcl", 1)->getChild("ID", 1)->lexeme, offset});
        paramlist = paramlist->getChild("paramlist", 1);
        offset -= 4;
    }

    // at this point offset should be 0
    traversedcls(procedure->getChild("dcls", 1), offsetTable, offset); // this is always first in main;
    code(procedure->getChild("statements", 1), offsetTable);
    code(procedure->getChild("expr", 1), offsetTable);

    int popCounter = -1 * (offset / 4);

    // call pop for each local variable that you pushed
    for (int i = 0; i < popCounter; i++)
    {
        pop();
    }
    JR(31);
    // for (auto it = offsetTable.begin(); it != offsetTable.end(); it++)
    // {
    //     cout << it->first << " " << it->second << endl;
    // }

    ProceduresCodeGen(t->getChild("procedures", 1));
    return;
}

int main()
{
    TreeNode *root = populateTreeNode();
    TreeNode *main = root->getChild("procedures", 1);
    while (main->getChild("procedures", 1))
    {
        main = main->getChild("procedures", 1);
    }
    main = main->getChild("main", 1);
    WainCodeGen(main);

    if (root->getChild("procedures", 1)->getChild("procedure", 1)) // ie if the root is procedures -> procedure procedures
    {
        ProceduresCodeGen(root->getChild("procedures", 1));
    }
    delete root;
}

// g++ -Wall -std=c++17 wlp4gen-preanalyzed.cc -o project5