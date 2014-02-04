#ifndef BFD_H
#define BFD_H_

#include <boost/filesystem/path.hpp>
//#include <boost/filesystem.hpp>
#include "Entity.h"
#include "Symbol.h"
#include "SymbolStore.h"
#include "Graph.h"
#include "Callback.h"
#include "ObjectPackage.h"
#include "Factory.h"

class Bfd
{
public:
//  Bfd(DependencyAddedEvent&, const std::string& name, SymbolStore& store);i
  Bfd(Factory&);
  Entity* Read(const boost::filesystem::path& objectfile, int packageLevel);
  Entity* ReadPackage(const boost::filesystem::path& objectfile, int packageLevel);
private:
  static const std::string packageName(const boost::filesystem::path& path, int packageLevel);
  Factory& m_Factory;
};

#endif
