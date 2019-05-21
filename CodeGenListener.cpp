//////////////////////////////////////////////////////////////////////
//
//    CodeGenListener - Walk the parser tree to do
//                             the generation of code
//
//    Copyright (C) 2018  Universitat Politecnica de Catalunya
//
//    This library is free software; you can redistribute it and/or
//    modify it under the terms of the GNU General Public License
//    as published by the Free Software Foundation; either version 3
//    of the License, or (at your option) any later version.
//
//    This library is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//    Affero General Public License for more details.
//
//    You should have received a copy of the GNU Affero General Public
//    License along with this library; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
//
//    contact: José Miguel Rivero (rivero@cs.upc.edu)
//             Computer Science Department
//             Universitat Politecnica de Catalunya
//             despatx Omega.110 - Campus Nord UPC
//             08034 Barcelona.  SPAIN
//
//////////////////////////////////////////////////////////////////////

#include "CodeGenListener.h"

#include "antlr4-runtime.h"

#include "../common/TypesMgr.h"
#include "../common/SymTable.h"
#include "../common/TreeDecoration.h"
#include "../common/code.h"

#include <cstddef>    // std::size_t

// uncomment the following line to enable debugging messages with DEBUG*
// #define DEBUG_BUILD
#include "../common/debug.h"

// using namespace std;


// Constructor
CodeGenListener::CodeGenListener(TypesMgr       & Types,
				 SymTable       & Symbols,
				 TreeDecoration & Decorations,
				 code           & Code) :
  Types{Types},
  Symbols{Symbols},
  Decorations{Decorations},
  Code{Code} {
}

void CodeGenListener::enterProgram(AslParser::ProgramContext *ctx) {
  DEBUG_ENTER();
  SymTable::ScopeId sc = getScopeDecor(ctx);
  Symbols.pushThisScope(sc);
}
void CodeGenListener::exitProgram(AslParser::ProgramContext *ctx) {
  Symbols.popScope();
  DEBUG_EXIT();
}

void CodeGenListener::enterFunction(AslParser::FunctionContext *ctx) {
  DEBUG_ENTER();
  subroutine subr(ctx->ID()->getText());
  Code.add_subroutine(subr);
  //Aqui estem modificant els parametres per en cas de que retorni alguna cosa, afegeix la variable _result
  subroutine & subrRef = Code.get_last_subroutine();
  if(ctx->return_type()) subrRef.add_param("_result");
  
  SymTable::ScopeId sc = getScopeDecor(ctx);
  Symbols.pushThisScope(sc);
  codeCounters.reset();
}
void CodeGenListener::exitFunction(AslParser::FunctionContext *ctx) {
  subroutine & subrRef = Code.get_last_subroutine();
  instructionList code = getCodeDecor(ctx->statements());
  code = code || instruction::RETURN();
  subrRef.set_instructions(code);
  
  Symbols.popScope();
  DEBUG_EXIT();
}

void CodeGenListener::enterDeclarations(AslParser::DeclarationsContext *ctx) {
  DEBUG_ENTER();
}
void CodeGenListener::exitDeclarations(AslParser::DeclarationsContext *ctx) {
  DEBUG_EXIT();
}

void CodeGenListener::enterParameter_decl(AslParser::Parameter_declContext *ctx){
  DEBUG_ENTER();
}
void CodeGenListener::exitParameter_decl(AslParser::Parameter_declContext *ctx){
   subroutine       & subrRef = Code.get_last_subroutine();
   for(auto ipdObj: ctx->pdObj()){
     //TypesMgr::TypeId        t1 = getTypeDecor(ipdObj->type());
     //std::size_t           size = Types.getSizeOfType(t1);
    // subrRef.add_var(ipdObj->ID()->getText(), size);           //add_var ha de fer algo pero amb parametres
     subrRef.add_param(ipdObj->ID()->getText()); 
   }
   
    DEBUG_EXIT();
}





void CodeGenListener::enterVariable_decl(AslParser::Variable_declContext *ctx) {
  DEBUG_ENTER();
}
void CodeGenListener::exitVariable_decl(AslParser::Variable_declContext *ctx) {
  for(auto i : ctx->ID()) {
    subroutine       & subrRef = Code.get_last_subroutine();
    TypesMgr::TypeId        t1 = getTypeDecor(ctx->type());
    std::size_t           size = Types.getSizeOfType(t1);
    subrRef.add_var(i->getText(), size);
  }
  DEBUG_EXIT();
}

