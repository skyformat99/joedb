#include "joedb/Database.h"
#include "is_identifier.h"

/////////////////////////////////////////////////////////////////////////////
const std::map<field_id_t, std::string> &joedb::Database::get_fields
/////////////////////////////////////////////////////////////////////////////
(
 table_id_t table_id
) const
{
 auto table_it = tables.find(table_id);
 if (table_it == tables.end())
  throw std::runtime_error("get_fields: invalid table_id");
 return table_it->second.field_names;
}

/////////////////////////////////////////////////////////////////////////////
const joedb::Type &joedb::Database::get_field_type
/////////////////////////////////////////////////////////////////////////////
(
 table_id_t table_id,
 field_id_t field_id
) const
{
 static Type null_type;
 auto table_it = tables.find(table_id);
 if (table_it == tables.end())
  return null_type;
 auto &fields = table_it->second.get_fields();
 auto field_it = fields.find(field_id);
 if (field_it == fields.end())
  return null_type;
 return field_it->second.get_type();
}

/////////////////////////////////////////////////////////////////////////////
record_id_t joedb::Database::get_last_record_id(table_id_t table_id) const
/////////////////////////////////////////////////////////////////////////////
{
 auto table_it = tables.find(table_id);
 if (table_it == tables.end())
  return 0;
 return table_it->second.freedom.size();
}

/////////////////////////////////////////////////////////////////////////////
bool joedb::Database::is_used
/////////////////////////////////////////////////////////////////////////////
(
 table_id_t table_id,
 record_id_t record_id
) const
{
 auto table_it = tables.find(table_id);
 if (table_it == tables.end())
  return false;
 return table_it->second.freedom.is_used(record_id + 1);
}

#define TYPE_MACRO(type, return_type, type_id, R, W)\
return_type joedb::Database::get_##type_id(table_id_t table_id,\
                                           record_id_t record_id,\
                                           field_id_t field_id) const\
{\
 auto table_it = tables.find(table_id);\
 if (table_it == tables.end())\
  throw std::runtime_error("get: invalid table_id");\
 return table_it->second.get_##type_id(record_id, field_id);\
}
#include "TYPE_MACRO.h"
#undef TYPE_MACRO

/////////////////////////////////////////////////////////////////////////////
void joedb::Database::create_table(const std::string &name)
/////////////////////////////////////////////////////////////////////////////
{
 if (!is_identifier(name))
  throw std::runtime_error("create_table: invalid identifier");
 if (find_table(name))
  throw std::runtime_error("create_table: name already used");

 ++current_table_id;
 tables.insert(std::make_pair(current_table_id, Table()));
 table_names[current_table_id] = name;
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Database::drop_table(table_id_t table_id)
/////////////////////////////////////////////////////////////////////////////
{
 auto it = tables.find(table_id);
 if (it == tables.end())
  throw std::runtime_error("drop_table: invalid table_id");
 table_names.erase(table_id);
 tables.erase(it);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Database::rename_table
/////////////////////////////////////////////////////////////////////////////
(
 table_id_t table_id,
 const std::string &name
)
{
 if (!is_identifier(name))
  throw std::runtime_error("rename_table: invalid identifier");

 auto table_it = tables.find(table_id);
 if (table_it == tables.end())
  throw std::runtime_error("rename_table: invalid table_id");

 if (find_table(name) != 0)
  throw std::runtime_error("rename_table: name already used");

 table_names[table_id] = name;
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Database::add_field
/////////////////////////////////////////////////////////////////////////////
(
 table_id_t table_id,
 const std::string &name,
 Type type
)
{
 if (!is_identifier(name))
  throw std::runtime_error("add_field: invalid identifier");

 auto it = tables.find(table_id);
 if (it == tables.end())
  throw std::runtime_error("add_field: invalid table_id");

 it->second.add_field(name, type);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Database::drop_field(table_id_t table_id, field_id_t field_id)
/////////////////////////////////////////////////////////////////////////////
{
 auto it = tables.find(table_id);
 if (it == tables.end())
  throw std::runtime_error("drop_field: invalid table_id");

 it->second.drop_field(field_id);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Database::rename_field
/////////////////////////////////////////////////////////////////////////////
(
 table_id_t table_id,
 field_id_t field_id,
 const std::string &name
)
{
 if (!is_identifier(name))
  throw std::runtime_error("rename_field: invalid identifier");

 auto table_it = tables.find(table_id);
 if (table_it == tables.end())
  throw std::runtime_error("rename_field: invalid table_id");

 auto &field_names = table_it->second.field_names;
 auto field_it = field_names.find(field_id);
 if (field_it == field_names.end())
  throw std::runtime_error("rename_field: invalid field_id");

 if (table_it->second.find_field(name))
  throw std::runtime_error("rename_field: name already used");

 field_it->second = name;
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Database::insert_into(table_id_t table_id, record_id_t record_id)
/////////////////////////////////////////////////////////////////////////////
{
 auto it = tables.find(table_id);
 if (it == tables.end())
  throw std::runtime_error("insert_into: invalid table_id");

 if (record_id <= 0 || (max_record_id && record_id > max_record_id))
  throw std::runtime_error("insert_into: too big");

 it->second.insert_record(record_id);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Database::insert_vector
/////////////////////////////////////////////////////////////////////////////
(
 table_id_t table_id,
 record_id_t record_id,
 record_id_t size
)
{
 auto it = tables.find(table_id);
 if (it == tables.end())
  throw std::runtime_error("insert_vector: invalid table_id");
 if (record_id <= 0 ||
     size <= 0 ||
     (max_record_id && (record_id > max_record_id || size > max_record_id)))
  throw std::runtime_error("insert_vector: too big");

 for (record_id_t i = 0; i < size; i++)
  it->second.insert_record(record_id + i);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Database::delete_from
/////////////////////////////////////////////////////////////////////////////
(
 table_id_t table_id,
 record_id_t record_id
)
{
 auto it = tables.find(table_id);
 if (it == tables.end())
  throw std::runtime_error("delete_from: invalid table_id");

 it->second.delete_record(record_id);
}

/////////////////////////////////////////////////////////////////////////////
#define TYPE_MACRO(type, return_type, type_id, R, W)\
void joedb::Database::update_##type_id(table_id_t table_id,\
                                       record_id_t record_id,\
                                       field_id_t field_id,\
                                       return_type value)\
{\
 auto it = tables.find(table_id);\
 if (it == tables.end())\
  throw std::runtime_error("update: invalid table_id");\
 it->second.update_##type_id(record_id, field_id, value);\
}\
void joedb::Database::update_vector_##type_id(table_id_t table_id,\
                                              record_id_t record_id,\
                                              field_id_t field_id,\
                                              record_id_t size,\
                                              const type *value)\
{\
 auto it = tables.find(table_id);\
 if (it == tables.end())\
  throw std::runtime_error("update_vector: invalid table_id");\
 it->second.update_vector_##type_id(record_id, field_id, size, value);\
}
#include "joedb/TYPE_MACRO.h"
#undef TYPE_MACRO
