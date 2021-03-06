#include "pobcpp.h"
#include "piglet.h"

#include "elsa/cc_type.h"
#include "elsa/cc_flags.h"

#include <string>
#include <sstream>
#include <map>
#include <algorithm>

//#define POBCPPDEBUG


extern char* itoa(int value); // elsa/pobcpp.cc FIXME

Pobcpp::Pobcpp(Patcher &patcher):patcher(patcher) { }

Pobcpp::~Pobcpp() {
  using std::string;
  using std::map;
  using std::pair;
  using std::vector;
  string original_file = file;
  map<int, vector<PobcppPatch*> >::iterator iter;
  for(iter = patchess.begin(); iter != patchess.end(); ++iter ) {
    std::sort(iter->second.begin(), iter->second.end(), PobcppPatchCmp());
    vector<PobcppPatch*>::const_iterator viter;
    int iline = iter->first;
    std::string sline  = patcher.getLine(iline, file);
    std::string line = sline;
    int diff = 0;
    for(viter = iter->second.begin(); viter != iter->second.end(); ++viter) {
      PobcppPatch* patch = *viter;
      if(patch->kind == Insert) {
        //std::cerr << "patch: " << patch->str << std::endl;
//        if(diff + patch->col-1 <= patch->str.size()-1) {
          sline.insert(diff + patch->col-1, patch->str);
          diff += patch->str.length();
//        }
      }
      else {
        if(patch->erase == 0) {
          sline.erase(diff+ patch->col-1, sline.size() - patch->col+1);
        }
        else {
          diff -= patch->erase;
          sline.erase(diff+ patch->col-1, patch->erase);
        }
      }
    }
    sline += " //";
    patcher.insertBefore(original_file.c_str(), UnboxedLoc(iter->first, 1), sline);
  }
  // FIXME
  patchess.clear();
}

std::string Pobcpp::getLine(int line) {
  return patcher.getLine(line, file);
}

bool Pobcpp::visitTypeSpecifier(TypeSpecifier *type) {
  if (type->isTS_classSpec()) {
    return subvisitTS_classSpec(type->asTS_classSpec());
  }
  return true;
}

bool Pobcpp::subvisitTS_classSpec(TS_classSpec *spec) {
  #ifdef POBCPPDEBUG
  std::cout << "subvisitTS_classSpec() call" << std::endl;
  #endif
  using std::string;
  if(spec->keyword == TI_UNIT) { // unit?
    int enumCount = spec->enumerators->count();
//    removeUnitDecl(spec->loc);
//    removeEnumeratorDecls(spec);
    string sline;
    string::size_type found;
    // Search for a ':' or a '{' and insert ' : public Pobcpp::Unit '
    if(spec->enumerators->count()) {
      //createEnumerator(spec);
    }
    int inheritance = !(spec->bases->count());
    //    std::cout << inheritance << std::endl;
    int iline = sourceLocManager->getLine(spec->beginBracket);
    int col = sourceLocManager->getCol(spec->beginBracket);
    if(inheritance)
      appendPobunitBaseClass(inheritance, iline, col);
    else
      appendPobunitBaseClass(inheritance, iline, col);
  }
  else if(spec->keyword == TI_CLASS) {
    unsigned short units = 0;  // How many units exist inside this class?
    units = countUnits(&(spec->members->list));

    if(!units)
      return true;
    else {
      int iline = sourceLocManager->getLine(spec->endBracket);
      int col = sourceLocManager->getCol(spec->endBracket);
      appendPobTypeArrayFunc(spec, iline, col-2, units);
    }
  }
  #ifdef POBCPPDEBUG
  std::cout << "subvisitTS_classSpec() end" << std::endl;
  #endif
  return true;
}

