//////////////////////////////////////////////////////////////////////
//
//    TypeCheckListener - Walk the parser tree to do the semantic
//                        typecheck for the Asl programming language
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

#include "TypeCheckListener.h"

#include "antlr4-runtime.h"

#include "../common/TypesMgr.h"
#include "../common/SymTable.h"
#include "../common/TreeDecoration.h"
#include "../common/SemErrors.h"

#include <iostream>
#include <string>

// uncomment the following line to enable debugging messages with DEBUG*
// #define DEBUG_BUILD
#include "../common/debug.h"

using namespace std;


// Constructor
TypeCheckListener::TypeCheckListener(TypesMgr       & Types,
				     SymTable       & Symbols,
				     TreeDecoration & Decorations,
				     SemErrors      & Errors) :
  Types{Types},
  Symbols {Symbols},
  Decorations{Decorations},
  Errors{Errors} {
}

void TypeCheckListener::enterProgram(AslParser::ProgramContext *ctx) {
  DEBUG_ENTER();
  SymTable::ScopeId sc = getScopeDecor(ctx);
  Symbols.pushThisScope(sc);
}
void TypeCheckListener::exitProgram(AslParser::ProgramContext *ctx) {
  if (Symbols.noMainProperlyDeclared())
    Errors.noMainProperlyDeclared(ctx);
  Symbols.popScope();
  Errors.print();
  DEBUG_EXIT();
}

void TypeCheckListener::enterFunction(AslParser::FunctionContext *ctx) {
  DEBUG_ENTER();
  SymTable::ScopeId sc = getScopeDecor(ctx);
  Symbols.pushThisScope(sc);
  
  TypesMgr::TypeId tRet = Types.createVoidTy();
  if(ctx->return_type()){
    tRet = getTypeDecor(ctx->return_type()->type());
  }
  Symbols.setCurrentFunctionTy(tRet);
  // Symbols.print();
}
void TypeCheckListener::exitFunction(AslParser::FunctionContext *ctx) {
  Symbols.popScope();
  DEBUG_EXIT();
}



void TypeCheckListener::enterDeclarations(AslParser::DeclarationsContext *ctx) {
  DEBUG_ENTER();
}
void TypeCheckListener::exitDeclarations(AslParser::DeclarationsContext *ctx) {
  DEBUG_EXIT();
}

void TypeCheckListener::enterVariable_decl(AslParser::Variable_declContext *ctx) {
  DEBUG_ENTER();
}
void TypeCheckListener::exitVariable_decl(AslParser::Variable_declContext *ctx) {
  DEBUG_EXIT();
}

void TypeCheckListener::enterType(AslParser::TypeContext *ctx) {
  DEBUG_ENTER();
}
void TypeCheckListener::exitType(AslParser::TypeContext *ctx) {
  DEBUG_EXIT();
}

void TypeCheckListener::enterStatements(AslParser::StatementsContext *ctx) {
  DEBUG_ENTER();
}
void TypeCheckListener::exitStatements(AslParser::StatementsContext *ctx) {
  DEBUG_EXIT();
}

void TypeCheckListener::enterAssignStmt(AslParser::AssignStmtContext *ctx) {
  DEBUG_ENTER();
}

void TypeCheckListener::exitAssignStmt(AslParser::AssignStmtContext *ctx) {
  TypesMgr::TypeId t1 = getTypeDecor(ctx->left_expr());
  TypesMgr::TypeId t2 = getTypeDecor(ctx->expr());

  if ((not Types.isErrorTy(t1)) and (not Types.isErrorTy(t2)) and
      (not Types.copyableTypes(t1, t2)) ) {
    Errors.incompatibleAssignment(ctx->ASSIGN());
  }


  if ((not Types.isErrorTy(t1)) and (not getIsLValueDecor(ctx->left_expr())))
    Errors.nonReferenceableLeftExpr(ctx->left_expr());
  
	
  DEBUG_EXIT();
}

