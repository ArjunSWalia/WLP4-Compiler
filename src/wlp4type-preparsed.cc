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
    TreeNode() : rule(""), lhs(""), kind(""), lexeme(""), type("") {}

    TreeNode *getChild(string str, int n)
    {
        // nth child of the tree?
        int childcount = 0;
        for (int i = 0; i < children.size(); i++)
        {
            // cout<<children.at(i)->lhs<<endl;
            if (children.at(i)->lhs == str || children.at(i)->kind == str)
            {
                // cout<<"THIS MUST RUN"<<endl;
                childcount++;
                if (childcount == n)
                {
                    return children.at(i);
                }
            }
        }
        return nullptr;
    }
};

struct Variable
{
    string name;
    string type;
    Variable(TreeNode &t)
    {
        // assert(t.lhs == "dcl");
        TreeNode *typeParent = t.getChild("type", 1);
        TreeNode *nameParent = t.getChild("ID", 1);
        name = nameParent->lexeme;
        if (typeParent->rhs.size() == 3) //[type,INT,STAR]
        {
            type = "int*";
        }
        else
        {
            type = "int";
        }
    }
    Variable() : name(""), type("") {}
};

struct VariableTable
{
    map<string, Variable> Vtable;
    void add(Variable v)
    {
        if (Vtable.find(v.name) == Vtable.end())
        {
            Vtable.insert({v.name, v});
        }
        else
        {
            cerr << "ERROR duplicate variable declaration";
        }
    }
    Variable get(string str)
    {
        if (Vtable.find(str) != Vtable.end())
        {
            return Vtable.find(str)->second;
        }
        else
        {
            cerr << "ERROR use of undeclared variable";
            Variable v;
            v.type = "ERROR";
            v.name = "ERROR";
            return v;
        }
    }
};

// need to test populateVtable, populateSignatureMain, populateSignatureProcedure,and struct defns for Procedure & proceduretable, but they are all pretty simple

void populateVtable(TreeNode *t, VariableTable &Vtable)
{
    if (t == nullptr)
    {
        return;
    }
    TreeNode *dcl = t->getChild("dcl", 1);
    if (dcl != nullptr)
    {
        Vtable.add(Variable(*(dcl)));
        // When dcls derives dcls dcl BECOMES NUM SEMI, the derived dcl must derive a sequence containing a type that derives INT.
        // When dcls derives dcls dcl BECOMES NULL SEMI, the derived dcl must derive a sequence containing a type that derives INT STAR.
        string str = Vtable.get(t->getChild("dcl", 1)->getChild("ID", 1)->lexeme).type; // grabs the type of the variable we have just added
        if (t->rule == "dcls dcls dcl BECOMES NUM SEMI")
        {
            if (str != "int")
            {
                cerr << "ERROR";
            }
        }
        else if (t->rule == "dcls dcls dcl BECOMES NULL SEMI")
        {
            if (str != "int*")
            {
                cerr << "ERROR";
            }
        }
    }
    populateVtable(t->getChild("dcls", 1), Vtable);
}

void populateSignatureMain(TreeNode *t, vector<string> &v, VariableTable &Vtable)
{
    TreeNode *param1 = t->getChild("dcl", 1);
    TreeNode *param2 = t->getChild("dcl", 2);
    if (param1->getChild("type", 1)->rhs.size() == 3)
    {
        v.push_back("int*");
    }
    else
    {
        v.push_back("int");
    }
    if (param2->getChild("type", 1)->rhs.size() == 3)
    {
        v.push_back("int*");
    }
    else
    {
        v.push_back("int");
    }
    Vtable.add(Variable(*(param1)));
    Vtable.add(Variable(*(param2)));
}

void populateSignatureProcedure(TreeNode *t, vector<string> &v, VariableTable &Vtable)
{
    // t = paramlist->dcl COMMA paramlist OR t = paramlist->dcl
    // we want to expand dcl or paramlist, like our child of paramlist can either be dcl or paramlist thats what we want
    if (t == nullptr)
    {
        return;
    }
    TreeNode *dcl = t->getChild("dcl", 1);
    if (dcl != nullptr)
    {
        if (dcl->getChild("type", 1)->rhs.size() == 3)
        {
            v.push_back("int*");
        }
        else
        {
            v.push_back("int");
        }
        Vtable.add(Variable(*(dcl)));
    }
    populateSignatureProcedure(t->getChild("paramlist", 1), v, Vtable); // if t->getChild("paramlist",1) = nullptr, i.e when t = paramlist .EMPTY, what do we do?
}

