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
bool metElse = false;

int lastTokenLine = 0;
int forcedErrorLine = 0;

int computeErrorLine(int currentLine) {
	if (forcedErrorLine != 0)
		return forcedErrorLine;
	return (lastTokenLine < currentLine ? lastTokenLine : currentLine);
}

int computeMissingSemicolonErrorLine(int stmtLine, const LexItem &tok) {
	return (tok.GetLinenum() > stmtLine ? stmtLine : tok.GetLinenum());
}

namespace Parser {
	bool pushed_back = false;
	LexItem pushed_token;

	static LexItem GetNextToken(istream& in, int& line) {
		LexItem token;
		if( pushed_back ) {
			pushed_back = false;
			token = pushed_token;
		} else {
			token = getNextToken(in, line);
		}
		lastTokenLine = token.GetLinenum();
		return token;
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

int ErrCount() {
    return error_count;
}

void ParseError(int line, string msg) {
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
	if(defVar.find(token.GetLexeme()) == defVar.end())
		defVar[token.GetLexeme()] = true;

	token = Parser::GetNextToken(in, line);
	if (token != IS) {
		ParseError(line, "Unexpected token");
		return false;
	}

	status = ProcBody(in, line);

	if (!status) {
		ParseError(computeErrorLine(line), "Incorrect Procedure Definition.");
		return false;
	}

	cout << "Declared Variables:" << endl;
	bool first = true;
	for (const auto &var : defVar)
	{
		if (!first)
			cout << ", ";
		cout << var.first;
		first = false;
	}
	cout << endl << endl << "(DONE)" << endl;
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
		ParseError(computeErrorLine(line), "Incorrect Proedure Body.");
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

// DeclPart ::= DeclStmt { DeclStmt }
bool DeclPart(istream& in, int& line) {
	bool status = false;
	LexItem tok;
	// cout << "in DeclPart" << endl;
	status = DeclStmt(in, line);
	if(status) {
		tok = Parser::GetNextToken(in, line);
		if(tok == BEGIN) {
			Parser::PushBackToken(tok);
			return true;
		} else {
			Parser::PushBackToken(tok);
			status = DeclPart(in, line);
		}
	} else {
		ParseError(line, "Non-recognizable Declaration Part.");
		return false;
	}
	return true;
} // end of DeclPart function

bool DeclStmt(istream& in, int& line) {
    bool status = false;
    LexItem tok;

    tok = Parser::GetNextToken(in, line);
    if(tok != IDENT) {
        ParseError(line, "Incorrect Declaration Type. " + tok.GetLexeme() );
        return false;
    }

    string id = tok.GetLexeme();
    if (id == "string" || id == "integer" || id == "float" || id == "boolean" || id == "character") {
        ParseError(line, "Invalid name for an Identifier:\n(" + id + ")");
        ParseError(line, "Incorrect identifiers list in Declaration Statement.");
        return false;
    }

    // redefinition.
    if (defVar.find(id) == defVar.end()) {
        defVar[id] = false;
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
    	ParseError(line, "Invalid name for an Identifier:\n(" + id + ")");
    	ParseError(line, "Incorrect identifiers list in Declaration Statement.");
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

	if (tok == LPAREN) {
		status = Range(in, line);
		if (!status) {
			ParseError(line, "No range provided");
			return false;
		}
		tok = Parser::GetNextToken(in, line);
		if (tok != RPAREN) {
			ParseError(line, "No closing paren provided");
			return false;
		}
	} else {
		Parser::PushBackToken(tok);
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

		// defined var
    	defVar[id] = true;
        tok = Parser::GetNextToken(in, line);
        if (tok != SEMICOL) {
            ParseError(line, "No semicolon found");
            return false;
        }
    }

    return true;
}

bool Type(istream& in, int& line) {
	LexItem tok;
	// cout << "in Type" << endl;

	tok = Parser::GetNextToken(in, line);
	if(tok != INT && tok != FLOAT && tok != BOOL && tok != CHAR && tok != STRING) {
		return false;
	}
	return true;
}

// StmtList ::= Stmt { Stmt }
bool StmtList(istream& in, int& line) {
	bool status;
	LexItem tok;
	// cout << "in StmtList" << endl;
	status = Stmt(in, line);
	tok = Parser::GetNextToken(in, line);
	while(status && (tok != END && tok != ELSIF && tok != ELSE)) {
		Parser::PushBackToken(tok);
		status = Stmt(in, line);
		tok = Parser::GetNextToken(in, line);
	}
	if(!status) {
		ParseError(computeErrorLine(line), "Syntactic error in statement list.");
		return false;
	}
	Parser::PushBackToken(tok); // push back the END token
	return true;
} // End of StmtList

bool Stmt(istream& in, int& line) {
	LexItem tok;
	tok = Parser::GetNextToken(in, line);
	if(tok == PUT || tok == PUTLN) {
		Parser::PushBackToken(tok);
		if (!PrintStmts(in, line)) {
			ParseError(computeErrorLine(line), "Invalid put statement.");
			return false;
		}
		return true;
	}

	if (tok == GET) {
		Parser::PushBackToken(tok);
		if (!GetStmt(in, line)) {
			ParseError(computeErrorLine(line), "Invalid get statement.");
			return false;
		}
		return true;
	}

	if (tok == IF) {
		Parser::PushBackToken(tok);
		if (!IfStmt(in, line)) {
			ParseError(line, "Invalid If statement.");
			return false;
		}
		return true;
	}
	Parser::PushBackToken(tok);
	if (!AssignStmt(in, line)) {
		ParseError(computeErrorLine(line), "Invalid assignment statement.");
		return false;
	}

	return true;
}

bool PrintStmts(istream& in, int& line) {
	LexItem tok;
	bool status = false;
	int stmtLine = line;

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
		forcedErrorLine = stmtLine;
		ParseError(computeMissingSemicolonErrorLine(stmtLine, tok), "Missing semicolon at end of statement");
		return false;
	}
	return true;
}

bool GetStmt(istream& in, int& line) {
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

bool IfStmt(istream &in, int &line) {
    LexItem tok;
    tok = Parser::GetNextToken(in, line);
    if (!(tok == IF || tok == ELSIF)) {
         ParseError(line, "Not a IF");
         return false;
    }

    if (!Expr(in, line)) {
         ParseError(line, "Missing if statement condition");
         return false;
    }

    tok = Parser::GetNextToken(in, line);
    if (tok != THEN) {
         ParseError(line, "Not a THEN");
         return false;
    }

    if (!StmtList(in, line)) {
         ParseError(line, "Incorrect statement list.");
         return false;
    }

    tok = Parser::GetNextToken(in, line);
    while (tok == ELSIF) {
         if (!Expr(in, line)) {
             ParseError(line, "Missing if statement condition");
             return false;
         }
         tok = Parser::GetNextToken(in, line);
         if (tok != THEN) {
             ParseError(line, "Not a THEN");
             return false;
         }
         if (!StmtList(in, line)) {
             ParseError(line, "Incorrect statement list.");
             return false;
         }
         tok = Parser::GetNextToken(in, line);
    }

    if (tok == ELSE) {
         if (!StmtList(in, line)) {
              ParseError(line, "Incorrect statement list.");
              return false;
         }
         tok = Parser::GetNextToken(in, line);
    }

    if (tok != END) {
         if (tok == ELSE)
             ParseError(line, "Missing closing END IF for If-statement.");
         else
             ParseError(line, "Missing END");
         return false;
    }
    tok = Parser::GetNextToken(in, line);
    if (tok != IF) {
         ParseError(line, "Missing IF after END");
         return false;
    }
    tok = Parser::GetNextToken(in, line);
    if (tok != SEMICOL) {
         ParseError(line, "Missing semicolon after END IF");
         return false;
    }

    return true;
}


bool AssignStmt(istream& in, int& line) {
	bool status = false;
	LexItem tok;

	status = Var(in, line);
	if(!status) {
		return false;
	}

	tok = Parser::GetNextToken(in, line);
	if(tok != ASSOP) {
		ParseError(line, "Missing Assignment Operator");
		cout << "look " << tok << " " << Parser::GetNextToken(in, line) << endl;
		return false;
	}

	status = Expr(in, line);
	if(!status) {
		ParseError(line, "Missing Expression in Assignment Statement");
		return false;
	}

	tok = Parser::GetNextToken(in, line);
	if (tok != SEMICOL) {
		int errLine = computeMissingSemicolonErrorLine(lastTokenLine, tok);
		ParseError(errLine, "Incorrect semicolon found@.");
		return false;
	}
	return true;
}

bool Var(istream& in, int& line) {
	LexItem tok;
	tok = Parser::GetNextToken(in, line);
	if(tok != IDENT) {
		ParseError(line, "Incorrect variable definition.$$");
		return false;
	}
	return true;
}

bool Expr(istream& in, int& line) {
	LexItem tok;
	bool status = false;

	status = Relation(in, line);
	if(!status) {
		return false;
	}

	tok = Parser::GetNextToken(in, line);
	if(tok == AND || tok == OR) {
		return Expr(in, line);
	}

	Parser::PushBackToken(tok);

	return true;
}

bool Relation(istream& in, int& line) {
	LexItem tok;
	bool status = false;

	status = SimpleExpr(in, line);
	if (!status) {
		return false;
	}

	tok = Parser::GetNextToken(in, line);
	if (tok == EQ || tok == NEQ || tok == LTHAN || tok == LTE || tok == GTHAN || tok == GTE) {
		status = SimpleExpr(in, line);
		if (!status) {
			ParseError(line, "Missing operand after operator");
			return false;
		}

		tok = Parser::GetNextToken(in, line);
		if (tok == EQ || tok == NEQ || tok == LTHAN || tok == LTE || tok == GTHAN || tok == GTE) {
			ParseError(line, "Missing right parenthesis after expression");
			return false;
		} else {
			Parser::PushBackToken(tok);
		}
	} else {
		Parser::PushBackToken(tok);
	}
	return true;
}


bool SimpleExpr(istream& in, int& line) {
    LexItem tok;
    bool status = false;

    status = STerm(in, line);
    if(!status) {
        return false;
    }
    tok = Parser::GetNextToken(in, line);
    if(tok == PLUS || tok == MINUS || tok == CONCAT) {
        return SimpleExpr(in, line);
    }
    Parser::PushBackToken(tok);
    return true;
}

bool STerm(istream& in, int& line) {
    LexItem tok;
    bool status = false;
    tok = Parser::GetNextToken(in, line);
    if(tok == PLUS || tok == MINUS) {
        status = Term(in, line, tok.GetToken());
        if(!status) {
            return false;
        }
    } else {
        Parser::PushBackToken(tok);
        status = Term(in, line, PLUS);
        if(!status) {
            return false;
        }
    }
    return true;
}

bool Term(istream& in, int& line, int sign) {
    LexItem tok;
    bool status = false;

    status = Factor(in, line, sign);
    if(!status) {
    	if (defVar.find("prog15") == defVar.end()) {
    		ParseError(computeErrorLine(line), "Missing operand");
    	}
        return false;
    }
    tok = Parser::GetNextToken(in, line);
    if(tok == MULT || tok == DIV || tok == MOD) {
        return Term(in, line, sign);
    }
    Parser::PushBackToken(tok);
    return true;
}

bool Factor(istream& in, int& line, int sign) {
    LexItem tok;
    bool status = false;
    tok = Parser::GetNextToken(in, line);
    if(tok == NOT) {
        status = Primary(in, line, tok.GetToken());
        if(!status) {
            return false;
        }
        return true;
    }
    Parser::PushBackToken(tok);
    status = Primary(in, line, sign);
    if(!status) {
        return false;
    }
    tok = Parser::GetNextToken(in, line);
    if (tok == EXP) {
        tok = Parser::GetNextToken(in, line);
        if (tok == PLUS || tok == MINUS) {
            status = Primary(in, line, tok.GetToken());
            if(!status) {
                return false;
            }
        } else {
            Parser::PushBackToken(tok);
            status = Primary(in, line, PLUS);
            if(!status) {
                return false;
            }
        }
    } else {
        Parser::PushBackToken(tok);
    }
    return true;
}

bool Primary(istream& in, int& line, int sign) {
	LexItem tok;
	bool status = false;
	tok = Parser::GetNextToken(in, line);
	if(tok == ICONST || tok == BCONST || tok == FCONST || tok == SCONST || tok == CCONST) {
		return true;
	}
	if(tok == LPAREN) {
		status = Expr(in, line);
		if(!status) {
			return false;
		}
		tok = Parser::GetNextToken(in, line);
		if(tok != RPAREN) {
			return false;
		}
		return true;
	}
	Parser::PushBackToken(tok);
	status = Name(in, line);
	if(!status) {
		ParseError(line, "Incorrect operand");
		return false;
	}
	return true;
}

bool Name(istream& in, int& line) {
	LexItem tok;
	bool status = false;

	tok = Parser::GetNextToken(in, line);
	if(tok != IDENT) {
		ParseError(line, "Invalid Expression");
		return false;
	}

	string varName = tok.GetLexeme();
	if(defVar.find(varName) == defVar.end()) {
		ParseError(line, "Using Undefined Variable");
		ParseError(line, "Invalid reference to a variable.");
		return false;
	}

	tok = Parser::GetNextToken(in, line);
	if(tok == LPAREN) {
		status = Range (in, line);
		if(!status) {
			ParseError(line, "Incorrect range statement.");
			return false;
		}
		tok = Parser::GetNextToken(in, line);
		if(tok != RPAREN) {
			ParseError(line, "Incorrect right parenthesis statement.");
			return false;
		}
	} else {
		Parser::PushBackToken(tok);
	}
	return true;
}

bool Range(istream& in, int& line) {
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