void TypeCheckListener::enterReturn(AslParser::ReturnContext *ctx){
  DEBUG_ENTER();
}
void TypeCheckListener::exitReturn(AslParser::ReturnContext *ctx){
  DEBUG_EXIT();

  if(ctx -> expr()){
    TypesMgr::TypeId t = getTypeDecor(ctx->expr());
    if(not Types.isErrorTy(t) and not Types.isPrimitiveNonVoidTy(t)){
      Errors.incompatibleReturn(ctx->RETURN());
    }
    else if(not Types.isErrorTy(t) and not Types.equalTypes(t, Symbols.getCurrentFunctionTy())){
      if(not Types.copyableTypes( Symbols.getCurrentFunctionTy(),t))
        Errors.incompatibleReturn(ctx->RETURN());
    }

  }
  else{
    if(not Types.isVoidTy(Symbols.getCurrentFunctionTy())){
      Errors.incompatibleReturn(ctx->RETURN());
    }
  }

}

void TypeCheckListener::enterIfStmt(AslParser::IfStmtContext *ctx) {
  DEBUG_ENTER();
}
void TypeCheckListener::exitIfStmt(AslParser::IfStmtContext *ctx) {
  TypesMgr::TypeId t1 = getTypeDecor(ctx->expr());
  if ((not Types.isErrorTy(t1)) and (not Types.isBooleanTy(t1)))
    Errors.booleanRequired(ctx);
  DEBUG_EXIT();
}


void TypeCheckListener::enterWhileStmt(AslParser::WhileStmtContext *ctx){
DEBUG_ENTER();
}
void TypeCheckListener::exitWhileStmt(AslParser::WhileStmtContext *ctx){
	TypesMgr::TypeId t1 = getTypeDecor(ctx->expr());
	if ((not Types.isErrorTy(t1)) and (not Types.isBooleanTy(t1)))
	Errors.booleanRequired(ctx);
	DEBUG_EXIT();
}


void TypeCheckListener::enterProcCall(AslParser::ProcCallContext *ctx) {
  DEBUG_ENTER();
}
void TypeCheckListener::exitProcCall(AslParser::ProcCallContext *ctx) {
  TypesMgr::TypeId t1 = getTypeDecor(ctx->ident());
  
  if(Types.isFunctionTy(t1)){
    TypesMgr::TypeId return_ty = Types.getFuncReturnType(t1);
    //assert(Types.isVoidTy(return_ty));
    //if(not Types.isVoidTy(return_ty)){
    //	Errors.isNotProcedure(ctx->ident());
    //}
    
    int numParams = Types.getNumOfParameters(t1);
    int comptadorParams = 0;  
    if(ctx->expr(0)){
      for(auto i:ctx->expr()) comptadorParams++;
    }
    if(comptadorParams != numParams){
    		Errors.numberOfParameters(ctx->ident());
    }
    else{
      vector<TypesMgr::TypeId> param_types = Types.getFuncParamsTypes(t1);

      bool segueix = true;
      for(int i = 0; i < Types.getNumOfParameters(t1)  and segueix; i++){
        if(not Types.equalTypes(param_types[i] , getTypeDecor(ctx->expr(i)))){
          segueix = false;
          Errors.incompatibleParameter(ctx->expr(i), i+1, ctx->ident());
        }
      }

    }
  }
  else if (not Types.isFunctionTy(t1) and not Types.isErrorTy(t1)) { //treure la primera comprovacio, pero es per recordar el que fa
    Errors.isNotCallable(ctx->ident());
  }
	
  DEBUG_EXIT();
}

