#include "../include/record/row.h"


uint32_t Row::SerializeTo(char *buf, Schema *schema) const {
  ASSERT(schema != nullptr, "Invalid schema before serialize.");
  ASSERT(schema->GetColumnCount() == fields_.size(), "Fields size do not match schema's column size.");

  uint32_t bytes_written=0;

  uint32_t field_count=GetFieldCount();

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
  for(int i=0;i<(field_count+7)/8;++i)
  {
    MACH_WRITE_TO(uint8_t,buf+bytes_written,null_bitmap[i]);
    bytes_written+=sizeof(uint8_t);
  }

  //write field
  for(uint32_t i=0;i<field_count;++i)
  {
    if(fields_[i]->IsNull())continue;
    bytes_written+=fields_[i]->SerializeTo(buf+bytes_written);
  }
  return bytes_written;
}

uint32_t Row::DeserializeFrom(char *buf, Schema *schema) {
  ASSERT(schema != nullptr, "Invalid schema before serialize.");
  ASSERT(fields_.empty(), "Non empty field in row.");

  uint32_t bytes_read=0;

  uint32_t field_count=schema->GetColumnCount();

  //read null bitmap
  bool null_bitmap[(field_count+7)/8];
  for(int i=0;i<(field_count+7)/8;++i)
  {
    null_bitmap[i]=MACH_READ_FROM(uint8_t,buf+bytes_read);
    bytes_read+=sizeof(uint8_t);
  }

  for(uint32_t i=0;i<field_count;++i)
  {
    Field *field=nullptr;
    if(null_bitmap[i/8]&(1<<(i%8)))
    {
      field=new Field(schema->GetColumn(i)->GetType());
    }
    else
    {
      bytes_read+=Field::DeserializeFrom(buf+bytes_read,schema->GetColumn(i)->GetType(),&field,null_bitmap[i]);
    }
    fields_.push_back(field);
  }
  return bytes_read;
}

uint32_t Row::GetSerializedSize(Schema *schema) const {
  ASSERT(schema != nullptr, "Invalid schema before serialize.");
  ASSERT(schema->GetColumnCount() == fields_.size(), "Fields size do not match schema's column size.");
  uint32_t written_size=0;

  uint32_t field_count=fields_.size();

  written_size+=((field_count+7)/8)*sizeof(uint8_t);

  for(uint32_t i=0;i<field_count;++i)
  {
    if(fields_[i]->IsNull())continue;
    written_size+=fields_[i]->GetSerializedSize();
  }

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