struct Procedure
{
    string name;
    vector<string> signature; // list of parameter types, list of int,int*...
    VariableTable vTable;     // string to variable
    Procedure(TreeNode *t)
    {

        // t = procedure INT ID LPAREN params RPAREN LBRACE dcls statements RETURN expr SEMI RBRACE
        // OR t = main INT WAIN LPAREN dcl COMMA dcl RPAREN LBRACE dcls statements RETURN expr SEMI RBRACE
        // must check the lhs as it procedure/main are nonterminals, therefore they have lhs values, not kind/lexeme
        if (t->lhs == "procedure")
        {
            name = t->getChild("ID", 1)->lexeme;
            TreeNode *paramlist = t->getChild("params", 1)->getChild("paramlist", 1); // paramlist dcl COMMA paramlist is passed into populatesignatureprocedure
            populateSignatureProcedure(paramlist, signature, vTable);
            populateVtable(t->getChild("dcls", 1), vTable);
        }
        else if (t->lhs == "main")
        {
            name = "wain";
            populateSignatureMain(t, signature, vTable); // t = main INT WAINT LPAREN dcl COMMA...
            populateVtable(t->getChild("dcls", 1), vTable);
            // You can check that the second parameter of wain has type int in the Procedure constructor, or by examining the signature of wain after the constructor returns.
            if (signature.at(1) != "int")
            {
                cerr << "ERROR";
            }
        }
    }
    Procedure() : name("") {}
};
struct ProcedureTable
{
    map<string, Procedure> pTable;

    void add(Procedure p)
    {
        if (pTable.find(p.name) == pTable.end())
        {
            pTable.insert({p.name, p});
        }
        else
        {
            cerr << "ERROR duplicate procedure declaration";
        }
    }
    Procedure get(string str)
    {
        if (pTable.find(str) != pTable.end())
        {
            return pTable.find(str)->second;
        }
        else
        {
            Procedure t;
            t.name = "ERROR";
            cerr << "ERROR use of undeclared procedure";
            return t;
        }
    }
    // colectProcedures calls annotateTypes which is new
    void collectProcedures(TreeNode *t);
};

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

void compareSignature(TreeNode *t, vector<string> &v)
{
    if (t == nullptr)
    {
        return;
    }
    v.push_back(t->getChild("expr", 1)->type);
    compareSignature(t->getChild("arglist", 1), v);
}

