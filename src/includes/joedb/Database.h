#ifndef joedb_Database_declared
#define joedb_Database_declared

#include "Readable_Writeable.h"
#include "Table.h"

#include <map>

namespace joedb
{
 class Database: public Readable_Writeable
 {
  private:
   const record_id_t max_record_id;

   std::map<table_id_t, Table> tables;
   std::map<table_id_t, std::string> table_names;
   table_id_t current_table_id = 0;

  public:
   Database(record_id_t max_record_id = 0): max_record_id(max_record_id) {}

   //
   // Readable override
   //
   const std::map<table_id_t, std::string> &get_tables() const override {return table_names;}
   const std::map<field_id_t, std::string> &get_fields(table_id_t table_id) const override;
   const Type &get_field_type(table_id_t table_id,
                              field_id_t field_id) const override;

   record_id_t get_last_record_id(table_id_t table_id) const override;
   bool is_used(table_id_t table_id, record_id_t record_id) const override;
   #define TYPE_MACRO(type, return_type, type_id, R, W)\
   return_type get_##type_id(table_id_t table_id,\
                             record_id_t record_id,\
                             field_id_t field_id) const override;
   #include "TYPE_MACRO.h"
   #undef TYPE_MACRO


   //
   // Writeable override
   //
   void create_table(const std::string &name) override;
   void drop_table(table_id_t table_id) override;
   void rename_table(table_id_t table_id, const std::string &name) override;
   void add_field(table_id_t table_id,
                  const std::string &name,
                  Type type) override;
   void drop_field(table_id_t table_id, field_id_t field_id) override;
   void rename_field(table_id_t, field_id_t, const std::string &name) override;

   void custom(const std::string &name) override {};
   void comment(const std::string &comment) override {};
   void timestamp(int64_t timestamp) override {};
   void valid_data() override {};

   record_id_t get_max_record_id() const override {return max_record_id;}

   void insert_into(table_id_t table_id, record_id_t record_id) override;
   void insert_vector(table_id_t table_id,
                      record_id_t record_id,
                      record_id_t size) override;
   void delete_from(table_id_t table_id, record_id_t record_id) override;

   #define TYPE_MACRO(type, return_type, type_id, R, W)\
   void update_##type_id(table_id_t table_id,\
                         record_id_t record_id,\
                         field_id_t field_id,\
                         return_type value) override;\
   void update_vector_##type_id(table_id_t table_id,\
                                record_id_t record_id,\
                                field_id_t field_id,\
                                record_id_t size,\
                                const type *value) override;
   #include "TYPE_MACRO.h"
   #undef TYPE_MACRO
 };
}

#endif