void CodeGenListener::enterType(AslParser::TypeContext *ctx) {
  DEBUG_ENTER();
}
void CodeGenListener::exitType(AslParser::TypeContext *ctx) {
  DEBUG_EXIT();
}

void CodeGenListener::enterStatements(AslParser::StatementsContext *ctx) {
  DEBUG_ENTER();
}
void CodeGenListener::exitStatements(AslParser::StatementsContext *ctx) {
  instructionList code;
  for (auto stCtx : ctx->statement()) {
    code = code || getCodeDecor(stCtx);
  }
  putCodeDecor(ctx, code);
  DEBUG_EXIT();
}

void CodeGenListener::enterAssignStmt(AslParser::AssignStmtContext *ctx) {
  DEBUG_ENTER();
}
void CodeGenListener::exitAssignStmt(AslParser::AssignStmtContext *ctx) {
  instructionList  code;

  if(ctx->left_expr()->expr()){ //IS ARRAY
    std::string     addrA = getAddrDecor(ctx->left_expr());
    std::string     offsA = getOffsetDecor(ctx->left_expr());
    instructionList code1 = getCodeDecor(ctx->left_expr());

    std::string     addrE = getAddrDecor(ctx->expr());
    instructionList code2 = getCodeDecor(ctx->expr());

    code = code1 || code2 || instruction::XLOAD(addrA, offsA, addrE);

  }
  else{
    std::string     addr1 = getAddrDecor(ctx->left_expr());
    // std::string     offs1 = getOffsetDecor(ctx->left_expr());
    instructionList code1 = getCodeDecor(ctx->left_expr());
    // TypesMgr::TypeId tid1 = getTypeDecor(ctx->left_expr());
    std::string     addr2 = getAddrDecor(ctx->expr());
    // std::string     offs2 = getOffsetDecor(ctx->expr());
    instructionList code2 = getCodeDecor(ctx->expr());
    // TypesMgr::TypeId tid2 = getTypeDecor(ctx->expr());
    code = code1 || code2 || instruction::LOAD(addr1, addr2);
  }


  putCodeDecor(ctx, code);
  DEBUG_EXIT();
}

void CodeGenListener::enterArray_read(AslParser::Array_readContext *ctx){
  DEBUG_ENTER();
}
void CodeGenListener::exitArray_read(AslParser::Array_readContext * ctx){
  instructionList code;
  std::string addrA = getAddrDecor(ctx->ident());
  instructionList codeE = getCodeDecor(ctx->expr());
  std::string addrE = getAddrDecor(ctx->expr());
   
  std::string temp = "%"+codeCounters.newTEMP();
  code = codeE || instruction::LOAD(temp, "1");
  code = code || instruction::MUL(temp, addrE, temp);

  code = code || instruction::LOADX(temp, addrA, temp);
  
  putCodeDecor(ctx, code);
  putAddrDecor(ctx, temp);
  putOffsetDecor(ctx, "");
  DEBUG_EXIT();
}

void CodeGenListener::enterIfStmt(AslParser::IfStmtContext *ctx) {
  DEBUG_ENTER();
}
void CodeGenListener::exitIfStmt(AslParser::IfStmtContext *ctx) {
  instructionList   code;
  std::string      addr1 = getAddrDecor(ctx->expr());
  instructionList  code1 = getCodeDecor(ctx->expr());
  instructionList  code2 = getCodeDecor(ctx->statements(0));
  
  std::string      label2 = codeCounters.newLabelIF();
  std::string      label3 = "else"+label2;
  std::string labelEndIf = "endif"+label2;
  if(ctx->ELSE()){
    instructionList  code3 = getCodeDecor(ctx->statements(1));
    code = code1 || instruction::FJUMP(addr1, label3) ||
         code2 || instruction::UJUMP(labelEndIf) || instruction::LABEL(label3)        ||
         code3 || instruction::LABEL(labelEndIf);
  }
  else{
    code = code1 || instruction::FJUMP(addr1, labelEndIf) ||
         code2 || instruction::LABEL(labelEndIf);
  }
  
  putCodeDecor(ctx, code);
  DEBUG_EXIT();
}

