// #include "../include/record/schema.h"

// /**
//  * TODO: Student Implement
//  */
// uint32_t Schema::SerializeTo(char *buf) const {
//   uint32_t bytes_written=0;

//   //write magic number
//   MACH_WRITE_UINT32(buf+bytes_written, SCHEMA_MAGIC_NUM);
//   bytes_written+=sizeof(uint32_t);

//   //write column count
//   uint32_t column_count=GetColumnCount();
//   MACH_WRITE_UINT32(buf+bytes_written, column_count);
//   bytes_written+=sizeof(uint32_t);

//   //write columns
//   for(const auto column:columns_)
//   {

//   // std::cout<<"ser"<<bytes_written<<std::endl;
//     bytes_written+=column->SerializeTo(buf+bytes_written);
//   }
  
//   //write is_manage_
//   MACH_WRITE_TO(bool, buf+bytes_written, is_manage_);
//   bytes_written+=sizeof(bool);
//   // std::cout<<"ser"<<bytes_written<<std::endl;
//   return bytes_written;
// }

// uint32_t Schema::GetSerializedSize() const {
//   uint32_t written_size=0;
  
//   written_size+=sizeof(uint32_t);
  
//   written_size+=sizeof(uint32_t);

//   for(const auto column:columns_)
//   {

//   // std::cout<<"getser"<<written_size<<std::endl;
//     written_size+=column->GetSerializedSize();
//   }

//   written_size+=sizeof(bool);

//   // std::cout<<"getser"<<written_size<<std::endl;
//   return written_size;
// }

// uint32_t Schema::DeserializeFrom(char *buf, Schema *&schema) {
//   // if (schema != nullptr) {
//   //   LOG(WARNING) << "Pointer to schema is not null in schema deserialize."<<std::endl;
//   //   return 0;
//   // }

//   uint32_t bytes_read=0;

//   //read magic number
//   uint32_t magic_number=MACH_READ_UINT32(buf+bytes_read);
//   bytes_read+=sizeof(uint32_t);
//   if(magic_number!=SCHEMA_MAGIC_NUM)
//   {
//     LOG(WARNING) << "schema magic number mismatch in schema deserialize"<<std::endl;
//     return 0;
//   }

//   //read column count
//   uint32_t column_count=MACH_READ_UINT32(buf+bytes_read);
//   bytes_read+=sizeof(uint32_t);

//   //read columns
//   std::vector<Column *>columns;
//   columns.reserve(column_count);
//   for(uint32_t i=0;i<column_count;++i)
//   {
//     Column *column=nullptr;
//     uint32_t bytes_column_read=Column::DeserializeFrom(buf+bytes_read,column);
//     if(!bytes_column_read)//Failed to deserialize column
//     {
//       for(auto col:columns)
//       {
//         delete col;
//       }
//       LOG(WARNING)<<"column deserialize failed in schema deserialize"<<std::endl;
//       return 0;
//     }
//     columns.push_back(column);
//     bytes_read+=bytes_column_read;
//   }

//   //read is_manage_
//   bool is_manage=MACH_READ_FROM(bool, buf+bytes_read);
//   bytes_read+=sizeof(bool);

//   schema=new Schema(columns, is_manage);
//   return bytes_read;
// }
#include "record/schema.h"

/**
 * TODO: Student Implement
 */
uint32_t Schema::SerializeTo(char *buf) const {
	// replace with your code here
	uint32_t offset = 0;

	MACH_WRITE_UINT32(buf, SCHEMA_MAGIC_NUM);
	offset += sizeof(uint32_t);

	MACH_WRITE_TO(bool, buf + offset, is_manage_);
	offset += sizeof(bool);

	uint32_t ColumnCount = GetColumnCount();
	MACH_WRITE_UINT32(buf + offset, ColumnCount);
	offset += sizeof(uint32_t);

	for (int i = 0; i < ColumnCount; ++i) {
		offset += columns_[i]->SerializeTo(buf + offset);
	}

	return offset;
}

uint32_t Schema::GetSerializedSize() const {
	// replace with your code here
	uint32_t offset = 2 * sizeof(uint32_t) + sizeof(bool);

	uint32_t ColumnCount = GetColumnCount();
	for (int i = 0; i < ColumnCount; ++i) {
		offset += columns_[i]->GetSerializedSize();
	}

	return offset;
}

uint32_t Schema::DeserializeFrom(char *buf, Schema *&schema) {
	// replace with your code here
	uint32_t offset = 0;

	uint32_t magic_num_ = MACH_READ_UINT32(buf);
	ASSERT(magic_num_ == SCHEMA_MAGIC_NUM, "Schema magic number mismatch");
	offset += sizeof(uint32_t);

	bool is_manage_ = MACH_READ_FROM(bool, buf + offset);
	offset += sizeof(bool);

	uint32_t ColumnCount = MACH_READ_UINT32(buf + offset);
	offset += sizeof(uint32_t);

	std::vector<Column *> columns_;
	for (int i = 0; i < ColumnCount; ++i) {
		Column *column;
		offset += Column::DeserializeFrom(buf + offset, column);
		columns_.push_back(column);
	}

	schema = new Schema(columns_, is_manage_);

	return offset;
}