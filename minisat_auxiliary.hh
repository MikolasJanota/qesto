/*
 * File:   minisat_aux.hh
 * Author: mikolas
 *
 * Created on October 21, 2010, 2:10 PM
 */
#ifndef MINISAT_AUX_HH
#define	MINISAT_AUX_HH
#include <ostream>
#include "auxiliary.hh"
#include "MiniSatExt.hh"
using std::ostream;
using std::vector;
using SATSPC::Solver;
using SATSPC::lbool;
using SATSPC::mkLit;
using SATSPC::sign;
using SATSPC::var;
using SATSPC::vec;
using SATSPC::Lit;
using SATSPC::Var;
typedef std::vector<Lit>                      LiteralVector;

ostream& print_model(ostream& out, const vec<lbool>& lv);
ostream& print_model(ostream& out, const vec<lbool>& lv, int l, int r);
ostream& print(ostream& out, const vec<Lit>& lv);
ostream& print(ostream& out, const vector<Lit>& lv);
ostream& print(ostream& out, Lit l);
ostream& operator << (ostream& outs, Lit lit);
ostream& operator << (ostream& outs, lbool lb);
ostream& operator << (ostream& outs, const LiteralVector& ls);

inline ostream& operator << (ostream& out, const vec<Lit>& lv) { return print(out,lv); }
inline ostream& operator << (ostream& out, const Minisat::LSet& lv) { return print(out,lv.toVec()); }

inline Lit to_lit(Var v, lbool val) {return val==SATSPC::l_True ? mkLit(v) : ~mkLit(v);}

inline Lit to_lit(const vec<lbool>& bv, Var v) {
  return (v<bv.size()) && (bv[v]==SATSPC::l_True) ? mkLit(v) : ~mkLit(v);
}

inline lbool eval(Lit l,const vec<lbool>& vals) {
  const Var v=var(l);
  if(v>=vals.size()) return SATSPC::l_Undef;
  const auto vv=vals[v];
  if(vv==SATSPC::l_Undef)return SATSPC::l_Undef;
  return (vv==SATSPC::l_False)==(sign(l)) ? SATSPC::l_True : SATSPC::l_False;
}

inline void to_lits(const vec<lbool>& bv, vec<Lit>& output, int s, const int e) {
  for (int index = s; index <= e; ++index) {
    if (bv[index]==SATSPC::l_True) output.push(mkLit((Var)index));
    else if (bv[index]==SATSPC::l_False) output.push(~mkLit((Var)index));
  }
}

// representative -> AND l_i
inline void
encode_and_pos(SATSPC::MiniSatExt& solver,Lit representative, const vector<Lit>& rhs) {
  for(size_t i=0;i<rhs.size();++i)
    solver.addClause(~representative,rhs[i]);
}

// AND l_i -> representative
inline void
encode_and_neg(SATSPC::MiniSatExt& solver,Lit representative, const vector<Lit>& rhs) {
  vec<Lit> ls(rhs.size()+1);
  for(size_t i=0;i<rhs.size();++i)
    ls[i]=~rhs[i];
  ls[rhs.size()]=representative;
  solver.addClause_(ls);
}

inline void
encode_and(SATSPC::MiniSatExt& solver,Lit representative, const vector<Lit>& rhs) {
  vec<Lit> ls(rhs.size()+1);
  for(size_t i=0;i<rhs.size();++i) {
    ls[i]=~rhs[i];
    solver.addClause(~representative,rhs[i]);
  }
  ls[rhs.size()]=representative;
  solver.addClause_(ls);
}

class Lit_equal {
public:
  inline bool operator () (const Lit& l1,const Lit& l2) const { return l1==l2; }
};

class Lit_hash {
public:
  inline size_t operator () (const Lit& l) const { return SATSPC::toInt(l); }
};


inline size_t literal_index(Lit l) {
  assert(var(l) > 0);
  const size_t v = (size_t) var(l);
  return sign(l) ? v<<1 : ((v<<1)+1);
}

inline Lit index2literal(size_t l) {
  const bool positive = (l&1);
  const Var  variable = l>>1;
  return positive ? mkLit(variable) : ~mkLit(variable);
}
#endif	/* MINISAT_AUX_HH */