/*
void CodeGenListener::exitIfStmt(AslParser::IfStmtContext *ctx) {
  instructionList   code;
  std::string      addr1 = getAddrDecor(ctx->expr());
  instructionList  code1 = getCodeDecor(ctx->expr());
  instructionList  code2 = getCodeDecor(ctx->statements());
  std::string      label = codeCounters.newLabelIF();
  std::string labelEndIf = "endif"+label;
  code = code1 || instruction::FJUMP(addr1, labelEndIf) ||
         code2 || instruction::LABEL(labelEndIf);
  putCodeDecor(ctx, code);
  DEBUG_EXIT();
}
*/
void CodeGenListener::enterWhileStmt(AslParser::WhileStmtContext *ctx){
  DEBUG_ENTER();
}
void CodeGenListener::exitWhileStmt(AslParser::WhileStmtContext *ctx){
  instructionList   code;
  std::string       addr1 = getAddrDecor(ctx->expr());
  instructionList   code1 = getCodeDecor(ctx->expr());
  instructionList   code2 = getCodeDecor(ctx->statements());
  std::string       label = codeCounters.newLabelWHILE();
  std::string labelEndWhile = "endwhile"+label;
  std::string labelWhile = "while" + label;
  code = instruction::LABEL(labelWhile) || code1 || instruction::FJUMP(addr1, labelEndWhile) ||
        code2  || instruction::UJUMP(labelWhile) || instruction::LABEL(labelEndWhile);
  putCodeDecor(ctx, code);
  DEBUG_EXIT();
}

void CodeGenListener::enterProcCall(AslParser::ProcCallContext *ctx) {
  DEBUG_ENTER();
}
void CodeGenListener::exitProcCall(AslParser::ProcCallContext *ctx) {
  instructionList code;
  for(auto p : ctx->expr()){
    code = code || getCodeDecor(p);
  }
  for(auto p : ctx->expr()){
    code = code || instruction::PUSH(getAddrDecor(p));
  }
  // std::string name = ctx->ident()->ID()->getSymbol()->getText();
  std::string name = ctx->ident()->getText();
  code = code || instruction::CALL(name);
  for(auto p : ctx->expr()){
    code = code || instruction::POP();
  }
  putCodeDecor(ctx, code);
  DEBUG_EXIT();
}


void CodeGenListener::enterReturn_func(AslParser::Return_funcContext *ctx){
  DEBUG_ENTER();
}
void CodeGenListener::exitReturn_func(AslParser::Return_funcContext *ctx){
  instructionList code;
  int i = 0;
  for(auto p : ctx->expr()){
    code = code || getCodeDecor(p);
    std::vector<TypesMgr::TypeId> vecTy = Types.getFuncParamsTypes(getTypeDecor(ctx->ident()));
    if(Types.isIntegerTy(getTypeDecor(p)) and Types.isFloatTy(vecTy[i])){
      std::string tempF = "%"+codeCounters.newTEMP();
      std::string addrE = getAddrDecor(p);
      code = code || instruction::FLOAT(tempF,addrE);
      putAddrDecor(p, tempF);
    }
    i++;
  }
  code = code || instruction::PUSH();
  for(auto p : ctx->expr()){
    code = code || instruction::PUSH(getAddrDecor(p));
  }
  // std::string name = ctx->ident()->ID()->getSymbol()->getText();
  std::string name = ctx->ident()->getText();
  code = code || instruction::CALL(name);
  for(auto p : ctx->expr()){
    code = code || instruction::POP();
  }
  std::string temp = "%"+codeCounters.newTEMP();
  code = code || instruction::POP(temp);
  putCodeDecor(ctx, code);
  putAddrDecor(ctx,temp);
  DEBUG_EXIT();
}

void CodeGenListener::enterReturn(AslParser::ReturnContext *ctx){
  DEBUG_ENTER();
}
void CodeGenListener::exitReturn(AslParser::ReturnContext * ctx){
  instructionList code;
  std::string       addr1 = getAddrDecor(ctx->expr());
  std::string temp;
  if(ctx->expr()){
    code = getCodeDecor(ctx->expr());
    subroutine       & subrRef = Code.get_last_subroutine();
    temp = (subrRef.params.begin()) -> name;
    code = code || instruction::LOAD(temp, addr1) || instruction::RETURN();
  }
  putCodeDecor(ctx,code);
  putOffsetDecor(ctx,"");
  putAddrDecor(ctx,temp);
  DEBUG_EXIT();
}





