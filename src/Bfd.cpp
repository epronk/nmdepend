#include "nmdepend.h"
#include "ObjectFile.h"
#include "Factory.h"

#include <bfd.h>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

#include <stdexcept>

namespace fs = boost::filesystem;

Bfd::Bfd(Factory& factory)
 : m_Factory(factory)
{
}

const std::string Bfd::packageName(const fs::path& path, int packageLevel)
{
  fs::path::iterator p = path.end();
  --p;
  
  for(int i = 0; i < packageLevel; ++i)
      --p;

  return p->string();
}

Entity* Bfd::Read(const fs::path& path, int packageLevel)
{
  char *target = 0;
  bfd* file = bfd_openr (path.string().c_str(), target);
  assert(file);

  Entity* object = NULL;

  fs::path::iterator p = path.end();
  --p;
  std::string name = p->string();
 
  if (bfd_check_format (file, bfd_object))
  {
  
    ObjectPackage* op = m_Factory.CreatePackage(packageName(path, packageLevel));
    assert(op);
    ObjectFile* of = m_Factory.CreateObject(name);
    of->Read(file);
    of->SetParent(*op);
    object = op;
  }

  if (bfd_check_format (file, bfd_archive))
  {
    ObjectPackage* package = m_Factory.CreatePackage(name);
    object = package;
    bfd *arfile = 0;

    std::cout << "file " << bfd_get_filename (file) << std::endl;

    while((arfile = bfd_openr_next_archived_file (file, arfile)))
    {
      const char* objectname = bfd_get_filename (arfile);
      ObjectFile* o = m_Factory.CreateObject(objectname);
      o->SetParent(*package);
      o->Read(arfile);
      std::cout << "file " << bfd_get_filename (arfile) << std::endl;
    }
  }

  return object;
}

Entity* Bfd::ReadPackage(const fs::path& path, int /*packageLevel*/)
{
  char *target = 0;
  bfd* file = bfd_openr (path.string().c_str(), target);
  assert(file);

  Entity* object = NULL;

  fs::path::iterator p = path.end();
  --p;
  std::string name = p->string();
 
  if (bfd_check_format (file, bfd_object))
  {
    throw std::logic_error("unknown format");
  }

  if (bfd_check_format (file, bfd_archive))
  {
    ObjectPackage* package = m_Factory.CreatePackage(name);
    object = package;
    bfd *arfile = 0;

    std::cout << "file " << bfd_get_filename (file) << std::endl;

    ObjectFile* o = m_Factory.CreateObject(name);
    while((arfile = bfd_openr_next_archived_file (file, arfile)))
    {
      o->SetParent(*package);
      o->Read(arfile);
      std::cout << "file " << bfd_get_filename (arfile) << std::endl;
    }
  }

  return object;
}

