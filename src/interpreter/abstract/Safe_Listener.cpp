#include "Safe_Listener.h"

/////////////////////////////////////////////////////////////////////////////
bool joedb::Safe_Listener::is_existing_table_id(table_id_t table_id) const
/////////////////////////////////////////////////////////////////////////////
{
 const auto &tables = db.get_tables();
 return tables.find(table_id) != tables.end();
}

/////////////////////////////////////////////////////////////////////////////
bool joedb::Safe_Listener::is_update_ok
/////////////////////////////////////////////////////////////////////////////
(
 table_id_t table_id,
 record_id_t record_id,
 field_id_t field_id,
 record_id_t size,
 Type::type_id_t type_id
) const
{
 const auto &tables = db.get_tables();
 auto table_it = tables.find(table_id);

 if (table_it != tables.end())
 {
  const auto &fields = table_it->second.get_fields();
  auto field_it = fields.find(field_id);
  if (field_it != fields.end() &&
      field_it->second.get_type().get_type_id() == type_id)
  {
   const Freedom_Keeper<> &freedom = table_it->second.get_freedom();

   for (record_id_t i = 0; i < size; i++)
    if (!freedom.is_used(record_id + i + 1))
     return false;

   return true;
  }
 }

 return false;
}

#define FORWARD(x) do {db_listener.x; listener.x;} while(false)

/////////////////////////////////////////////////////////////////////////////
void joedb::Safe_Listener::after_create_table(const std::string &name)
/////////////////////////////////////////////////////////////////////////////
{
 if (db.find_table(name))
  throw std::runtime_error("after_create_table: name already used");

 FORWARD(after_create_table(name));
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Safe_Listener::after_drop_table(table_id_t table_id)
/////////////////////////////////////////////////////////////////////////////
{
 if (!is_existing_table_id(table_id))
  throw std::runtime_error("after_drop_table: invalid table_id");

 FORWARD(after_drop_table(table_id));
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Safe_Listener::after_rename_table
/////////////////////////////////////////////////////////////////////////////
(
 table_id_t table_id,
 const std::string &name
)
{
 if (!is_existing_table_id(table_id))
  throw std::runtime_error("after_rename_table: invalid table_id");
 if (db.find_table(name))
  throw std::runtime_error("after_rename_table: name already used");

 FORWARD(after_rename_table(table_id, name));
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Safe_Listener::after_add_field
/////////////////////////////////////////////////////////////////////////////
(
 table_id_t table_id,
 const std::string &name,
 Type type
)
{
 if (!is_existing_table_id(table_id))
  throw std::runtime_error("after_add_field: invalid table_id");
 // TODO: make sure name is not already used

 FORWARD(after_add_field(table_id, name, type));
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Safe_Listener::after_drop_field
/////////////////////////////////////////////////////////////////////////////
(
 table_id_t table_id,
 field_id_t field_id
)
{
 // TODO: existing table
 // TODO: existing field
 FORWARD(after_drop_field(table_id, field_id));
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Safe_Listener::after_rename_field
/////////////////////////////////////////////////////////////////////////////
(
 table_id_t table_id,
 field_id_t field_id,
 const std::string &name
)
{
 // TODO: existing table
 // TODO: existing field
 // TODO: name not used
 FORWARD(after_rename_field(table_id, field_id, name));
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Safe_Listener::after_custom(const std::string &name)
/////////////////////////////////////////////////////////////////////////////
{
 FORWARD(after_custom(name));
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Safe_Listener::after_comment(const std::string &comment)
/////////////////////////////////////////////////////////////////////////////
{
 FORWARD(after_comment(comment));
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Safe_Listener::after_timestamp(int64_t timestamp)
/////////////////////////////////////////////////////////////////////////////
{
 FORWARD(after_timestamp(timestamp));
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Safe_Listener::after_valid_data()
/////////////////////////////////////////////////////////////////////////////
{
 FORWARD(after_valid_data());
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Safe_Listener::after_insert
/////////////////////////////////////////////////////////////////////////////
(
 table_id_t table_id,
 record_id_t record_id
)
{
 if (!is_existing_table_id(table_id))
  throw std::runtime_error("after_insert: invalid table_id");
 if (record_id <= 0 || (max_record_id && record_id > max_record_id))
  throw std::runtime_error("after_insert: bad record_id");
 FORWARD(after_insert(table_id, record_id));
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Safe_Listener::after_insert_vector
/////////////////////////////////////////////////////////////////////////////
(
 table_id_t table_id,
 record_id_t record_id,
 record_id_t size
)
{
 if (!is_existing_table_id(table_id))
  throw std::runtime_error("after_insert_vector: invalid table_id");
 if (record_id <= 0 ||
     size <= 0 ||
     (max_record_id && record_id > max_record_id) ||
     (max_record_id && size > max_record_id))
  throw std::runtime_error("after_insert_vector: bad record_id or size");
 FORWARD(after_insert_vector(table_id, record_id, size));
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Safe_Listener::after_delete
/////////////////////////////////////////////////////////////////////////////
(
 table_id_t table_id,
 record_id_t record_id
)
{
 // TODO: existing table_id
 // TODO: existing record_id
 FORWARD(after_delete(table_id, record_id));
}

#define TYPE_MACRO(type, return_type, type_id, R, W)\
void joedb::Safe_Listener::after_update_##type_id(table_id_t table_id,\
                                                  record_id_t record_id,\
                                                  field_id_t field_id,\
                                                  return_type value)\
{\
 if (is_update_ok(table_id,\
                  record_id,\
                  field_id,\
                  1,\
                  Type::type_id_t::type_id))\
  FORWARD(after_update_##type_id(table_id, record_id, field_id, value));\
 else\
  throw std::runtime_error("Wrong update");\
}\
void joedb::Safe_Listener::after_update_vector_##type_id(table_id_t table_id,\
                                                  record_id_t record_id,\
                                                  field_id_t field_id,\
                                                  record_id_t size,\
                                                  const type *value)\
{\
 if (is_update_ok(table_id,\
                  record_id,\
                  field_id,\
                  size,\
                  Type::type_id_t::type_id))\
  FORWARD(after_update_vector_##type_id(table_id, record_id, field_id, size, value));\
 else\
  throw std::runtime_error("Wrong update");\
}
#include "TYPE_MACRO.h"
#undef TYPE_MACRO

#undef FORWARD