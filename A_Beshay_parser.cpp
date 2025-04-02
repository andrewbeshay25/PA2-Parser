/* Implementation of Recursive-Descent Parser
	for the Simple Ada-Like (SADAL) Language
 * A_Beshay_parser.cpp
 * Programming Assignment 2
 * Spring 2025
*/
#include <queue>
#include "parser.h"

map<string, bool> defVar;

namespace Parser {
	bool pushed_back = false;
	LexItem	pushed_token;

	static LexItem GetNextToken(istream& in, int& line) {
		if( pushed_back ) {
			pushed_back = false;
			return pushed_token;
		}
		return getNextToken(in, line);
	}

	static void PushBackToken(LexItem & t) {
		if( pushed_back ) {
			abort();
		}
		pushed_back = true;
		pushed_token = t;	
	}

}

static int error_count = 0;

int ErrCount()
{
    return error_count;
}

void ParseError(int line, string msg)
{
	++error_count;
	cout << line << ": " << msg << endl;
}
extern bool Type(istream& in, int& line) {
	return true;
}

extern bool Prog(istream& in, int& line) {
	bool status = false;
	LexItem token;
	cout << "in Prog" << endl;

	token = Parser::GetNextToken(in, line);

	if (token != PROCEDURE) {
		ParseError(line, "Unexpected token");
		return false;
	}

	token = Parser::GetNextToken(in, line);
	if (token != IDENT) {
		ParseError(line, "Unexpected token");
		return false;
	}

	token = Parser::GetNextToken(in, line);
	if (token != IS) {
		ParseError(line, "Unexpected token");
		return false;
	}

	status = ProcBody(in, line);

	if (!status) {
		ParseError(line, "some message here");
		return false;
	}

	return true;
}
extern bool ProcBody(istream& in, int& line) {
	LexItem token;
	bool status = false;
	cout << "in ProcBody" << endl;

	status = DeclPart(in, line);
	if (!status) {
		ParseError(line, "some message here");
		return false;
	}

	token = Parser::GetNextToken(in, line);
	if (token != BEGIN) {
		ParseError(line, "Unexpected token");
		return false;
	}

	status = StmtList(in, line);
	if (!status) {
		ParseError(line, "some message here");
		return false;
	}

	token = Parser::GetNextToken(in, line);
	if (token != END) {
		ParseError(line, "Unexpected token");
		return false;
	}

	token = Parser::GetNextToken(in, line);
	if (token != IDENT) {
		ParseError(line, "Unexpected token");
		return false;
	}
	return true;
}

//DeclPart ::= DeclStmt { DeclStmt }
bool DeclPart(istream& in, int& line) {
	bool status = false;
	LexItem tok;
	cout << "in DeclPart" << endl;
	status = DeclStmt(in, line);
	if(status)
	{
		tok = Parser::GetNextToken(in, line);
		if(tok == BEGIN )
		{
			Parser::PushBackToken(tok);
			return true;
		}
		else 
		{
			Parser::PushBackToken(tok);
			status = DeclPart(in, line);
		}
	}
	else
	{
		ParseError(line, "Non-recognizable Declaration Part.");
		return false;
	}
	return true;
}//end of DeclPart function

//StmtList ::= Stmt { Stmt }
bool DeclStmt(istream& in, int& line) {
	bool status = false;
	LexItem tok;
	cout << "in DeclStmt" << endl;

	tok = Parser::GetNextToken(in, line);
	if(tok != IDENT) {
		ParseError(line, "Unexpected token");
		return false;
	}

	tok = Parser::GetNextToken(in, line);
	if (tok == COMMA) {

	}



	return true;
}

bool StmtList(istream& in, int& line)
{
	bool status;
	LexItem tok;
	//cout << "in StmtList" << endl;
	status = Stmt(in, line);
	tok = Parser::GetNextToken(in, line);
	while(status && (tok != END && tok != ELSIF && tok != ELSE))
	{
		Parser::PushBackToken(tok);
		status = Stmt(in, line);
		tok = Parser::GetNextToken(in, line);
	}
	if(!status)
	{
		ParseError(line, "Syntactic error in statement list.");
		return false;
	}
	Parser::PushBackToken(tok); //push back the END token
	return true;
}//End of StmtList

extern bool Stmt(istream& in, int& line) {
	return true;
}
extern bool PrintStmts(istream& in, int& line){
	return true;
}
extern bool GetStmt(istream& in, int& line){
	return true;
}
extern bool IfStmt(istream& in, int& line){
	return true;
}
extern bool AssignStmt(istream& in, int& line){
	return true;
}
extern bool Var(istream& in, int& line){
	return true;
}
extern bool Expr(istream& in, int& line){
	return true;
}
extern bool Relation(istream& in, int& line){
	return true;
}
extern bool SimpleExpr(istream& in, int& line){
	return true;
}
extern bool STerm(istream& in, int& line){
	return true;
}
extern bool Term(istream& in, int& line, int sign){
	return true;
}
extern bool Factor(istream& in, int& line, int sign){
	return true;
}
extern bool Primary(istream& in, int& line, int sign){
	return true;
}
extern bool Name(istream& in, int& line){
	return true;
}
extern bool Range(istream& in, int& line){
	return true;
}