// should be similar to to rootConstr method from treeprint.cc
TreeNode *populateTreeNode()
{
    string prodrule; // start BOF procedures EOF
    getline(cin, prodrule);
    TreeNode *t = new TreeNode();
    t->rule = prodrule;
    t->rhs = splitter(prodrule);
    t->lhs = t->rhs.at(0);

    // starting at 1 for rhs
    if (t->rhs.at(1) == ".EMPTY")
    {
        t->rhs = vector<string>(); // CHANGE to {""};
    }
    else
    {
        for (int i = 1; i < t->rhs.size(); i++)
        {
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
            }
        }
    }

    return t;
}
void annotateTypes(Procedure &p, ProcedureTable &globalTable, TreeNode *t)
{
    // initally called with t = procedure INT ID LPAREN params RPAREN LBRACE dcls statements RETURN expr SEMI RBRACE
    //  OR t = main INT WAIN LPAREN dcl COMMA dcl RPAREN LBRACE dcls statements RETURN expr SEMI RBRACE

    // we iterate over the children first, because we want to compute the types of the children before the parents.
    for (int i = 0; i < t->children.size(); i++)
    {
        annotateTypes(p, globalTable, t->children.at(i));
    }
    // annotating types

    // The type of a factor deriving NUM or NULL is the same as the type of that token.
    if (t->rule == "factor NUM")
    {
        t->type = "int";
    }
    else if (t->rule == "factor NULL")
    {
        t->type = "int*";
    }
    // When factor derives ID, the derived ID must have a type, and the type of the factor is the same as the type of the ID.
    // When lvalue derives ID, the derived ID must have a type, and the type of the lvalue is the same as the type of the ID.
    else if (t->rule == "factor ID" || t->rule == "lvalue ID")
    {
        // p has a table of variables within v, globaltable is the list of all procedure names
        if (p.vTable.get(t->getChild("ID", 1)->lexeme).type == "")
        {
            cerr << "ERROR";
        }
        else
        {
            t->type = p.vTable.get(t->getChild("ID", 1)->lexeme).type;
        }
    }
    // The type of a factor deriving LPAREN expr RPAREN is the same as the type of the expr.
    else if (t->rule == "factor LPAREN expr RPAREN")
    {
        t->type = t->getChild("expr", 1)->type;
    }
    // The type of an lvalue deriving LPAREN lvalue RPAREN is the same as the type of the derived lvalue.

    else if (t->rule == "lvalue LPAREN lvalue RPAREN")
    {
        t->type = t->getChild("lvalue", 1)->type;
    }

    // The type of a factor deriving AMP lvalue is int*. The type of the derived lvalue (i.e. the one preceded by AMP) must be int.
    else if (t->rule == "factor AMP lvalue")
    {
        if (t->getChild("lvalue", 1)->type == "int")
        {
            t->type = "int*";
        }
        else
        {
            cerr << "ERROR";
        }
    }

    // The type of a factor or lvalue deriving STAR factor is int. The type of the derived factor (i.e. the one preceded by STAR) must be int*.
    else if (t->rule == "factor STAR factor" || t->rule == "lvalue STAR factor")
    {
        if (t->getChild("factor", 1)->type == "int*")
        {
            t->type = "int";
        }
        else
        {
            cerr << "ERROR";
        }
    }

    // The type of a factor deriving NEW INT LBRACK expr RBRACK is int*. The type of the derived expr must be int.
    else if (t->rule == "factor NEW INT LBRACK expr RBRACK")
    {
        if (t->getChild("expr", 1)->type == "int")
        {
            t->type = "int*";
        }
        else
        {
            cerr << "ERROR";
        }
    }

    // The type of a factor deriving ID LPAREN RPAREN is int. The procedure whose name is ID must have an empty signature.
    else if (t->lhs == "factor" && t->rhs.at(1) == "ID")
    {

        // Check that the ID's lexeme is not in the local VariableTable for the current procedure.
        // foo(){ func()}
        // Note the "current procedure" refers to the procedure in which the call occurs, not the procedure being called. For example, if wain calls the procedure proc then this lookup should be done in wain's local symbol table.
        if (p.vTable.Vtable.find(t->getChild("ID", 1)->lexeme) != p.vTable.Vtable.end()) // I say this because if I use func.vTable.get(getChild("ID",1)->lexeme) it either returns a variable or throws an error
        {
            cerr << "ERROR";
        }
        // If the lexeme is not in the local symbol table (meaning it does not refer to a local variable in this context), now check that the ID's lexeme is in the top-level ProcedureTable. If it's not, this is a "use of undeclared procedure" error.
        else
        {
            if (globalTable.pTable.find(t->getChild("ID", 1)->lexeme) == globalTable.pTable.end()) // this should already fail above when we create procedure func...
            {
                cerr << "ERROR use of undeclared procedure";
            }
        }
        if (t->rule == "factor ID LPAREN RPAREN")
        {
            if (globalTable.get(t->getChild("ID", 1)->lexeme).signature.size() == 0)
            {
                t->type = "int";
            }
            else
            {
                cerr << "ERROR";
            }
        }
        else if (t->rule == "factor ID LPAREN arglist RPAREN")
        {
            vector<string> funcsig = globalTable.get(t->getChild("ID", 1)->lexeme).signature;
            vector<string> funccall;
            compareSignature(t->getChild("arglist", 1), funccall);
            if (funcsig != funccall)
            {
                cerr << "ERROR";
            }
            else
            {
                t->type = "int";
            }
        }
        // WLP4 procedures always return int, so after this error checking is done, you simply need to assign int as the type of the node. We already do this above, but no harm in redundancy
        t->type = "int";
    }
    // The type of a term deriving factor is the same as the type of the derived factor.
    else if (t->rule == "term factor")
    {
        t->type = t->getChild("factor", 1)->type;
    }

    // The type of a term directly deriving anything other than just factor is int. The term and factor directly derived from such a term must have type int.
    else if (t->lhs == "term" && t->rhs.at(1) != "factor")
    {
        /*
            term term STAR factor
            term term SLASH factor
            term term PCT factor
        */
        if (t->getChild("term", 1)->type == "int" && t->getChild("factor", 1)->type == "int")
        {
            t->type = "int";
        }
        else
        {
            cerr << "ERROR";
        }
    }

    // The type of an expr deriving term is the same as the type of the derived term.
    else if (t->rule == "expr term")
    {
        t->type = t->getChild("term", 1)->type;
    }

    // When expr derives expr PLUS term:
    else if (t->rule == "expr expr PLUS term")
    {
        if (t->getChild("expr", 1)->type == "int" && t->getChild("term", 1)->type == "int")
        {
            t->type = "int";
        }
        else if (t->getChild("expr", 1)->type == "int*" && t->getChild("term", 1)->type == "int")
        {
            t->type = "int*";
        }
        else if (t->getChild("expr", 1)->type == "int" && t->getChild("term", 1)->type == "int*")
        {
            t->type = "int*";
        }
        else
        {
            cerr << "ERROR";
        }
    }
    // When expr derives expr MINUS term:
    else if (t->rule == "expr expr MINUS term")
    {
        if (t->getChild("expr", 1)->type == "int" && t->getChild("term", 1)->type == "int")
        {
            t->type = "int";
        }
        else if (t->getChild("expr", 1)->type == "int*" && t->getChild("term", 1)->type == "int")
        {
            t->type = "int*";
        }
        else if (t->getChild("expr", 1)->type == "int*" && t->getChild("term", 1)->type == "int*")
        {
            t->type = "int";
        }
        else
        {
            cerr << "ERROR";
        }
    }
    return;
}

