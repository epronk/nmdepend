//#include "ExampleTestCase.h"
#include <cppunit/extensions/HelperMacros.h>

#include "boost/shared_ptr.hpp"

#include "ObjectPackage.h"
#include "ObjectFile.h"
#include "SymbolStore.h"

#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/path.hpp"

namespace fs = boost::filesystem;

#include <boost/ref.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/scoped_array.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/shared_array.hpp>

using namespace std;
using boost::scoped_ptr;
using boost::scoped_array;
using boost::shared_ptr;
using boost::weak_ptr;
using boost::shared_array;

class ObjectFileTest;

CPPUNIT_TEST_SUITE_REGISTRATION( ObjectFileTest );

#include <iostream>
using namespace std;

template<class T>
class CallBackStub : public Callback<T>
{
public:
  typedef T* Ptr;

  typedef std::pair<Ptr, Ptr> pair;
  virtual void operator()(T& from,T& to)
  {
    m_Pair = make_pair(&from, &to);
  }
  void test(T& from, T& to)
  {
    CPPUNIT_ASSERT_EQUAL(pair(&from, &to), m_Pair);
  }
  friend std::ostream& operator<<(std::ostream& out, const pair& v)
  {
    out << *(v.first) << " " << *(v.second);
  }
private:  
  pair m_Pair;
};

class ObjectFileTest : public CPPUNIT_NS::TestFixture
{
  CPPUNIT_TEST_SUITE( ObjectFileTest );
  CPPUNIT_TEST( testSymbolStore );
  CPPUNIT_TEST( linkTwoObjects );
  CPPUNIT_TEST( testRead );
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
  }

protected:
  
  void testSymbolStore()  
  {
    SymbolStore f;

    SymbolPtr p = f.Add("func1");
    SymbolPtr q = p;
    CPPUNIT_ASSERT(p == q);

    SymbolPtr r = f.Add("func1");
    CPPUNIT_ASSERT(p == r);

    SymbolPtr s = f.Add("func2");
    CPPUNIT_ASSERT(p != s);
    
  }
  
  void linkTwoObjects()  
  {
    SymbolStore store;
    CallBackStub<ObjectFile> g1;
    CallBackStub<ObjectPackage> g2;
    CallBackStub<Package> g3;
    CallBackStub<Package> g4;

    Package aaaa(g4, "aaaa");
    Package aaa(g3, "aaa");
    aaa.SetParent(aaaa);
    ObjectPackage aa(g2, "aa"); // superpackage of a
    aa.SetParent(aaa);
    ObjectFile a(g1, "a.obj", store);
    a.SetParent(aa);

    a.AddImportSymbol("b");
    a.AddImportSymbol("c");
    a.AddImportSymbol("c");
    a.AddImportSymbol("d");

    Package bbbb(g4, "bbbb");
    Package bbb(g3, "bbb");
    bbb.SetParent(bbbb);
    ObjectPackage bb(g2, "bb"); // superpackage of b
    bb.SetParent(bbb);
    ObjectFile b(g1, "b.obj", store);
    b.SetParent(bb);

    b.AddExportSymbol("a");
    b.AddExportSymbol("b");
    b.AddExportSymbol("c");

    //CPPUNIT_ASSERT(!a.Depend(b));
    //CPPUNIT_ASSERT(!aa.Depend(bb));
    //CPPUNIT_ASSERT(!aaa.Depend(bbb));

    // Link all objects
    a.Link();
    b.Link();

    g1.test(a,b);
    g2.test(aa,bb);
    g3(aaa,bbb);
    g4(aaaa,bbbb);
    
    std::set<SymbolPtr> inter;

    a.intersection(b, inter);
    CPPUNIT_ASSERT(!inter.empty());
    CPPUNIT_ASSERT(a.Depend(b));
    //CPPUNIT_ASSERT(aa.Depend(bb));
    CPPUNIT_ASSERT(aaa.Depend(bbb));
  }

  void testRead()
  {
    SymbolStore store;
    CallBackStub<ObjectFile> g1;

    ObjectFile b(g1, "b.obj", store);
    CPPUNIT_ASSERT_EQUAL(string("b.obj"), b.Name());
    fs::path fileName = fs::initial_path() / "project1/sub1/b.o";
    fileName.normalize();
    b.Read(fileName);

    ObjectFile f(g1, "f.obj", store);
    fileName = fs::initial_path() / "project1/sub2/f.o";
    fileName.normalize();
    CPPUNIT_ASSERT(!b.Depend(f));
    f.Read(fileName);
    CPPUNIT_ASSERT(b.Depend(f));
  }
};
