/* Implementation of Recursive-Descent Parser
	for the Simple Ada-Like (SADAL) Language
 * A_Beshay_parser.cpp
 * Programming Assignment 2
 * Spring 2025
*/
#include <queue>
#include "parser.h"

map<string, bool> defVar;
bool endOfLineError = true;

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

bool Prog(istream& in, int& line) {
	bool status = false;
	LexItem token;
	// cout << "in Prog" << endl;

	token = Parser::GetNextToken(in, line);

	if (token != PROCEDURE) {
		ParseError(line, "Incorrect compilation file.");
		return false;
	}

	token = Parser::GetNextToken(in, line);
	if (token != IDENT) {
		ParseError(line, "Missing Procedure Name.");
		return false;
	}

	token = Parser::GetNextToken(in, line);
	if (token != IS) {
		ParseError(line, "Unexpected token");
		return false;
	}

	status = ProcBody(in, line);

	if (!status) {
		ParseError(endOfLineError ? line-1 : line, "Incorrect Procedure Definition.");
		return false;
	}

	return true;
}
bool ProcBody(istream& in, int& line) {
	LexItem token;
	bool status = false;
	// cout << "in ProcBody" << endl;

	status = DeclPart(in, line);
	if (!status) {
		ParseError(line, "Incorrect procedure body.");
		return false;
	}

	token = Parser::GetNextToken(in, line);
	if (token != BEGIN) {
		ParseError(line, "Incorrect procedure body.");
		return false;
	}

	status = StmtList(in, line);
	if (!status) {
		ParseError(endOfLineError ? line-1 : line,"Incorrect Procedure Body.");
		return false;
	}

	token = Parser::GetNextToken(in, line);
	if (token != END) {
		ParseError(line, "Unexpected token2");
		return false;
	}

	token = Parser::GetNextToken(in, line);
	if (token != IDENT) {
		ParseError(line, "Unexpected token3");
		return false;
	}
	return true;
}

//DeclPart ::= DeclStmt { DeclStmt }
bool DeclPart(istream& in, int& line) {
	bool status = false;
	LexItem tok;
	// cout << "in DeclPart" << endl;
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

bool DeclStmt(istream& in, int& line) {
	bool status = false;
	LexItem tok;
	// cout << "in DeclStmt" << endl;

	tok = Parser::GetNextToken(in, line);
	if(tok != IDENT) {
		ParseError(line, "Incorrect Declaration Type. " + tok.GetLexeme());
		return false;
	}

	if (defVar.find(tok.GetLexeme()) == defVar.end()) {
		defVar[tok.GetLexeme()] = false;
	} else {
		ParseError(line, "Variable Redefinition");
		ParseError(line, "Incorrect identifiers list in Declaration Statement.");
		return false;
	}

	tok = Parser::GetNextToken(in, line);
	if (tok == IDENT) {
		ParseError(line, "Missing comma in declaration statement.");
		ParseError(line, "Incorrect identifiers list in Declaration Statement.");
		return false;
	}

	LexItem identToBeConsidered = tok;

	if (tok == COMMA) {
		return true;
	}

	if (tok != COLON) {
		ParseError(line, "Incorrect Declaration Statement.1");
		return false;
	}

	tok = Parser::GetNextToken(in, line);
	if(tok != CONST) {
		Parser::PushBackToken(tok);
	}

	status = Type(in, line);
	if (!status) {
		ParseError(line, "Incorrect Declaration Type.");
		return false;
	}

	tok = Parser::GetNextToken(in, line);
	if (tok == SEMICOL) {
		return true;
	}

	if(tok != ASSOP) {
		Parser::PushBackToken(tok);
		status = Range(in, line);
		if (!status) {
			ParseError(line, "Incorrect Declaration Statement.");
			return false;
		}
	} else if(tok == ASSOP) {
		status = Expr(in, line);
		if (!status) {
			ParseError(line, "No Expression found");
			return false;
		}

		// defined vars
		defVar[identToBeConsidered.GetLexeme()] = true;

		tok = Parser::GetNextToken(in, line);
		if (tok != SEMICOL) {
			ParseError(line, "No semicolon found");
			return false;
		}
	}

	// MAKE THE DECLARED VARIABLES TRUE SOMEHOW!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	return true;
}

extern bool Type(istream& in, int& line) {
	LexItem tok;
	// cout << "in Type" << endl;

	tok = Parser::GetNextToken(in, line);
	if(tok != INT && tok != FLOAT && tok != BOOL && tok != CHAR && tok != STRING) {
		return false;
	}
	return true;
}

//StmtList ::= Stmt { Stmt }
bool StmtList(istream& in, int& line) {
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
		ParseError(endOfLineError ? line-1 : line, "Syntactic error in statement list.");
		return false;
	}
	Parser::PushBackToken(tok); //push back the END token
	return true;
}//End of StmtList