void checkStatements(TreeNode *t)
{
    // t = statements ... or statement, essentially just any treenode that can potentially contain the treenode statements, which is either statement or statements
    if (t->getChild("test", 1))
    {
        // check for test
        // Whenever test directly derives a sequence containing two exprs, they must both have the same type.
        if (t->getChild("test", 1)->getChild("expr", 1)->type != t->getChild("test", 1)->getChild("expr", 2)->type)
        {
            cerr << "ERROR";
        }
    }

    // When statement derives lvalue BECOMES expr SEMI, the derived lvalue and the derived expr must have the same type.
    // When statement derives PRINTLN LPAREN expr RPAREN SEMI, the derived expr must have type int.
    // When statement derives DELETE LBRACK RBRACK expr SEMI, the derived expr must have type int*.
    if (t->rule == "statement lvalue BECOMES expr SEMI")
    {
        if (t->getChild("lvalue", 1)->type != t->getChild("expr", 1)->type)
        {
            cerr << "ERROR";
        }
    }
    else if (t->rule == "statement PRINTLN LPAREN expr RPAREN SEMI")
    {
        if (t->getChild("expr", 1)->type != "int")
        {
            cerr << "ERROR";
        }
    }
    else if (t->rule == "statement DELETE LBRACK RBRACK expr SEMI")
    {
        if (t->getChild("expr", 1)->type != "int*")
        {
            cerr << "ERROR";
        }
    }

    // t = statement -> ... statements ... statements
    int n = 1;
    while (t->getChild("statements", n))
    {
        checkStatements(t->getChild("statements", n));
        n++;
    }
    // t = statements -> statements statement
    if (t->getChild("statement", 1))
    {
        checkStatements(t->getChild("statement", 1));
    }
    return;
}

void ProcedureTable::collectProcedures(TreeNode *t)
{
    // the token that should be called is the token procedures -> procedure procedures
    // OR procedures->main
    if (t == nullptr)
    {
        return;
    }
    if (t->getChild("main", 1))
    {
        // calling annotatetypes every time we add a procedure
        Procedure p = Procedure(t->getChild("main", 1));
        add(p);
        // this is in reference to the proceduretable that calls the function
        annotateTypes(p, *this, t->getChild("main", 1));
        // You can check that the type of the return expr of a procedure is int in collectProcedures immediately after annotateTypes finishes.
        if (t->getChild("main", 1)->getChild("expr", 1)->type != "int")
        {
            cerr << "ERROR";
        }
        checkStatements(t->getChild("main", 1)->getChild("statements", 1));
    }
    else if (t->getChild("procedure", 1))
    {
        Procedure p = Procedure(t->getChild("procedure", 1));
        add(p);
        annotateTypes(p, *this, t->getChild("procedure", 1));
        // You can check that the type of the return expr of a procedure is int in collectProcedures immediately after annotateTypes finishes.
        if (t->getChild("procedure", 1)->getChild("expr", 1)->type != "int")
        {
            
            cerr << "ERROR";
        }
        checkStatements(t->getChild("procedure", 1)->getChild("statements", 1));
    }
    collectProcedures(t->getChild("procedures", 1));
}

int main()
{
    TreeNode *root = populateTreeNode();
    // ProcedureTable pTable;
    ProcedureTable pTable;
    pTable.collectProcedures(root->getChild("procedures", 1));
    delete root;
}