void CodeGenListener::enterReadStmt(AslParser::ReadStmtContext *ctx) {
  DEBUG_ENTER();
}
void CodeGenListener::exitReadStmt(AslParser::ReadStmtContext *ctx) {
  instructionList  code;
  std::string     addr1 = getAddrDecor(ctx->left_expr());
  // std::string     offs1 = getOffsetDecor(ctx->left_expr());
  instructionList code1 = getCodeDecor(ctx->left_expr());
  // TypesMgr::TypeId tid1 = getTypeDecor(ctx->left_expr());
  code = code1 || instruction::READI(addr1);
  putCodeDecor(ctx, code);
  DEBUG_EXIT();
}

void CodeGenListener::enterWriteExpr(AslParser::WriteExprContext *ctx) {
  DEBUG_ENTER();
}
void CodeGenListener::exitWriteExpr(AslParser::WriteExprContext *ctx) {
  instructionList code;
  std::string     addr1 = getAddrDecor(ctx->expr());
  // std::string     offs1 = getOffsetDecor(ctx->expr());
  instructionList code1 = getCodeDecor(ctx->expr());
  TypesMgr::TypeId tid1 = getTypeDecor(ctx->expr());
  if(Types.isFloatTy(tid1))
    code = code1 || instruction::WRITEF(addr1);
  else code = code1 || instruction::WRITEI(addr1);
  putCodeDecor(ctx, code);
  DEBUG_EXIT();
}

void CodeGenListener::enterWriteString(AslParser::WriteStringContext *ctx) {
  DEBUG_ENTER();
}
void CodeGenListener::exitWriteString(AslParser::WriteStringContext *ctx) {
  instructionList code;
  std::string s = ctx->STRING()->getText();
  std::string temp = "%"+codeCounters.newTEMP();
  int i = 1;
  while (i < int(s.size())-1) {
    if (s[i] != '\\') {
      code = code ||
	     instruction::CHLOAD(temp, s.substr(i,1)) ||
	     instruction::WRITEC(temp);
      i += 1;
    }
    else {
      assert(i < int(s.size())-2);
      if (s[i+1] == 'n') {
        code = code || instruction::WRITELN();
        i += 2;
      }
      else if (s[i+1] == 't' or s[i+1] == '"' or s[i+1] == '\\') {
        code = code ||
               instruction::CHLOAD(temp, s.substr(i,2)) ||
	       instruction::WRITEC(temp);
        i += 2;
      }
      else {
        code = code ||
               instruction::CHLOAD(temp, s.substr(i,1)) ||
	       instruction::WRITEC(temp);
        i += 1;
      }
    }
  }
  putCodeDecor(ctx, code);
  DEBUG_EXIT();
}

void CodeGenListener::enterLeft_expr(AslParser::Left_exprContext *ctx) {
  DEBUG_ENTER();
}
void CodeGenListener::exitLeft_expr(AslParser::Left_exprContext *ctx) {
  if(ctx->expr()) { //Is Array
    instructionList code;
    code = code || getCodeDecor(ctx->expr());
    std::string  addrExp = getAddrDecor(ctx->expr());
    std::string temp = "%"+codeCounters.newTEMP();

    code = code || instruction::LOAD(temp, "1");
    
    code = code || instruction::MUL(temp, addrExp, temp);
    putOffsetDecor(ctx, temp);
    putCodeDecor(ctx, code);
    putAddrDecor(ctx, getAddrDecor(ctx->ident())); //CREC QUE AIXO NO ES FA SERVIR
  }
  else{
    putAddrDecor(ctx, getAddrDecor(ctx->ident()));
    putOffsetDecor(ctx, getOffsetDecor(ctx->ident()));
    putCodeDecor(ctx, getCodeDecor(ctx->ident()));
  }
 
  DEBUG_ENTER();
}

void CodeGenListener::enterPar(AslParser::ParContext *ctx){
  DEBUG_ENTER();
}
 
