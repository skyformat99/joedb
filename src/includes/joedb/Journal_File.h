#ifndef joedb_Journal_File_declared
#define joedb_Journal_File_declared

#include "Listener.h"
#include "Database.h"

namespace joedb
{
 class Generic_File;

 class Journal_File: public Listener
 {
  public:
   Journal_File(Generic_File &file);

   enum class state_t
   {
    no_error,
    bad_file,
    unsupported_version,
    bad_format,
    crash_check
   };

   state_t get_state() const {return state;}
   void checkpoint();
   void replay_log(Listener &listener);
   void rewind();
   void play_until(Listener &listener, uint64_t end);

   void after_create_table(const std::string &name) override;
   void after_drop_table(table_id_t table_id) override;
   void after_add_field(table_id_t table_id,
                        const std::string &name,
                        Type type) override;
   void after_drop_field(table_id_t table_id,
                         field_id_t field_id) override;
   void after_custom(const std::string &name) override;
   void after_insert(table_id_t table_id, record_id_t record_id) override;
   void after_delete(table_id_t table_id, record_id_t record_id) override;

   #define TYPE_MACRO(type, return_type, type_id, read_method, write_method)\
   void after_update_##type_id(table_id_t table_id,\
                               record_id_t record_id,\
                               field_id_t field_id,\
                               return_type value) override;
   #include "TYPE_MACRO.h"
   #undef TYPE_MACRO

   ~Journal_File() override;

   static const uint32_t version_number;
   static const uint64_t header_size;

  private:
   Generic_File &file;
   unsigned checkpoint_index;
   uint64_t checkpoint_position;
   state_t state;

   Database db_schema;
   table_id_t table_of_last_operation;
   record_id_t record_of_last_operation;

   Type read_type();

   void read_update(table_id_t table_id, record_id_t record_id);

   enum class operation_t: uint8_t
   {
    end_of_file   = 0x00,
    create_table  = 0x01,
    drop_table    = 0x02,
    add_field     = 0x03,
    drop_field    = 0x04,
    insert_into   = 0x05,
    delete_from   = 0x06,
    update        = 0x07,
    append        = 0x08,
    update_last   = 0x09,
    custom        = 0x10
   };
 };
}

#endif
