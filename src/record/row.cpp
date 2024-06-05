// #include "../include/record/row.h"

// /**
//  * TODO: Student Implement
//  */
// uint32_t Row::SerializeTo(char *buf, Schema *schema) const {
//   ASSERT(schema != nullptr, "Invalid schema before serialize.");
//   ASSERT(schema->GetColumnCount() == fields_.size(), "Fields size do not match schema's column size.");

//   uint32_t bytes_written=0;


//   //write rowId
//   MACH_WRITE_TO(RowId, buf+bytes_written, rid_);
//   bytes_written+=sizeof(RowId);

//   uint32_t field_count=GetFieldCount();

//   //write null bitmap
//   bool null_bitmap[field_count];
//   memset(null_bitmap,0,sizeof(null_bitmap));
//   for(uint32_t i=0;i<field_count;++i)
//   {
//     if(fields_[i]->IsNull())
//     {
//       null_bitmap[i]=1;
//     }
//   }
//   memcpy(buf+bytes_written,null_bitmap, sizeof(null_bitmap));
//   bytes_written+=sizeof(null_bitmap);

//   //write field
//   for(uint32_t i=0;i<field_count;++i)
//   {
//     if(fields_[i]->IsNull())continue;
//     bytes_written+=fields_[i]->SerializeTo(buf+bytes_written);
//   }
//   return bytes_written;
// }

// uint32_t Row::DeserializeFrom(char *buf, Schema *schema) {
//   ASSERT(schema != nullptr, "Invalid schema before serialize.");
//   ASSERT(fields_.empty(), "Non empty field in row.");

//   uint32_t bytes_read=0;


//   //read rowid
//   rid_=MACH_READ_FROM(RowId, buf+bytes_read);
//   bytes_read+=sizeof(RowId);

//   uint32_t field_count=schema->GetColumnCount();

//   //read null bitmap
//   bool null_bitmap[field_count];
//   memcpy(null_bitmap,buf+bytes_read,sizeof(null_bitmap));
//   bytes_read+=sizeof(null_bitmap);

//   for(uint32_t i=0;i<field_count;++i)
//   {
//     Field *field=nullptr;
//     if(null_bitmap[i])
//     {
//       field=new Field(schema->GetColumn(i)->GetType());
//     }
//     else
//     {
//       bytes_read+=Field::DeserializeFrom(buf+bytes_read,schema->GetColumn(i)->GetType(),&field,null_bitmap[i]);
//     }
//     fields_.push_back(field);
//   }
//   return bytes_read;
// }

// uint32_t Row::GetSerializedSize(Schema *schema) const {
//   ASSERT(schema != nullptr, "Invalid schema before serialize.");
//   ASSERT(schema->GetColumnCount() == fields_.size(), "Fields size do not match schema's column size.");
//   uint32_t written_size=0;

//   written_size+=sizeof(RowId);

//   uint32_t field_count=fields_.size();

//   written_size+=field_count*sizeof(bool);

//   for(uint32_t i=0;i<field_count;++i)
//   {
//     if(fields_[i]->IsNull())continue;
//     written_size+=fields_[i]->GetSerializedSize();
//   }

//   return written_size;
// }

// void Row::GetKeyFromRow(const Schema *schema, const Schema *key_schema, Row &key_row) {
//   auto columns = key_schema->GetColumns();
//   std::vector<Field> fields;
//   uint32_t idx;
//   for (auto column : columns) {
//     schema->GetColumnIndex(column->GetName(), idx);
//     fields.emplace_back(*this->GetField(idx));
//   }
//   key_row = Row(fields);
// }
#include "record/row.h"

/**
 * TODO: Student Implement
 */
uint32_t Row::SerializeTo(char *buf, Schema *schema) const {
	ASSERT(schema != nullptr, "Invalid schema before serialize.");
	ASSERT(schema->GetColumnCount() == fields_.size(), "Fields size do not match schema's column size.");
	// replace with your code here
	uint32_t offset = 0;
	uint32_t ColumnCount = schema->GetColumnCount();

	MACH_WRITE_UINT32(buf, ColumnCount);
	offset += sizeof(uint32_t);

	int Null_Bitmap_Len_ = (ColumnCount + 7) / 8;
	std::unique_ptr<unsigned char[]> Null_Bitmap(new unsigned char[Null_Bitmap_Len_]);
	memset(Null_Bitmap.get(), 0, Null_Bitmap_Len_);
	for (int i = 0; i < ColumnCount; ++i) {
		if (fields_[i]->IsNull()) {
			Null_Bitmap[i / 8] |= (1 << (i % 8));
		}
	}
	for (int i = 0; i < Null_Bitmap_Len_; ++i) {
		MACH_WRITE_TO(unsigned char, buf + offset, Null_Bitmap[i]);
		offset += sizeof(unsigned char);
	}

	for (int i = 0; i < ColumnCount; ++i) {
		if (!fields_[i]->IsNull()) {
			offset += fields_[i]->SerializeTo(buf + offset);
		}
	}

	return offset;
}

uint32_t Row::DeserializeFrom(char *buf, Schema *schema) {
	ASSERT(schema != nullptr, "Invalid schema before serialize.");
	ASSERT(fields_.empty(), "Non empty field in row.");
	// replace with your code here
	uint32_t offset = 0;

	uint32_t ColumnCount = MACH_READ_UINT32(buf);
	offset += sizeof(uint32_t);

	int Null_Bitmap_Len_ = (ColumnCount + 7) / 8;
	std::unique_ptr<unsigned char[]> Null_Bitmap(new unsigned char[Null_Bitmap_Len_]);
	for (int i = 0; i < Null_Bitmap_Len_; ++i) {
		Null_Bitmap[i] = MACH_READ_FROM(unsigned char, buf + offset);
		offset += sizeof(unsigned char);
	}

	for (int i = 0; i < ColumnCount; ++i) {
		fields_.push_back(nullptr);
		if (!(Null_Bitmap[i / 8] & (1 << (i % 8)))) {
			offset += Field::DeserializeFrom(buf + offset, schema->GetColumn(i)->GetType(), &fields_[i], false);
		} else {
			fields_[i] = new Field(schema->GetColumn(i)->GetType());
		}
	}

	return offset;
}

uint32_t Row::GetSerializedSize(Schema *schema) const {
	ASSERT(schema != nullptr, "Invalid schema before serialize.");
	ASSERT(schema->GetColumnCount() == fields_.size(), "Fields size do not match schema's column size.");
	// replace with your code here
	uint32_t ColumnCount = schema->GetColumnCount();
	uint32_t offset = sizeof(uint32_t) + (ColumnCount + 7) / 8;

	for (int i = 0; i < ColumnCount; ++i) {
		if (!fields_[i]->IsNull()) {
			offset += fields_[i]->GetSerializedSize();
		}
	}

	return offset;
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