void CodeGenListener::exitPar(AslParser::ParContext *ctx){
  putAddrDecor(ctx, getAddrDecor(ctx->expr()));
  putOffsetDecor(ctx, "");//getOffsetDecor(ctx->expr()));
  putCodeDecor(ctx, getCodeDecor(ctx->expr()));
  DEBUG_EXIT();
}

void CodeGenListener::enterArithmetic(AslParser::ArithmeticContext *ctx) {
  DEBUG_ENTER();
}
void CodeGenListener::exitArithmetic(AslParser::ArithmeticContext *ctx) {
  std::string     addr1 = getAddrDecor(ctx->expr(0));
  instructionList code1 = getCodeDecor(ctx->expr(0));
  std::string     addr2 = getAddrDecor(ctx->expr(1));
  instructionList code2 = getCodeDecor(ctx->expr(1));
  instructionList code  = code1 || code2;
  TypesMgr::TypeId t1 = getTypeDecor(ctx->expr(0));
  TypesMgr::TypeId t2 = getTypeDecor(ctx->expr(1));
  TypesMgr::TypeId t  = getTypeDecor(ctx);
  std::string temp;
  if (ctx->MUL()){
    if(Types.isFloatTy(t)){
      std::string ftemp1 = addr1;
      std::string ftemp2 = addr2;
      if(not Types.isFloatTy(t1)){
        ftemp1 = "%"+codeCounters.newTEMP();
        code = code || instruction::FLOAT(ftemp1, addr1);
      }
      if(not Types.isFloatTy(t2)){
        ftemp2 = "%"+codeCounters.newTEMP();
        code = code || instruction::FLOAT(ftemp2, addr2);
      }
      temp = "%"+codeCounters.newTEMP();
      code = code || instruction::FMUL(temp, ftemp1, ftemp2);
    }
    else {
      temp = "%"+codeCounters.newTEMP();
      code = code || instruction::MUL(temp, addr1, addr2);
    }
  }
  else if(ctx->DIV()){
    if(Types.isFloatTy(t1)){
      std::string ftemp1 = addr1;
      std::string ftemp2 = addr2;
      
      if(not Types.isFloatTy(t1)){
        ftemp1 = "%"+codeCounters.newTEMP();
        code = code || instruction::FLOAT(ftemp1, addr1);
      }
      if(not Types.isFloatTy(t2)){
        ftemp2 = "%"+codeCounters.newTEMP();
        code = code || instruction::FLOAT(ftemp2, addr2);
      }
      temp = "%"+codeCounters.newTEMP();
      code = code || instruction::FDIV(temp, ftemp1, ftemp2);
    }
    else {
      temp = "%"+codeCounters.newTEMP();
      code = code || instruction::DIV(temp, addr1, addr2);
    }
  }
  else if(ctx->MOD()){
    //code = code || instruction::
  }
  else if(ctx->PLUS()){
    if(Types.isFloatTy(t)){
      std::string ftemp1 = addr1;
      std::string ftemp2 = addr2;
     if(not Types.isFloatTy(t1)){
        ftemp1 = "%"+codeCounters.newTEMP();
        code = code || instruction::FLOAT(ftemp1, addr1);
      }
      if(not Types.isFloatTy(t2)){
        ftemp2 = "%"+codeCounters.newTEMP();
        code = code || instruction::FLOAT(ftemp2, addr2);
      }
      temp = "%"+codeCounters.newTEMP();
      code = code || instruction::FADD(temp, ftemp1, ftemp2);
    }
    else {
      temp = "%"+codeCounters.newTEMP();
      code = code || instruction::ADD(temp, addr1, addr2);
    }
  }
  else if(ctx->MINUS()){
    if(Types.isFloatTy(t)){
      std::string ftemp1 = addr1;
      std::string ftemp2 = addr2;
      if(not Types.isFloatTy(t1)){
        ftemp1 = "%"+codeCounters.newTEMP();
        code = code || instruction::FLOAT(ftemp1, addr1);
      }
      if(not Types.isFloatTy(t2)){
        ftemp2 = "%"+codeCounters.newTEMP();
        code = code || instruction::FLOAT(ftemp2, addr2);
      }
      temp = "%"+codeCounters.newTEMP();
      code = code || instruction::FSUB(temp, ftemp1, ftemp2);
    }
    else {
      temp = "%"+codeCounters.newTEMP();
      code = code || instruction::SUB(temp, addr1, addr2);
    }
  }
  putAddrDecor(ctx, temp);
  putOffsetDecor(ctx, "");
  putCodeDecor(ctx, code);
  DEBUG_EXIT();
}

