// Copyright (c) 2004 E.J. Pronk and R.E.W. van Beusekom
//
// This file is part of Nmdepend.
// 
// Nmdepend is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// Nmdepend is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with Nmdepend; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#include <iostream>
#include <stdexcept>
#include <cassert>
#include "ObjectFile.h"
#include "Symbol.h"
#include "bfd.h"

using namespace std;

ObjectFile::ObjectFile(DependencyAddedEvent& callback,
					   SymbolAdded& symbol,
					   const std::string& name, SymbolStore& store)
 : Entity(name)
 , on_dependency_added(callback)
 , on_symbol_added(symbol)
 , m_SymbolStore(store)
{
}

void ObjectFile::AddImportSymbol(const std::string& name)
{
  m_SymImports.insert(m_SymbolStore.Add(name));
}

void ObjectFile::AddExportSymbol(const std::string& name)
{
  SymbolPtr p = m_SymbolStore.Add(name);
  Symbol& sym = *p; 
  sym.setOwner(this);
  m_SymExports.insert(p);
}

void ObjectFile::intersection(const Entity& entity, SymIndex_t& i) const
{
  i.clear();
  const ObjectFile& rsh = dynamic_cast<const ObjectFile&>(entity);
  set_intersection (m_SymImports.begin(), m_SymImports.end(),
              rsh.m_SymExports.begin(), rsh.m_SymExports.end(),
              std::inserter(i, i.begin()));
}

void ObjectFile::Link()
{
	for(SymIndex_t::iterator pos = m_SymImports.begin()
			; pos != m_SymImports.end(); ++pos)
	{
		const SymbolPtr r = *pos;
    
		ObjectFile* owner = r->Owner();
		if(owner)
		{
			on_symbol_added((*pos)->Name());
			if (owner != this)
			{
				on_dependency_added(*this, *owner);
				//std::cout << from.Name() << to.Name() << std::endl;
				assert(Parent());
				Parent()->Link(*owner->Parent());
			}
		}
	}
}

	void ObjectFile::Read(const boost::filesystem::path& objectfile)
	{
		char *target = 0;
		cout << objectfile.string() << endl;
		bfd* file = bfd_openr (objectfile.string().c_str(), target);
		if(!file)
			throw std::logic_error("Can't open " + objectfile.string());
		Read(file);
	}

	void ObjectFile::Read(bfd* file)
	{
		assert(bfd_check_format (file, bfd_object));

		// This is the filename with full path inside the object file.
		// Might be useful for analysing libraries.
		//std::cout << "file " << bfd_get_filename (file) << std::endl;


		// bfd_fffalse not in bdf.h on Cygwin and Gentoo.
		//bfd_boolean dynamic = static_cast<bfd_boolean>(0); // was: bfd_fffalse;
		bfd_boolean dynamic = 0;
		unsigned int size = 0;
		void *minisyms = NULL;

		long symcount = bfd_read_minisymbols (file, dynamic, &minisyms, &size);

		asymbol* store = bfd_make_empty_symbol (file);
		assert(store);

		bfd_byte* from = static_cast<bfd_byte*>(minisyms);
		bfd_byte* fromend = from + symcount * size;
		for (; from < fromend; from += size)
		{
			asymbol* sym = bfd_minisymbol_to_symbol (file, dynamic, from, store);
			assert(sym);

			const char* symname = bfd_asymbol_name (sym);
    
			if (sym->flags == 0)
			{
				asection* section = bfd_get_section (sym);

				if (bfd_is_und_section (section))
				{
				  AddImportSymbol(symname);
				}
				else
				{
				    AddExportSymbol(symname);
				} 
			}

			if (sym->flags & BSF_GLOBAL)
			{
			    asection* section = bfd_get_section (sym);


				if (bfd_is_und_section (section))
				{
					AddImportSymbol(symname);
				}
				else
				{
					AddExportSymbol(symname);
				} 
			}

			if(sym->flags & BSF_FUNCTION)
			{
				AddExportSymbol(symname);
			}

			if(sym->flags & BSF_OBJECT)
			{
				AddExportSymbol(symname);
			}
		}
	}

	bool ObjectFile::Depend(const Entity& o) const
	{
		SymIndex_t intersect;
		intersection(o, intersect);

		return !intersect.empty();
	}
