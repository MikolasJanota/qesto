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
#include "QestoGroups.hh"
using namespace std;
static void SIG_handler(int signum);
ostream& print_usage(const Options& options,ostream& o);
Qesto* ps=NULL;
QestoGroups* gps=NULL;
char *_strdup(const char *s);

int main(int argc, char** argv) {
  signal(SIGHUP, SIG_handler);
  signal(SIGTERM, SIG_handler);
  signal(SIGINT, SIG_handler);
  signal(SIGABRT, SIG_handler);
  signal(SIGUSR1, SIG_handler);
#ifndef NDEBUG
  cout << "c DEBUG version." << endl;
#endif
  cerr<<"c qesto, v1.0"<<std::endl;
  cerr<<"c (C) 2015 Mikolas Janota, mikolas.janota@gmail.com"<<std::endl;
#ifndef EXPERT
  // prepare nonexpert options
  int nargc = 0;
  char* nargv[20];
  nargv[nargc++] = _strdup(argv[0]);
  nargv[nargc++] = _strdup("-g");
  nargv[nargc++] = _strdup("-e");
  nargv[nargc++] = _strdup("-y");
  nargv[nargc++] = argc>=2 ? _strdup(argv[1]) : _strdup("-");
  if (argc>2) {
      cerr<<"c WARNING: ingoring some options after FILENAME"<<std::endl;
      return 100;
  }
  argv=nargv;
  argc=nargc;
#else
  cerr<<"c WARNING: running in the EXPERT mode, I'm very stupid without any options."<<std::endl;
#endif
  Options options;
  if (!options.parse(argc, argv)) {
    cerr << "ERROR: processing options." << endl;
    print_usage(options,cerr);
    return 100;
  }
  while (nargc--) delete[] nargv[nargc];
  const auto& rest = options.get_rest();
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
  std::cout<<"c done reading: "<<read_cpu_time()<<std::endl;
  QFla qf;
  qf.pref=rq.get_prefix();
  qf.cnf=rq.get_clauses();
  if(qf.pref.size()==0) {
    qf.pref.push_back(make_pair(EXISTENTIAL,VarVector()));
  }
  bool r;
  LevelInfo levs(qf.pref);
  Groups* grs=NULL;
  if(options.get_groups()) {
    grs=new Groups(options,levs,qf);
    gps=new QestoGroups(options,levs,*grs);
    r=gps->solve();
    std::cout<<"c bt_count: "<<gps->get_btcount()<<std::endl;
  } else {
    ps=new Qesto(options,qf);
    r=ps->solve();
    std::cout<<"c bt_count: "<<ps->get_btcount()<<std::endl;
  }
  std::cout<<"s cnf "<<(r?'1':'0')<<std::endl;
  return r ? 10 : 20;
}

static void SIG_handler(int signum) {
  cerr<<"# received external signal " << signum << endl;
  if(ps){cerr<<"c bt_count: "<<ps->get_btcount()<<std::endl;}
  if(gps){cerr<<"c bt_count: "<<gps->get_btcount()<<std::endl;}
  cerr<<"Terminating ..." << endl;
  exit(0);
}

ostream& print_usage(const Options& options,ostream& o) {
  o << "USAGE: [OPTIONS] [FILENAME]" << endl;
  return options.print(o);
}

char *_strdup(const char *s) {
    size_t sz = strlen(s) + 1;
    char *p = new char[sz];
    while (sz--) p[sz] = s[sz];
    return p;
}