void CodeGenListener::enterLogic(AslParser::LogicContext *ctx){
  DEBUG_ENTER();
}
void CodeGenListener::exitLogic(AslParser::LogicContext *ctx){
  std::string     addr1 = getAddrDecor(ctx->expr(0));
  instructionList code1 = getCodeDecor(ctx->expr(0));
  std::string     addr2 = getAddrDecor(ctx->expr(1));
  instructionList code2 = getCodeDecor(ctx->expr(1));
  instructionList code  = code1 || code2;
  
  // TypesMgr::TypeId t1 = getTypeDecor(ctx->expr(0));
  // TypesMgr::TypeId t2 = getTypeDecor(ctx->expr(1));
  // TypesMgr::TypeId t  = getTypeDecor(ctx);
  std::string temp = "%"+codeCounters.newTEMP();
  if (ctx->AND()){
    code = code || instruction::AND(temp, addr1, addr2);
  }
  else if(ctx->OR()){
    code = code || instruction::OR(temp, addr1, addr2);
  }
  putAddrDecor(ctx, temp);
  putOffsetDecor(ctx, "");
  putCodeDecor(ctx, code);
  DEBUG_EXIT();
}

void CodeGenListener::enterNotplusminus(AslParser::NotplusminusContext *ctx){
  DEBUG_ENTER();
}
void CodeGenListener::exitNotplusminus(AslParser::NotplusminusContext *ctx){
  std::string     addr1 = getAddrDecor(ctx->expr());
  instructionList code1 = getCodeDecor(ctx->expr());
  instructionList code  = code1;
  std::string temp = "%"+codeCounters.newTEMP();
  if(ctx->NOT()){
    code = code || instruction::NOT(temp, addr1);
  }
  else if(ctx->PLUS()){    
    code = code;
  }
  else if(ctx->MINUS()){
    TypesMgr::TypeId t = getTypeDecor(ctx->expr());
    if(Types.isFloatTy(t)){
      code = code || instruction::FNEG(temp, addr1);
    }
    else code = code || instruction::NEG(temp, addr1);
  }
  putAddrDecor(ctx, temp);
  putOffsetDecor(ctx, "");
  putCodeDecor(ctx, code);  
  DEBUG_EXIT();
}


