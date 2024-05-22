#include "../include/record/row.h"

/**
 * TODO: Student Implement
 */
uint32_t Row::SerializeTo(char *buf, Schema *schema) const {
  ASSERT(schema != nullptr, "Invalid schema before serialize.");
  ASSERT(schema->GetColumnCount() == fields_.size(), "Fields size do not match schema's column size.");

  uint32_t bytes_written=0;

  //write magic number
  MACH_WRITE_UINT32(buf+bytes_written, ROW_MAGIC_NUM);
  bytes_written+=sizeof(uint32_t);

  //write schema
  // std::cout<<bytes_written<<std::endl;
  bytes_written+=schema->SerializeTo(buf+bytes_written);
  // std::cout<<bytes_written<<std::endl;

  //write rowId
  MACH_WRITE_TO(RowId, buf+bytes_written, rid_);
  bytes_written+=sizeof(RowId);

  //write field count
  uint32_t field_count=GetFieldCount();
  MACH_WRITE_UINT32(buf+bytes_written, field_count);
  bytes_written+=sizeof(uint32_t);

  //write null bitmap
  uint8_t null_bitmap[(field_count+7)/8];
  memset(null_bitmap,0,sizeof(null_bitmap));
  for(uint32_t i=0;i<field_count;++i)
  {
    if(fields_[i]->IsNull())
    {
      null_bitmap[i/8]|=(1<<(i%8));
    }
  }
  memcpy(buf+bytes_written,null_bitmap, sizeof(null_bitmap));
  bytes_written+=sizeof(null_bitmap);

  //write field
  for(uint32_t i=0;i<field_count;++i)
  {
    bytes_written+=fields_[i]->SerializeTo(buf+bytes_written);
  }
  // std::cout<<"seralize"<<' '<<bytes_written<<std::endl;
  return bytes_written;
}

uint32_t Row::DeserializeFrom(char *buf, Schema *schema) {
  ASSERT(schema != nullptr, "Invalid schema before serialize.");
  ASSERT(fields_.empty(), "Non empty field in row.");

  uint32_t bytes_read=0;

  //read magic number
  uint32_t magic_number=MACH_READ_UINT32(buf+bytes_read);
  bytes_read+=sizeof(uint32_t);
  if(magic_number!=ROW_MAGIC_NUM)
  {
    LOG(WARNING) << "row magic number mismatch in row deserialize"<<std::endl;
    return 0;
  }

  //read schema
  bytes_read+=Schema::DeserializeFrom(buf+bytes_read,schema);

  //read rowid
  rid_=MACH_READ_FROM(RowId, buf+bytes_read);
  bytes_read+=sizeof(RowId);

  //read field count
  uint32_t field_count=MACH_READ_UINT32(buf+bytes_read);
  bytes_read+=sizeof(uint32_t);

  //read null bitmap
  uint8_t null_bitmap[(field_count+7)/8];
  memcpy(null_bitmap,buf+bytes_read,sizeof(null_bitmap));
  bytes_read+=sizeof(null_bitmap);

  //read field
  fields_.clear();
  for(uint32_t i=0;i<field_count;++i)
  {
    Field *field=nullptr;
    bytes_read+=Field::DeserializeFrom(buf+bytes_read,schema->GetColumn(i)->GetType(),&field,null_bitmap[i/8]&(1<<(i%8)));
    fields_.push_back(field);
  }

  return bytes_read;
}

uint32_t Row::GetSerializedSize(Schema *schema) const {
  ASSERT(schema != nullptr, "Invalid schema before serialize.");
  ASSERT(schema->GetColumnCount() == fields_.size(), "Fields size do not match schema's column size.");
  uint32_t written_size=0;

  written_size+=sizeof(uint32_t);

  written_size+=schema->GetSerializedSize();
  // std::cout<<schema->GetSerializedSize()<<std::endl;

  written_size+=sizeof(RowId);

  uint32_t field_count=GetFieldCount();
  written_size+=sizeof(uint32_t);

  written_size+=(field_count+7)/8;

  for(uint32_t i=0;i<field_count;++i)
  {
    written_size+=fields_[i]->GetSerializedSize();
  }

  // std::cout<<"getseralize size"<<' '<<written_size<<std::endl;
  return written_size;
}

void Row::GetKeyFromRow(const Schema *schema, const Schema *key_schema, Row &key_row) {
  auto columns = key_schema->GetColumns();
  std::vector<Field> fields;
  uint32_t idx;
  for (auto column : columns) {
    schema->GetColumnIndex(column->GetName(), idx);
    fields.emplace_back(*this->GetField(idx));
  }
  key_row = Row(fields);
}
