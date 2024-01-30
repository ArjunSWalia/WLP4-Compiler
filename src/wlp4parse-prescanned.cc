#include <cassert>
#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <map>
#include <algorithm>
#include <sstream>
#include <memory>
#include "wlp4data.h"
using namespace std;

struct CFGRule
{
    string LHS;
    vector<string> RHS;
    CFGRule() : LHS(""), RHS({""}) {}
    CFGRule(string LHS, vector<string> RHS) : LHS(LHS), RHS(RHS) {}
};

struct SLRDFA
{
    map<pair<int, string>, int> transitions;
    map<pair<int, string>, int> reductions;
};

struct Token
{
    string kind;
    string lexeme;
    Token() : kind(""), lexeme("") {}
    Token(string kind, string lexeme) : kind(kind), lexeme(lexeme) {}
};

struct TokenList
{
    vector<Token> tokens;
    int currToken;
};

struct TreeNode
{
    string kind;
    string lexeme;
    vector<unique_ptr<TreeNode>> children;
    TreeNode(string kind, string lexeme) : kind(kind), lexeme(lexeme) {}
    void print()
    {
        if (lexeme.size() == 0)
        {
            cout << kind <<" .EMPTY"<<endl;
        }
        else
        {
            cout << kind << " " << lexeme<<endl;
        }
        for (int i = 0; i < children.size(); i++)
        {
            children.at(i)->print();
        }
    }
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

void populateCFG(vector<CFGRule> &Rules)
{
    string str;
    string LHS;
    vector<string> RHS;
    vector<string> empty;
    istringstream rules(WLP4_CFG);
    // skip first line of .CFG
    getline(rules, str);
    CFGRule temp;
    while (getline(rules, str))
    {
        LHS = str.substr(0, str.find(' '));
        RHS = splitter(str.substr(str.find(' ') + 1));
        if (RHS.at(0) == ".EMPTY")
        {
            RHS = empty;
        }
        temp = CFGRule(LHS, RHS);
        Rules.push_back(temp);
    }
}

void populateTransitions(SLRDFA &dfa)
{
    int fromstate;
    string symbol;
    int tostate;
    istringstream transitions(WLP4_TRANSITIONS);
    getline(transitions, symbol);
    while (transitions >> fromstate >> symbol >> tostate)
    {
        pair<int, string> pair1(fromstate, symbol);
        dfa.transitions.insert({pair1, tostate});
    }
}

void populateReductions(SLRDFA &dfa)
{
    int statenumber;
    string tag;
    int rulenumber;
    istringstream reductions(WLP4_REDUCTIONS);
    getline(reductions, tag);
    while (reductions >> statenumber >> rulenumber >> tag)
    {
        pair<int, string> pair1(statenumber, tag);
        dfa.reductions.insert({pair1, rulenumber});
    }
}

void populateTokenList(TokenList &tokenlist)
{
    tokenlist.currToken = 0;
    string kind;
    string lexeme;
    tokenlist.tokens.push_back(Token("BOF", "BOF"));
    while (cin >> kind >> lexeme)
    {
        tokenlist.tokens.push_back(Token(kind, lexeme));
    }
    tokenlist.tokens.push_back(Token("EOF", "EOF"));
}

void reduceTrees(vector<unique_ptr<TreeNode>> &ParseTree, CFGRule &Rule)
{
    int len = Rule.RHS.size();

    string vectostring;
    // setting the string up for printing
    for (int i = 0; i < Rule.RHS.size(); i++)
    {
        vectostring += Rule.RHS.at(i);
        if (i != Rule.RHS.size() - 1)
        {
            vectostring += " ";
        }
    }
    unique_ptr<TreeNode> temp = make_unique<TreeNode>(Rule.LHS, vectostring);
    for (int i = ParseTree.size() - len; i < ParseTree.size(); i++)
    {
        temp->children.push_back(move(ParseTree.at(i))); // adds in reverse order
    }
    for (int i = 0; i < len; i++)
    {
        if (!ParseTree.empty())
        {
            ParseTree.pop_back();
        }
    }
    ParseTree.push_back(move(temp));
}

void reduceStates(vector<int> &SLRStates, CFGRule &Rule, SLRDFA &dfa)
{
    int len = Rule.RHS.size();
    for (int i = 0; i < len; i++)
    {
        SLRStates.pop_back();
    }
    SLRStates.push_back(dfa.transitions.find({SLRStates.back(), Rule.LHS})->second);
}
bool shift(vector<unique_ptr<TreeNode>> &ParseTree, vector<int> &SLRStates, TokenList &tokenlist, SLRDFA &dfa)
{
    string kind = tokenlist.tokens.at(tokenlist.currToken).kind;
    string lexeme = tokenlist.tokens.at(tokenlist.currToken).lexeme;
    unique_ptr<TreeNode> temp = make_unique<TreeNode>(kind, lexeme);
    ParseTree.push_back(move(temp));
    // checking if a transition exists
    auto hasTrans = dfa.transitions.find({SLRStates.back(), kind});
    if (hasTrans != dfa.transitions.end())
    {
        SLRStates.push_back(hasTrans->second);
    }
    else
    {
        cerr << "ERROR";
        return false;
    }
    // consuming first token from unread input
    tokenlist.currToken++;
    return true;
}

int main()
{
    vector<CFGRule> CFGRules;
    populateCFG(CFGRules);
    SLRDFA dfa;
    populateTransitions(dfa);
    populateReductions(dfa);
    TokenList tokenlist;
    tokenlist.currToken = 0;
    populateTokenList(tokenlist);
    vector<int> SLRStates;
    SLRStates.push_back(0);
    vector<unique_ptr<TreeNode>> ParseTree;
    while (tokenlist.currToken != tokenlist.tokens.size())
    {
        // while there is a reduction
        while (dfa.reductions.find({SLRStates.back(), tokenlist.tokens.at(tokenlist.currToken).kind}) != dfa.reductions.end())
        {
            CFGRule cfg = CFGRules.at(dfa.reductions.find({SLRStates.back(), tokenlist.tokens.at(tokenlist.currToken).kind})->second);
            reduceTrees(ParseTree, cfg);
            reduceStates(SLRStates, cfg, dfa);
        }
        if (!shift(ParseTree, SLRStates, tokenlist, dfa))
        {
            ParseTree.clear();
            return EXIT_FAILURE;
        }
    }
    reduceTrees(ParseTree, CFGRules.at(0));
    // our parsetree stack should be of size one
    ParseTree.at(0)->print();
    ParseTree.clear();
}