void CodeGenListener::enterRelational(AslParser::RelationalContext *ctx) {
  DEBUG_ENTER();
}
void CodeGenListener::exitRelational(AslParser::RelationalContext *ctx) {
  std::string     addr1 = getAddrDecor(ctx->expr(0));
  instructionList code1 = getCodeDecor(ctx->expr(0));
  std::string     addr2 = getAddrDecor(ctx->expr(1));
  instructionList code2 = getCodeDecor(ctx->expr(1));
  instructionList code  = code1 || code2;
  TypesMgr::TypeId t1 = getTypeDecor(ctx->expr(0));
  TypesMgr::TypeId t2 = getTypeDecor(ctx->expr(1));
  TypesMgr:: TypeId t  = getTypeDecor(ctx);
  std::string temp;
  
  if(ctx->EQUAL()){
    if(Types.isFloatTy(t1) or Types.isFloatTy(t2)){
      std::string ftemp1 = addr1;
      std::string ftemp2 = addr2;
      if(not Types.isFloatTy(t1)){
        ftemp1 = "%"+codeCounters.newTEMP();
        code = code || instruction::FLOAT(ftemp1, addr1);
      }
      if(not Types.isFloatTy(t2)){
        ftemp2 = "%"+codeCounters.newTEMP();
        code = code || instruction::FLOAT(ftemp2, addr2);
      }
      temp = "%"+codeCounters.newTEMP();
      code = code || instruction::FEQ(temp, ftemp1, ftemp2);
    }
    else {
      temp = "%"+codeCounters.newTEMP();
      code = code || instruction::EQ(temp, addr1, addr2);
    }
  }
  else if(ctx->NE()){
    if(Types.isFloatTy(t1) or Types.isFloatTy(t2)){
      std::string ftemp1 = addr1;
      std::string ftemp2 = addr2;
      if(not Types.isFloatTy(t1)){
        ftemp1 = "%"+codeCounters.newTEMP();
        code = code || instruction::FLOAT(ftemp1, addr1);
      }
      if(not Types.isFloatTy(t2)){
        ftemp2 = "%"+codeCounters.newTEMP();
        code = code || instruction::FLOAT(ftemp2, addr2);
      }
      temp = "%"+codeCounters.newTEMP();
      code = code || instruction::FEQ(temp, ftemp1, ftemp2);
      code = code || instruction::NOT(temp,temp);
    }
    else {
      temp = "%"+codeCounters.newTEMP();
      code = code || instruction::EQ(temp, addr1, addr2);
      code = code || instruction::NOT(temp,temp);
    }
     
  }
  else if(ctx->GT()){
    if(Types.isFloatTy(t1) or Types.isFloatTy(t2)){
      std::string ftemp1 = addr1;
      std::string ftemp2 = addr2;
      if(not Types.isFloatTy(t1)){
        ftemp1 = "%"+codeCounters.newTEMP();
        code = code || instruction::FLOAT(ftemp1, addr1);
      }
      if(not Types.isFloatTy(t2)){
        ftemp2 = "%"+codeCounters.newTEMP();
        code = code || instruction::FLOAT(ftemp2, addr2);
      }
      temp = "%"+codeCounters.newTEMP();
      code = code || instruction::FLE(temp, ftemp1, ftemp2);
      code = code || instruction::NOT(temp,temp);
    }
    else {
      temp = "%"+codeCounters.newTEMP();
      code = code || instruction::LE(temp, addr1, addr2);
      code = code || instruction::NOT(temp,temp);
    }  
    
  }
  else if(ctx->GE()){
    if(Types.isFloatTy(t1) or Types.isFloatTy(t2)){
      std::string ftemp1 = addr1;
      std::string ftemp2 = addr2;
      if(not Types.isFloatTy(t1)){
        ftemp1 = "%"+codeCounters.newTEMP();
        code = code || instruction::FLOAT(ftemp1, addr1);
      }
      if(not Types.isFloatTy(t2)){
        ftemp2 = "%"+codeCounters.newTEMP();
        code = code || instruction::FLOAT(ftemp2, addr2);
      }
      temp = "%"+codeCounters.newTEMP();
      code = code || instruction::FLT(temp, ftemp1, ftemp2);
      code = code || instruction::NOT(temp,temp);
    }
    else {
      temp = "%"+codeCounters.newTEMP();
      code = code || instruction::LT(temp, addr1, addr2);
      code = code || instruction::NOT(temp,temp);
    }
    

  }
  else if(ctx->LE()){
    if(Types.isFloatTy(t1) or Types.isFloatTy(t2)){
      std::string ftemp1 = addr1;
      std::string ftemp2 = addr2;
     if(not Types.isFloatTy(t1)){
        ftemp1 = "%"+codeCounters.newTEMP();
        code = code || instruction::FLOAT(ftemp1, addr1);
      }
      if(not Types.isFloatTy(t2)){
        ftemp2 = "%"+codeCounters.newTEMP();
        code = code || instruction::FLOAT(ftemp2, addr2);
      }
      temp = "%"+codeCounters.newTEMP();
      code = code || instruction::FLE(temp, ftemp1, ftemp2);
    }
    else {
      temp = "%"+codeCounters.newTEMP();
      code = code || instruction::LE(temp, addr1, addr2);
    }
    
  }
  else if(ctx->LT()){
    if(Types.isFloatTy(t1) or Types.isFloatTy(t2)){
      std::string ftemp1 = addr1;
      std::string ftemp2 = addr2;
      if(not Types.isFloatTy(t1)){
        ftemp1 = "%"+codeCounters.newTEMP();
        code = code || instruction::FLOAT(ftemp1, addr1);
      }
      if(not Types.isFloatTy(t2)){
        ftemp2 = "%"+codeCounters.newTEMP();
        code = code || instruction::FLOAT(ftemp2, addr2);
      }
      temp = "%"+codeCounters.newTEMP();
      code = code || instruction::FLT(temp, ftemp1, ftemp2);
    }
    else {
      temp = "%"+codeCounters.newTEMP();
      code = code || instruction::LT(temp, addr1, addr2);
    }   
  }
  
  putAddrDecor(ctx, temp);
  putOffsetDecor(ctx, "");
  putCodeDecor(ctx, code);
  DEBUG_EXIT();
}