void TypeCheckListener::enterReturn_func(AslParser::Return_funcContext *ctx){
  DEBUG_ENTER();
}
void TypeCheckListener::exitReturn_func(AslParser::Return_funcContext *ctx){
TypesMgr::TypeId t1 = getTypeDecor(ctx->ident());
TypesMgr::TypeId tAux = Types.createErrorTy();
  if(Types.isFunctionTy(t1)){
    TypesMgr::TypeId return_ty = Types.getFuncReturnType(t1);
    //assert(Types.isVoidTy(return_ty));
    if(Types.isVoidTy(return_ty)){
    	Errors.isNotFunction(ctx->ident());
    }
    //AQUI SHA DE COMRPOVAR ELS TYPE DEL RETURN???????????????????????????
    //if(not Types.equalTypes(return_ty))
      

    else{
      int numParams = Types.getNumOfParameters(t1);
      int comptadorParams = 0;  
      if(ctx->expr(0)){
        for(auto i:ctx->expr()) comptadorParams++;
      }
      if(comptadorParams != numParams){
          Errors.numberOfParameters(ctx->ident());
      }
      else{
        vector<TypesMgr::TypeId> param_types = Types.getFuncParamsTypes(t1);
        bool segueix = true;
        for(int i = 0; i < Types.getNumOfParameters(t1)  and segueix; i++){
          if(not Types.equalTypes(param_types[i] , getTypeDecor(ctx->expr(i)))){
            segueix = false;
            Errors.incompatibleParameter(ctx->expr(i), i+1, ctx->ident());
          }
        }
        if(segueix){
          //putTypeDecor(ctx, return_ty);
          tAux = return_ty;
        }

      }
    }
  }
  else if (not Types.isFunctionTy(t1) and not Types.isErrorTy(t1)) { //treure la primera comprovacio, pero es per recordar el que fa
    Errors.isNotCallable(ctx->ident());
  }
  bool b = getIsLValueDecor(ctx->ident());
  putIsLValueDecor(ctx, b);
  putTypeDecor(ctx, tAux);

  DEBUG_EXIT();
}

void TypeCheckListener::enterReadStmt(AslParser::ReadStmtContext *ctx) {
  DEBUG_ENTER();
}
void TypeCheckListener::exitReadStmt(AslParser::ReadStmtContext *ctx) {
  TypesMgr::TypeId t1 = getTypeDecor(ctx->left_expr());
  if ((not Types.isErrorTy(t1)) and (not Types.isPrimitiveTy(t1)) and
      (not Types.isFunctionTy(t1)))
    Errors.readWriteRequireBasic(ctx);
  if ((not Types.isErrorTy(t1)) and (not getIsLValueDecor(ctx->left_expr())))
    Errors.nonReferenceableExpression(ctx);
  DEBUG_EXIT();
}

void TypeCheckListener::enterWriteExpr(AslParser::WriteExprContext *ctx) {
  DEBUG_ENTER();
}
void TypeCheckListener::exitWriteExpr(AslParser::WriteExprContext *ctx) {
  TypesMgr::TypeId t1 = getTypeDecor(ctx->expr());
  if ((not Types.isErrorTy(t1)) and (not Types.isPrimitiveTy(t1)))
    Errors.readWriteRequireBasic(ctx);
  DEBUG_EXIT();
}

void TypeCheckListener::enterWriteString(AslParser::WriteStringContext *ctx) {
  DEBUG_ENTER();
}
void TypeCheckListener::exitWriteString(AslParser::WriteStringContext *ctx) {
  DEBUG_EXIT();
}

void TypeCheckListener::enterLeft_expr(AslParser::Left_exprContext *ctx) {
  DEBUG_ENTER();
}
void TypeCheckListener::exitLeft_expr(AslParser::Left_exprContext *ctx) {
  TypesMgr::TypeId t1 = getTypeDecor(ctx->ident());
  TypesMgr::TypeId tRes = Types.createErrorTy();
  if(ctx->expr()){
    bool estaBe = true;
    if(not Types.isErrorTy(t1) and not Types.isArrayTy(t1)){
      Errors.nonArrayInArrayAccess(ctx->ident());
      estaBe = false;
    }
    
    TypesMgr::TypeId t2 = getTypeDecor(ctx->expr());
    if(not Types.isErrorTy(t2) and not Types.isIntegerTy(t2)){
      Errors.nonIntegerIndexInArrayAccess(ctx->expr());
      estaBe = false;
    }
    
    if(estaBe){
      tRes = Types.getArrayElemType(t1);
    }
  
  }
  else {
    tRes = t1;
  }

  putTypeDecor(ctx, tRes);
  bool b = getIsLValueDecor(ctx->ident());
  putIsLValueDecor(ctx, b);
  DEBUG_EXIT();
}



