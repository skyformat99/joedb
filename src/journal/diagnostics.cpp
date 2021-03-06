#include "diagnostics.h"
#include "Generic_File.h"
#include "Journal_File.h"
#include "type_io.h"

#include <iostream>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 void dump_header(std::ostream &out, Generic_File &file)
 ////////////////////////////////////////////////////////////////////////////
 {
  file.set_position(0);

  out << "About this file\n";
  out << "---------------\n";

  {
   out << "joedb: ";
   std::string joedb;
   for (int i = 5; --i >= 0;)
    joedb.push_back(char(file.read<uint8_t>()));
   write_string(out, joedb);
   out << '\n';
  }

  {
   const uint32_t version = file.read<uint32_t>();
   out << "version: " << version << '\n';
  }

  {
   uint64_t pos[4];
   for (int i = 0; i < 4; i++)
    pos[i] = file.read<uint64_t>();

   for (int i = 0; i < 4; i++)
    out << "checkpoint[" << i << "] = " << pos[i] << '\n';
  }

  out << "file size: " << file.get_size() << '\n';
 }

 ////////////////////////////////////////////////////////////////////////////
 void about_joedb(std::ostream &out)
 ////////////////////////////////////////////////////////////////////////////
 {
  out << "About this version of joedb\n";
  out << "---------------------------\n";
  out << "Journal_File::compatible_version: ";
  out << Journal_File::compatible_version << '\n';
  out << "Journal_File::version_number: ";
  out << Journal_File::version_number << '\n';
  out << "version: " <<
  #include "../../VERSION"
  ;
  out << "\ncompiled: " << __DATE__ << ' ' << __TIME__ << '\n';
  out << "web site: https://www.remi-coulom.fr/joedb/\n";
 }
}
