#include "../include/record/column.h"
#include<iostream>
#include "glog/logging.h"

Column::Column(std::string column_name, TypeId type, uint32_t index, bool nullable, bool unique)
    : name_(std::move(column_name)), type_(type), table_ind_(index), nullable_(nullable), unique_(unique) {
  ASSERT(type != TypeId::kTypeChar, "Wrong constructor for CHAR type.");
  switch (type) {
    case TypeId::kTypeInt:
      len_ = sizeof(int32_t);
      break;
    case TypeId::kTypeFloat:
      len_ = sizeof(float_t);
      break;
    default:
      ASSERT(false, "Unsupported column type.");
  }
}

Column::Column(std::string column_name, TypeId type, uint32_t length, uint32_t index, bool nullable, bool unique)
    : name_(std::move(column_name)),
      type_(type),
      len_(length),
      table_ind_(index),
      nullable_(nullable),
      unique_(unique) {
  ASSERT(type == TypeId::kTypeChar, "Wrong constructor for non-VARCHAR type.");
}

Column::Column(const Column *other)
    : name_(other->name_),
      type_(other->type_),
      len_(other->len_),
      table_ind_(other->table_ind_),
      nullable_(other->nullable_),
      unique_(other->unique_) {}

/**
* TODO: Student Implement
*/
uint32_t Column::SerializeTo(char *buf) const {
  uint32_t bytes_written=0;
  //write magic number
  MACH_WRITE_UINT32(buf+bytes_written, COLUMN_MAGIC_NUM);
  bytes_written+=sizeof(uint32_t);

  //write column name
  uint32_t name_length=name_.length();
  memcpy(buf+bytes_written,&name_length,sizeof(uint32_t));
  bytes_written+=sizeof(uint32_t);
  memcpy(buf+bytes_written,name_.c_str(),name_length);
  bytes_written+=name_length;
  
  //write type
  MACH_WRITE_TO(TypeId, buf+bytes_written, type_);
  bytes_written+=sizeof(TypeId);

  //write max byte length for kTypeChar
  if(type_==kTypeChar)
  {
    MACH_WRITE_UINT32(buf+bytes_written, len_);
    bytes_written+=sizeof(uint32_t);
  }

  //write index
  MACH_WRITE_UINT32(buf+bytes_written, table_ind_);
  bytes_written+=sizeof(uint32_t);

  //write nullable
  MACH_WRITE_TO(bool, buf+bytes_written, nullable_);
  bytes_written+=sizeof(bool);

  //write unique
  MACH_WRITE_TO(bool, buf+bytes_written, unique_);
  bytes_written+=sizeof(bool);
  // std::cout<<"ser"<<' '<<bytes_written<<std::endl;
  return bytes_written;
}

/**
 * TODO: Student Implement
 */
uint32_t Column::GetSerializedSize() const {
  uint32_t written_bytes;
  written_bytes=sizeof(uint32_t) + sizeof(uint32_t)+name_.length() + sizeof(TypeId) + ((type_==kTypeChar)?sizeof(uint32_t):0) + sizeof(uint32_t) + sizeof(bool) + sizeof(bool);
  // std::cout<<"getser"<<' '<<written_bytes<<std::endl;
  return written_bytes;
}

/**
 * TODO: Student Implement
 */
uint32_t Column::DeserializeFrom(char *buf, Column *&column) {
  //  if (column != nullptr) {
  //   LOG(WARNING) << "Pointer to column is not null in column deserialize."<<std::endl;
  //   return 0;
  // }

  uint32_t bytes_read=0;

  //read magic number
  uint32_t magic_number=MACH_READ_UINT32(buf+bytes_read);
  bytes_read+=sizeof(uint32_t);
  //check magic number
  if(magic_number!=COLUMN_MAGIC_NUM)
  {
    LOG(WARNING) << "column magic number mismatch in column deserialize"<<std::endl;
    return 0;
  }

  //read column name
  uint32_t name_length=MACH_READ_UINT32(buf+bytes_read);
  bytes_read+=sizeof(uint32_t);
  std::string name(buf+bytes_read, buf+bytes_read+name_length);
  bytes_read+=name_length;

  //read type
  TypeId type=MACH_READ_FROM(TypeId, buf+bytes_read);
  bytes_read+=sizeof(TypeId);

  //read max byte length for kTypeChar
  uint32_t len=0;
  if(type==kTypeChar)
  {
    len=MACH_READ_UINT32(buf+bytes_read);
    bytes_read+=sizeof(uint32_t);
  }

  //read index
  uint32_t index=MACH_READ_UINT32(buf+bytes_read);
  bytes_read+=sizeof(uint32_t);

  //read nullable
  bool nullable=MACH_READ_FROM(bool, buf+bytes_read);
  bytes_read+=sizeof(bool);

  //read unique
  bool unique=MACH_READ_FROM(bool, buf+bytes_read);
  bytes_read+=sizeof(bool);

  if(type==kTypeChar)
  {
    column=new Column(name, type, len, index, nullable, unique);
  }
  else
  {
    column=new Column(name, type, index, nullable, unique);
  }

  return bytes_read;
}
