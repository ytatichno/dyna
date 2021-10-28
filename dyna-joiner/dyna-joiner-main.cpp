#include <cerrno>
#include <cstring>
#include <exception>
#include <iostream>
#include <fstream>
#include <sstream>
#include <system_error>
#include "../Context.h"
using namespace std;
using namespace dyna;

void test()
{
  string json_str = R"j({"name":"RawInfo","Vars":[{"File":"t.cpp","Line":1,"Column":0,"Name":"^z"},{"File":"t.cpp","Line":1,"Column":0,"Name":"z"},{"File":"t.cpp","Line":3,"Column":0,"Name":"j"},{"File":"t.cpp","Line":9,"Column":0,"Name":"a"},{"File":"t.cpp","Line":11,"Column":0,"Name":"i"}],"Loops":[{"File":"t.cpp","Line":3,"Column":0,"Private":[],"Flow":{"0":{"Min":1,"Max":1},"2":{"Min":1,"Max":2}},"Anti":{"0":{"Min":0,"Max":1},"2":{"Min":0,"Max":1}},"UseAfterLoop":[0],"ReadOccurred":[0,1,2],"WriteOccurred":[0,2],"Output":[0,2]},{"File":"t.cpp","Line":11,"Column":0,"Private":[],"Flow":{"3":{"Min":1,"Max":9},"4":{"Min":1,"Max":10}},"Anti":{"3":{"Min":1,"Max":9},"4":{"Min":0,"Max":9}},"UseAfterLoop":[],"ReadOccurred":[3,4],"WriteOccurred":[3,4],"Output":[3,4]}]})j";
  //loop duplicate
  //string json_str = R"j({"name":"RawInfo","Vars":[{"File":"t.cpp","Line":1,"Column":0,"Name":"^z"},{"File":"t.cpp","Line":1,"Column":0,"Name":"z"},{"File":"t.cpp","Line":3,"Column":0,"Name":"j"},{"File":"t.cpp","Line":9,"Column":0,"Name":"a"},{"File":"t.cpp","Line":11,"Column":0,"Name":"i"}],"Loops":[{"File":"t.cpp","Line":3,"Column":0,"Private":[],"Flow":{"0":{"Min":1,"Max":1},"2":{"Min":1,"Max":2}},"Anti":{"0":{"Min":0,"Max":1},"2":{"Min":0,"Max":1}},"UseAfterLoop":[0],"ReadOccurred":[0,1,2],"WriteOccurred":[0,2],"Output":[0,2]},{"File":"t.cpp","Line":3,"Column":0,"Private":[],"Flow":{"3":{"Min":1,"Max":9},"4":{"Min":1,"Max":10}},"Anti":{"3":{"Min":1,"Max":9},"4":{"Min":0,"Max":9}},"UseAfterLoop":[],"ReadOccurred":[3,4],"WriteOccurred":[3,4],"Output":[3,4]}]})j";
  string json_str2 = R"j({"name":"RawInfo","Vars":[{"File":"t.cpp","Line":1,"Column":0,"Name":"^z"},{"File":"t.cpp","Line":1,"Column":0,"Name":"z"},{"File":"t.cpp","Line":3,"Column":0,"Name":"j"},{"File":"t.cpp","Line":9,"Column":0,"Name":"a"},{"File":"t.cpp","Line":11,"Column":0,"Name":"i"}],"Loops":[{"File":"t.cpp","Line":3,"Column":0,"Private":[0],"Flow":{"0":{"Min":1,"Max":10},"2":{"Min":1,"Max":2}},"Anti":{"0":{"Min":0,"Max":1},"2":{"Min":0,"Max":1}},"UseAfterLoop":[0,1],"ReadOccurred":[0,1,2],"WriteOccurred":[0,2],"Output":[0,2]}]})j";
  AnalysisStorage astorage;
  ContextStringsStore cs_store;
  astorage.add_from_json(json_str, cs_store);
  astorage.add_from_json(json_str2, cs_store);
  astorage.debug_print(cout);
}

int main(int argc, char** argv)
{
  vector<string> fnames;
  bool dbg_flag = false;
  //## parse command line arguments ##
  for (int i = 1; i < argc; ++i) {
    if (strcmp(argv[i], "-d") == 0) {
      dbg_flag = true;
    }
    else if (strcmp(argv[i], "--help") == 0) {
      cout << "usage: dyna-joiner [-d] dyna-result-files..." << endl;
      return 0;
    }
    else {
      fnames.push_back(argv[i]);
    }
  }

  try {
    AnalysisStorage astorage;
    ContextStringsStore cs_store;
    for (const auto& fname : fnames) {
      try {
        ifstream file(fname);
        if (!file.is_open())
          throw system_error(errno, generic_category(), string("cannot open file '") + fname + "'");
        stringstream ss;
        ss << file.rdbuf();
        astorage.add_from_json(ss.str(), cs_store);
      }
      catch (const exception& e) {
        throw runtime_error(fname + ": " + e.what());
      }
      catch (...) {
        throw runtime_error(string("cannot read file '") + fname + "'");
      }
    }

    if (dbg_flag)
      astorage.debug_print(cout);
    else
      cout << astorage.toJSON();
  }
  catch (const exception& e) {
    cerr << "error: " << e.what() << endl;
    return 1;
  }
  return 0;
}