void TypeCheckListener::enterArray_read(AslParser::Array_readContext *ctx){
  DEBUG_ENTER();
}
void TypeCheckListener::exitArray_read(AslParser::Array_readContext *ctx){
  TypesMgr::TypeId t1 = getTypeDecor(ctx->ident());
  TypesMgr::TypeId tRes = Types.createErrorTy();

  bool estaBe = true;

  if(not Types.isErrorTy(t1) and not Types.isArrayTy(t1)){
    Errors.nonArrayInArrayAccess(ctx->ident());
    estaBe = false;
  }
  
  TypesMgr::TypeId t2 = getTypeDecor(ctx->expr());
  if(not Types.isErrorTy(t2) and not Types.isIntegerTy(t2)){
    Errors.nonIntegerIndexInArrayAccess(ctx->expr());
    estaBe = false;
  }
  if(estaBe) tRes = Types.getArrayElemType(t1);
  putTypeDecor(ctx, tRes);
  bool b = getIsLValueDecor(ctx->ident());
  putIsLValueDecor(ctx, b);
  DEBUG_EXIT();
}






void TypeCheckListener::enterArithmetic(AslParser::ArithmeticContext *ctx) {
  DEBUG_ENTER();
}
void TypeCheckListener::exitArithmetic(AslParser::ArithmeticContext *ctx) {
  TypesMgr::TypeId t1 = getTypeDecor(ctx->expr(0));
  TypesMgr::TypeId t2 = getTypeDecor(ctx->expr(1));
  TypesMgr::TypeId t = Types.createVoidTy();
  if (((not Types.isErrorTy(t1)) and (not Types.isNumericTy(t1))) or
      ((not Types.isErrorTy(t2)) and (not Types.isNumericTy(t2)))){
    Errors.incompatibleOperator(ctx->op);
  }

  t = Types.createIntegerTy();
  putTypeDecor(ctx, t);
  putIsLValueDecor(ctx, false);
  DEBUG_EXIT();
}

void TypeCheckListener::enterRelational(AslParser::RelationalContext *ctx) {
  DEBUG_ENTER();
}
void TypeCheckListener::exitRelational(AslParser::RelationalContext *ctx) {
  TypesMgr::TypeId t1 = getTypeDecor(ctx->expr(0));
  TypesMgr::TypeId t2 = getTypeDecor(ctx->expr(1));
  TypesMgr::TypeId t = Types.createVoidTy();
  std::string oper = ctx->op->getText();
  if ((not Types.isErrorTy(t1)) and (not Types.isErrorTy(t2)) and
      (not Types.comparableTypes(t1, t2, oper))){
    Errors.incompatibleOperator(ctx->op);
  }
  
  t = Types.createBooleanTy();
  
  putTypeDecor(ctx, t);
  putIsLValueDecor(ctx, false);
  DEBUG_EXIT();
}



void TypeCheckListener::enterNotplusminus(AslParser::NotplusminusContext *ctx){
  DEBUG_ENTER();
}
void TypeCheckListener::exitNotplusminus(AslParser::NotplusminusContext *ctx){
  TypesMgr::TypeId t1 = getTypeDecor(ctx->expr());
  TypesMgr::TypeId t = Types.createVoidTy();
  if(ctx->NOT()){
    if((not Types.isErrorTy(t1)) and (not Types.isBooleanTy(t1))){
        Errors.booleanRequired(ctx->expr());
        t = Types.createErrorTy();
    }
    //else
     t = Types.createBooleanTy();
    putTypeDecor(ctx, t);
    putIsLValueDecor(ctx, false);
  }
  DEBUG_EXIT();
}


void TypeCheckListener::enterLogic(AslParser::LogicContext * ctx){
  DEBUG_ENTER();
}
void TypeCheckListener::exitLogic(AslParser::LogicContext * ctx){
  TypesMgr::TypeId t1 = getTypeDecor(ctx->expr(0));
  TypesMgr::TypeId t2 = getTypeDecor(ctx->expr(1));
  TypesMgr::TypeId t = Types.createVoidTy();
  std::string oper = ctx->op->getText();


  if ((not Types.isErrorTy(t1)) and (not Types.isErrorTy(t2)) and
      (not Types.equalTypes(t1, t2) or not Types.isBooleanTy(t1))){
        Errors.incompatibleOperator(ctx->op);
        t = Types.createErrorTy();
  }
  //else 
   t = Types.createBooleanTy();

  putTypeDecor(ctx, t);
  putIsLValueDecor(ctx, false);
  DEBUG_EXIT();
}



