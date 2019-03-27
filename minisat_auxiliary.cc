#include "minisat_auxiliary.hh"
using SATSPC::l_True;
using SATSPC::l_False;
using SATSPC::l_Undef;

char get_sign(lbool l) {
    if (l==l_True) return '+';
    else if (l==l_False) return '-';
    else if (l==l_Undef) return '?';
    assert (false);
    return -1;
}

ostream& print(ostream& out, Lit l) {
  if (l==SATSPC::lit_Undef) return out<<"lit_Undef";
  return out << (sign(l) ? "-" : "+") << var(l);
}

ostream& print(ostream& out, const vec<Lit>& lv) {for (int i=0;i<lv.size();++i) out <<  lv[i] << " "; return out;}

ostream& print(ostream& out, const vector<Lit>& lv) {for (size_t i=0;i<lv.size();++i) out << lv[i] << " "; return out;}

ostream& operator << (ostream& outs, const LiteralVector& ls){
  for (size_t i=0;i<ls.size();++i) {
    if (i) outs<<' ';
    outs<<ls[i];
  }
  return outs;
}

ostream& print_model(ostream& out,const vec<lbool>& lv) { return print_model(out, lv, 1, lv.size()-1); }

ostream& print_model(ostream& out, const vec<lbool>& lv, int l, int r) {
    for (int i=l;i<=r;++i) {
        int v=0;
        if(i>l) out<<" ";
        if (lv[i]==l_True) v=i;
        else if (lv[i]==l_False) v=-i;
        else if (lv[i]==l_Undef) v=0;
        else assert (false);
        out<<v;
    }
    return out;
}

ostream& operator << (ostream& outs, Lit lit) { return print(outs, lit); }

ostream& operator << (ostream& outs, lbool lb) {
  if (lb==l_Undef) return outs << "l_Undef";
  if (lb==l_True) return outs << "l_True";
  if (lb==l_False) return outs << "l_False";
  assert(0);
  return outs << "ERROR";
}
