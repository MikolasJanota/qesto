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
  // rareqs -uupb -h3 FILENAME
  char* nargv[4];
  char o1[3];
  char o2[3];
  char hs[2];
  strcpy(o1, "-g");
  strcpy(o2, "-e");
  strcpy(hs, "-");
  nargv[0]=argv[0];
  nargv[1]=o1;
  nargv[2]=o2;
  nargv[3]=argc>=2 ? argv[1] : hs;
  if (argc>2) { cerr<<"c WARNING: ingoring some options after FILENAME"<<std::endl;}
  argv=nargv;
  argc=4;
#else
  cerr<<"c WARNING: running in the EXPERT mode, I'm very stupid without any options."<<std::endl;
#endif
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