bool Pobcpp::visitExpression(Expression* exp) {
  using std::string;
  if(!(exp->kind() == Expression::E_RANKSOF))
    return false;
  E_ranksof* e_ranksof = dynamic_cast<E_ranksof*>(exp);
  int beginParentCol = sourceLocManager->getCol(e_ranksof->beginParenthesis);
  int beginParentLine = sourceLocManager->getLine(e_ranksof->beginParenthesis);
  int endParentCol = sourceLocManager->getCol(e_ranksof->endParenthesis);
  int endParentLine = sourceLocManager->getLine(e_ranksof->endParenthesis);
  int commaCol = sourceLocManager->getCol(e_ranksof->comma);
  int commaLine = sourceLocManager->getLine(e_ranksof->comma);

  PobcppPatch* erase1 = new PobcppPatch(Erase, string(), beginParentCol+1, 1);
  (patchess[beginParentLine]).push_back(erase1);
  PobcppPatch* insert1 = new PobcppPatch(Insert, string("_<"), beginParentCol+1);
  (patchess[beginParentLine]).push_back(insert1);
  PobcppPatch* erase2 = new PobcppPatch(Erase, string(), endParentCol+1, 1);
  (patchess[endParentLine]).push_back(erase2);
  if(!e_ranksof->singleUnit || !e_ranksof->implicit) {
    PobcppPatch* erase2 = new PobcppPatch(Erase, string(), commaCol+1, 1);
    (patchess[commaLine]).push_back(erase2);
  } else {
    
  }
  
  if(e_ranksof->implicit) {	
    if(!e_ranksof->singleUnit) {
      PobcppPatch* insert2 = new PobcppPatch(Insert, string(">(comm, "), commaCol+1); //FIXME comm must be the communicator identifier of the function
      (patchess[commaLine]).push_back(insert2);
    } else {
      PobcppPatch* insert2 = new PobcppPatch(Insert, string(">(comm"), endParentCol+1); //FIXME comm must be the communicator identifier of the function
      (patchess[endParentLine]).push_back(insert2);
      
    }
    PobcppPatch* insert3 = new PobcppPatch(Insert, string(", Unit_Type(this))"), endParentCol+1);
    (patchess[endParentLine]).push_back(insert3);
  }
  else {
    PobcppPatch* insert2 = new PobcppPatch(Insert, string(">("), commaCol+1);
    (patchess[commaLine]).push_back(insert2);
    PobcppPatch* insert3 = new PobcppPatch(Insert, string(", Unit_Type(this))"), endParentCol+1);
    (patchess[endParentLine]).push_back(insert3);
  }
  return true;
}

void Pobcpp::appendPobTypeArrayFunc(TS_classSpec* spec, int iline, std::string::size_type found, unsigned int units) {
  //FIXME
  return;
	using std::string;
  //TODO FIXME
  // Append this definition:
  // Pob_Type_Array __get_types() {
  //   Pob_Type_Array pobtypes(units);
  //   pobtypes.add_type<unit A>(0);
  //   pobtypes.add_type<unit B>(1);
  //   ...
  //   return pobtypes;
  // }
  string function = "public: static Pobcpp::Pob_Type_Array __get_types() { Pobcpp::Pob_Type_Array pobtypes(" + string(itoa(units)) + "); ";

  unsigned int j = 0; // Unit order
  for(unsigned int i = 0; i < classes.size(); i++) { // FIXME Better search algorithm.
    if(classes[i].c != spec->name->asPQ_name()->name)
      continue;
    if(classes[i].enumerators == 0) {
      function += "pobtypes.add_type<" + string(classes[i].u) + ">(" + string(itoa(j)) + ");";
    } else {
      function += "pobtypes.add_type<" + string(classes[i].u) + ">(" + string(itoa(j)) + ", " + string(itoa(classes[i].enumerators)) +  ");";
    }
    j++;
  }
  function += "return pobtypes; } private:";
  PobcppPatch* insert = new PobcppPatch(Insert, function, found+2);
  (patchess[iline]).push_back(insert);
}

void Pobcpp::appendPobunitBaseClass(bool firstBaseClass, int line, std::string::size_type found) {
  if(firstBaseClass) {
    PobcppPatch* insert = new PobcppPatch(Insert, std::string(" : virtual public Pobcpp::Unit "), found);
    (patchess[line]).push_back(insert);
  }
  else {
    PobcppPatch* insert = new PobcppPatch(Insert, std::string(" ,virtual public Pobcpp::Unit "), found);
    (patchess[line]).push_back(insert);
  }
}

unsigned int Pobcpp::countUnits(ASTList<Member> *memberList) {
  unsigned int units = 0;
  FOREACH_ASTLIST_NC(Member, *memberList, iter) {
    if(iter.data()->isMR_decl()) {
      MR_decl* iter_decl = iter.data()->asMR_decl();
      if(iter_decl->d->spec->isTS_classSpec()) {
        if(iter_decl->d->spec->asTS_classSpec()->keyword == TI_UNIT) {
          units++;
        }
      }
    }
  }
  return units;
}

int main(int argc, char **argv) {
  PigletParser parser(false);
  Patcher p;
  Pobcpp visitor(p);
	for (int i = 1 ;i< argc;i+=2) {
    visitor.setFile(argv[i+1]);
    TranslationUnit *unit = parser.getASTNoExc(argv[i]);
    visitor.classes = parser.classes;
    unit->traverse(visitor);
  }
  return 0;
}