bool Stmt(istream& in, int& line) {
	LexItem tok;
	tok = Parser::GetNextToken(in, line);
	if(tok == PUT || tok == PUTLN) {
		Parser::PushBackToken(tok);
		if (!PrintStmts(in, line)) {
			ParseError(endOfLineError ? line-1 : line, "Invalid put statement.");
			return false;
		}
		return true;
	} else if (tok == GET) {
		Parser::PushBackToken(tok);
		if (!GetStmt(in, line)) {
			ParseError(endOfLineError ? line-1 : line, "Invalid get statement.");
			return false;
		}
		return true;
	} else if (tok == IF) {
		Parser::PushBackToken(tok);
		if (!IfStmt(in, line)) {
			ParseError(line, "Invalid if statement.");
			return false;
		}
		return true;
	}
	Parser::PushBackToken(tok);
	if (!AssignStmt(in, line)) {
		ParseError(endOfLineError ? line-1 : line, "Invalid assignment statement.");
		return false;
	}
	ParseError(endOfLineError ? line-1 : line, "Not sure which one to use..");
	return false;
}
bool PrintStmts(istream& in, int& line){
	LexItem tok;
	bool status = false;
	int linenum_before = line;

	tok = Parser::GetNextToken(in, line);
	if(tok != PUT && tok != PUTLN) {
		ParseError(line, "Putline or put is wrong.");
		return false;
	}

	tok = Parser::GetNextToken(in, line);
	if(tok != LPAREN) {
		ParseError(line, "Missing Left Parenthesis");
		return false;
	}
	status = Expr(in, line);
	if(!status) {
		ParseError(line, "Incorrect variable definition.@@");
		return false;
	}

	tok = Parser::GetNextToken(in, line);
	if(tok != RPAREN) {
		ParseError(line, "Missing Right Parenthesis");
		return false;
	}
	tok = Parser::GetNextToken(in, line);
	if(tok != SEMICOL) {
		Parser::PushBackToken(tok);
		if (linenum_before != tok.GetLinenum()) {
			endOfLineError = true;
		}
		ParseError(endOfLineError ? line-1 : line, "Missing semicolon at end of statement");
		return false;
	}
	return true;
}
bool GetStmt(istream& in, int& line){
	LexItem tok;
	bool status = false;

	tok = Parser::GetNextToken(in, line);
	if(tok != GET) {
		ParseError(line, "Not a GET");
		return false;
	}

	tok = Parser::GetNextToken(in, line);
	if(tok != LPAREN) {
		ParseError(line, "Not a left parenth");
		return false;
	}

	status = Var(in, line);
	if(!status) {
		ParseError(line, "Incorrect variable definition. ^^");
		return false;
	}
	tok = Parser::GetNextToken(in, line);
	if(tok != RPAREN) {
		ParseError(line, "Not a right parenth");
		return false;
	}

	tok = Parser::GetNextToken(in, line);
	if(tok != SEMICOL) {
		ParseError(line, "Not a semicolon ##");
		return false;
	}
	return true;
}
bool IfStmt(istream& in, int& line){
	LexItem tok;
	bool status = false;

	tok = Parser::GetNextToken(in, line);
	if(tok != IF && tok != ELSIF) {
		ParseError(line, "Not a IF");
		return false;
	}

	status = Expr(in, line);

	if(!status) {
		ParseError(line, "Incorrect expression statement.");
		return false;
	}

	tok = Parser::GetNextToken(in, line);
	if(tok != THEN) {
		ParseError(line, "Not a THEN");
		return false;
	}

	status = StmtList(in, line);
	if(!status) {
		ParseError(line, "Incorrect statement list. !");
		return false;
	}

	tok = Parser::GetNextToken(in, line);
	if(tok == ELSIF) {
		Parser::PushBackToken(tok);
		return IfStmt(in, line);
	}

	if(tok == ELSE) {
		status = StmtList(in, line);
		if(!status) {
			ParseError(line, "Incorrect statement list. !");
			return false;
		}
	}

	if(tok == END) {
		tok = Parser::GetNextToken(in, line);
		if(tok == IF) {
			tok = Parser::GetNextToken(in, line);
			if (tok == SEMICOL) {
				return true;
			}
		}
	} else {
		ParseError(line, "Something went wrong here!");
		return false;
	}

	return false;
}
bool AssignStmt(istream& in, int& line){
	bool status = false;
	LexItem tok;

	status = Var(in, line);
	if(!status) {
		return false;
	}

	status = Expr(in, line);
	if(!status) {
		ParseError(line, "Missing Expression in Assignment Statement");
		return false;
	}

	tok = Parser::GetNextToken(in, line);
	if (tok != SEMICOL) {
		ParseError(line, "Incorrect semicolon found@.");
		return false;
	}

	return true;
}
bool Var(istream& in, int& line){
	LexItem tok;
	tok = Parser::GetNextToken(in, line);
	if(tok != IDENT) {
		ParseError(line, "Incorrect variable definition.$$");
		return false;
	}
	return true;
}
bool Expr(istream& in, int& line){
	LexItem tok;
	bool status = false;

	status = Relation(in, line);
	if(!status) {
		ParseError(line, "Incorrect relatioon statement.");
		return false;
	}

	tok = Parser::GetNextToken(in, line);
	if(tok == AND || tok == OR) {
		return Expr(in, line);
	}

	Parser::PushBackToken(tok);

	return true;
}
bool Relation(istream& in, int& line){
	LexItem tok;
	bool status = false;

	status = SimpleExpr(in, line);
	if(!status) {
		ParseError(line, "Incorrect simple expression statement.1");
		return false;
	}

	tok = Parser::GetNextToken(in, line);

	if(tok == EQ || tok == NEQ || tok == LTHAN || tok == LTE || tok == GTHAN || tok == GTE) {
		status = SimpleExpr(in, line);
		if(!status) {
			ParseError(line, "Missing operand after operator");
			return false;
		}
	} else {
		Parser::PushBackToken(tok);
	}

	return true;
}
extern bool SimpleExpr(istream& in, int& line){
	LexItem tok;
	bool status = false;

	status = STerm(in, line);
	if(!status) {
		ParseError(line, "Incorrect S term statement.");
		return false;
	}

	tok = Parser::GetNextToken(in, line);
	if(tok == PLUS || tok == MINUS || tok == CONCAT) {
		return SimpleExpr(in, line);
	}

	Parser::PushBackToken(tok);

	return true;
}
extern bool STerm(istream& in, int& line){
	LexItem tok;
	bool status = false;

	tok = Parser::GetNextToken(in, line);
	if(tok == PLUS || tok == MINUS) {
		status = Term(in, line, tok.GetToken());
		if(!status) {
			ParseError(line, "Incorrect T-Term statement.");
			return false;
		}
	} else {
		Parser::PushBackToken(tok);
		status = Term(in, line, PLUS);
		if(!status) {
			ParseError(line, "Incorrect T-Term statement.");
			return false;
		}
	}
	return true;
}
extern bool Term(istream& in, int& line, int sign){
	LexItem tok;
	bool status = false;

	status = Factor(in, line, sign);
	if(!status) {
		ParseError(line, "Incorrect factor statement.");
		return false;
	}
	tok = Parser::GetNextToken(in, line);
	if(tok == MULT || tok == DIV || tok == MOD) {
		return Term(in, line, sign);
	}

	Parser::PushBackToken(tok);

	return true;
}
extern bool Factor(istream& in, int& line, int sign){
	LexItem tok;
	bool status = false;

	if (tok == NOT) {
		status = Primary(in, line, tok.GetToken());
		if(!status) {
			ParseError(line, "Incorrect primary expression statement.");
			return false;
		}
		return true;
	}

	status = Primary(in, line, sign);
	if(!status) {
		ParseError(line, "Incorrect factor statement.");
		return false;
	}
	tok = Parser::GetNextToken(in, line);
	if (tok == EXP) {
		tok = Parser::GetNextToken(in, line);
		if (tok == PLUS || tok == MINUS) {
			status = Primary(in, line, tok.GetToken());
			if(!status) {
				ParseError(line, "Incorrect primary expression statement.");
				return false;
			}
		} else {
			Parser::PushBackToken(tok);
			status = Primary(in, line, PLUS);
			if(!status) {
				ParseError(line, "Incorrect primary expression statement.");
				return false;
			}
		}
	} else {
		Parser::PushBackToken(tok);
	}
	return true;
}
extern bool Primary(istream& in, int& line, int sign){
	LexItem tok;
	bool status = false;

	tok = Parser::GetNextToken(in, line);
	if(tok == ICONST || tok == BCONST || tok == FCONST || tok == SCONST || tok == CCONST) {
		return true;
	}

	if(tok == LPAREN) {
		status = Expr(in, line);
		if(!status) {
			ParseError(line, "Incorrect expression2 statement.");
			return false;
		}
		tok = Parser::GetNextToken(in, line);
		if(tok != RPAREN) {
			ParseError(line, "Incorrect Right Parenthesis statement.");
			return false;
		}
		return true;
	}

	Parser::PushBackToken(tok);

	status = Name(in, line);
	if(!status) {
		ParseError(line, "Incorrect operand statement.");
		return false;
	}

	return true;
}
extern bool Name(istream& in, int& line){
	LexItem tok;
	bool status = false;

	tok = Parser::GetNextToken(in, line);
	if(tok != IDENT) {
		ParseError(line, "Incorrect identifier statement. %%" + tok.GetLexeme());
		return false;
	}

	tok = Parser::GetNextToken(in, line);
	if(tok == LPAREN) {
		status = Range (in, line);
		if(!status) {
			ParseError(line, "Incorrect range 22statement.");
			return false;
		}
		tok = Parser::GetNextToken(in, line);
		if(tok != RPAREN) {
			ParseError(line, "Incorrect right parenthesis2 statement.");
			return false;
		}
	} else {
		Parser::PushBackToken(tok);
	}

	return true;
}
extern bool Range(istream& in, int& line){
	LexItem tok;
	bool status = false;

	status = SimpleExpr(in, line);
	if(!status) {
		ParseError(line, "Incorrect simple expression43 statement.");
		return false;
	}

	tok = Parser::GetNextToken(in, line);
	if(tok == DOT) {
		tok = Parser::GetNextToken(in, line);
		if(tok != DOT) {
			ParseError(line, "Incorrect dot placement statement.");
			return false;
		}
		status = SimpleExpr(in, line);
		if(!status) {
			ParseError(line, "Incorrect simple expression433 statement.");
		}
	} else {
		Parser::PushBackToken(tok);
	}
	return true;
}