void TypeCheckListener::enterValue(AslParser::ValueContext *ctx) {
  DEBUG_ENTER();
}
void TypeCheckListener::exitValue(AslParser::ValueContext *ctx) { //NO SE SI HAURIA D"ANAR AL SYMBOL
    TypesMgr::TypeId t = Types.createVoidTy();
  if(ctx->INTVAL()){
    t = Types.createIntegerTy();
  }
  else if(ctx->CHARVAL()){
    t = Types.createCharacterTy();
  }
  else if(ctx->BOOLVAL()){
    t = Types.createBooleanTy();
  }
  
  else if(ctx->FLOATVAL()){
    t = Types.createFloatTy();
  }
  putTypeDecor(ctx, t);
  putIsLValueDecor(ctx, false);
  DEBUG_EXIT();
}

void TypeCheckListener::enterPar(AslParser::ParContext *ctx){
  DEBUG_ENTER();
}
void TypeCheckListener::exitPar(AslParser::ParContext *ctx){
  TypesMgr::TypeId t = getTypeDecor(ctx->expr());
  putTypeDecor(ctx, t);
  putIsLValueDecor(ctx, false);
  DEBUG_EXIT();
}

void TypeCheckListener::enterExprIdent(AslParser::ExprIdentContext *ctx) {
  DEBUG_ENTER();
}
void TypeCheckListener::exitExprIdent(AslParser::ExprIdentContext *ctx) {
  TypesMgr::TypeId t1 = getTypeDecor(ctx->ident());
  putTypeDecor(ctx, t1);
  bool b = getIsLValueDecor(ctx->ident());
  putIsLValueDecor(ctx, b);
  DEBUG_EXIT();
}





void TypeCheckListener::enterIdent(AslParser::IdentContext *ctx) {
  DEBUG_ENTER();
}
void TypeCheckListener::exitIdent(AslParser::IdentContext *ctx) {
  std::string ident = ctx->getText();
  if (Symbols.findInStack(ident) == -1) {
    Errors.undeclaredIdent(ctx->ID());
    TypesMgr::TypeId te = Types.createErrorTy();
    putTypeDecor(ctx, te);
    putIsLValueDecor(ctx, true);
  }
  else {
    TypesMgr::TypeId t1 = Symbols.getType(ident);
    putTypeDecor(ctx, t1);
    if (Symbols.isFunctionClass(ident))
      putIsLValueDecor(ctx, false);
    else
      putIsLValueDecor(ctx, true);
  }
  DEBUG_EXIT();
}



// void TypeCheckListener::enterEveryRule(antlr4::ParserRuleContext *ctx) {
//   DEBUG_ENTER();
// }
// void TypeCheckListener::exitEveryRule(antlr4::ParserRuleContext *ctx) {
//   DEBUG_EXIT();
// }
// void TypeCheckListener::visitTerminal(antlr4::tree::TerminalNode *node) {
//   DEBUG("visitTerminal");
// }
// void TypeCheckListener::visitErrorNode(antlr4::tree::ErrorNode *node) {
// }


// Getters for the necessary tree node atributes:
//   Scope, Type ans IsLValue
SymTable::ScopeId TypeCheckListener::getScopeDecor(antlr4::ParserRuleContext *ctx) {
  return Decorations.getScope(ctx);
}
TypesMgr::TypeId TypeCheckListener::getTypeDecor(antlr4::ParserRuleContext *ctx) {
  return Decorations.getType(ctx);
}
bool TypeCheckListener::getIsLValueDecor(antlr4::ParserRuleContext *ctx) {
  return Decorations.getIsLValue(ctx);
}

// Setters for the necessary tree node attributes:
//   Scope, Type ans IsLValue
void TypeCheckListener::putScopeDecor(antlr4::ParserRuleContext *ctx, SymTable::ScopeId s) {
  Decorations.putScope(ctx, s);
}
void TypeCheckListener::putTypeDecor(antlr4::ParserRuleContext *ctx, TypesMgr::TypeId t) {
  Decorations.putType(ctx, t);
}
void TypeCheckListener::putIsLValueDecor(antlr4::ParserRuleContext *ctx, bool b) {
  Decorations.putIsLValue(ctx, b);
}