void CodeGenListener::enterValue(AslParser::ValueContext *ctx) {
  DEBUG_ENTER();
}
void CodeGenListener::exitValue(AslParser::ValueContext *ctx) {
  instructionList code;
  std::string temp = "%"+codeCounters.newTEMP();
  TypesMgr::TypeId t1 = getTypeDecor(ctx);
  if(Types.isIntegerTy(t1)){
    code = instruction::ILOAD(temp, ctx->getText());
  }
  else if(Types.isFloatTy(t1)){
    code = instruction::FLOAD(temp, ctx->getText());
  }
  else if(Types.isCharacterTy(t1)){
    code = instruction::CHLOAD(temp,ctx->getText());
  }
  else if(Types.isBooleanTy(t1)){
    std::string boolVal = ctx->getText();
    if(boolVal == "true") boolVal = "1";
    else boolVal = "0";
    code = instruction::LOAD(temp,boolVal);
  }
  putAddrDecor(ctx, temp);
  putOffsetDecor(ctx, "");
  putCodeDecor(ctx, code);
  DEBUG_EXIT();
}

void CodeGenListener::enterExprIdent(AslParser::ExprIdentContext *ctx) {
  DEBUG_ENTER();
}
void CodeGenListener::exitExprIdent(AslParser::ExprIdentContext *ctx) {
  putAddrDecor(ctx, getAddrDecor(ctx->ident()));
  putOffsetDecor(ctx, getOffsetDecor(ctx->ident()));
  putCodeDecor(ctx, getCodeDecor(ctx->ident()));
  DEBUG_EXIT();
}

void CodeGenListener::enterIdent(AslParser::IdentContext *ctx) {
  DEBUG_ENTER();
}
void CodeGenListener::exitIdent(AslParser::IdentContext *ctx) {
  putAddrDecor(ctx, ctx->ID()->getText());
  putOffsetDecor(ctx, "");
  putCodeDecor(ctx, instructionList());
  DEBUG_EXIT();
}

// void CodeGenListener::enterEveryRule(antlr4::ParserRuleContext *ctx) {
//   DEBUG_ENTER();
// }
// void CodeGenListener::exitEveryRule(antlr4::ParserRuleContext *ctx) {
//   DEBUG_EXIT();
// }
// void CodeGenListener::visitTerminal(antlr4::tree::TerminalNode *node) {
//   DEBUG(">>> visit " << node->getSymbol()->getLine() << ":" << node->getSymbol()->getCharPositionInLine() << " CodeGen TerminalNode");
// }
// void CodeGenListener::visitErrorNode(antlr4::tree::ErrorNode *node) {
// }


// Getters for the necessary tree node atributes:
//   Scope, Type, Addr, Offset and Code
SymTable::ScopeId CodeGenListener::getScopeDecor(antlr4::ParserRuleContext *ctx) {
  return Decorations.getScope(ctx);
}
TypesMgr::TypeId CodeGenListener::getTypeDecor(antlr4::ParserRuleContext *ctx) {
  return Decorations.getType(ctx);
}
std::string CodeGenListener::getAddrDecor(antlr4::ParserRuleContext *ctx) {
  return Decorations.getAddr(ctx);
}
std::string  CodeGenListener::getOffsetDecor(antlr4::ParserRuleContext *ctx) {
  return Decorations.getOffset(ctx);
}
instructionList CodeGenListener::getCodeDecor(antlr4::ParserRuleContext *ctx) {
  return Decorations.getCode(ctx);
}

// Setters for the necessary tree node attributes:
//   Addr, Offset and Code
void CodeGenListener::putAddrDecor(antlr4::ParserRuleContext *ctx, const std::string & a) {
  Decorations.putAddr(ctx, a);
}
void CodeGenListener::putOffsetDecor(antlr4::ParserRuleContext *ctx, const std::string & o) {
  Decorations.putOffset(ctx, o);
}
void CodeGenListener::putCodeDecor(antlr4::ParserRuleContext *ctx, const instructionList & c) {
  Decorations.putCode(ctx, c);
}
