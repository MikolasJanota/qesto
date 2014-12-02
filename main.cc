/*
 * File:  main.cc
 * Author:  mikolas
 * Created on: Thu, Sep 04, 2014
 * Copyright (C) 2014, Mikolas Janota
 */
#include <signal.h>
#include "ReadQ.hh"
#include "Options.hh"
#include "Qesto.hh"
using namespace std;
static void SIG_handler(int signum);
ostream& print_usage(const Options& options,ostream& o);
Qesto* ps=NULL;

int main(int argc, char** argv) {
  signal(SIGHUP, SIG_handler);
  signal(SIGTERM, SIG_handler);
  signal(SIGINT, SIG_handler);
  signal(SIGABRT, SIG_handler);
  signal(SIGUSR1, SIG_handler);
  Options options;
  if (!options.parse(argc, argv)) {
    cerr << "ERROR: processing options." << endl;
    print_usage(options,cerr);
    return 100;
  }
  auto& rest = options.get_rest();
  if (rest.size()>1)
    cerr<<"WARNING: garbage at the end of command line."<<endl;

  if (options.get_help()){
    print_usage(options,cout);
    return 0;
  }
  const string flafile(rest.size() ? rest[0] : "-");
  if (flafile=="-") cout<<"c reading from standard input"<<endl;
  else cout<<"c reading from "<<flafile<<endl;
  Reader* fr=NULL;
  gzFile ff=Z_NULL;
  if (flafile.size()==1 && flafile[0]=='-') {
    fr=new Reader(cin);
  } else {
    ff = gzopen(flafile.c_str(), "rb");
    if (ff == Z_NULL) {
      cerr << "Unable to open file: " << flafile << endl;
      exit(100);
    }
    fr=new Reader(ff);
  }
  ReadQ rq(*fr,false);
  try {
    rq.read();
  } catch (ReadException& rex) {
    cerr << rex.what() << endl;
    exit(100);
  }
  QFla qf;
  qf.pref=rq.get_prefix();
  qf.cnf=rq.get_clauses();
  if(qf.pref.size()==0) {
    qf.pref.push_back(make_pair(EXISTENTIAL,VarVector()));
  }
  ps=new Qesto(options,qf);
  const bool r=ps->solve();
  std::cout<<"s cnf "<<(r?'1':'0')<<std::endl;
  std::cout<<"c bt_count: "<<ps->get_btcount()<<std::endl;
  return r ? 10 : 20;
}

static void SIG_handler(int signum) {
  cerr<<"# received external signal " << signum << endl;
  if(ps){cerr<<"c bt_count: "<<ps->get_btcount()<<std::endl;}
  cerr<<"Terminating ..." << endl;
  exit(0);
}

ostream& print_usage(const Options& options,ostream& o) {
  o << "USAGE: [OPTIONS] [FILENAME]" << endl;
  return options.print(o);
}
