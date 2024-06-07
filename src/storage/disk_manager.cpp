// #include "storage/disk_manager.h"

// #include <sys/stat.h>

// #include <filesystem>
// #include <stdexcept>

// #include "glog/logging.h"

// DiskManager::DiskManager(const std::string &db_file) : file_name_(db_file) {
//   std::scoped_lock<std::recursive_mutex> lock(db_io_latch_);
//   db_io_.open(db_file, std::ios::binary | std::ios::in | std::ios::out);
//   // directory or file does not exist
//   if (!db_io_.is_open()) {
//     db_io_.clear();
//     // create a new file
//     std::filesystem::path p = db_file;
//     if (p.has_parent_path()) std::filesystem::create_directories(p.parent_path());
//     db_io_.open(db_file, std::ios::binary | std::ios::trunc | std::ios::out);
//     db_io_.close();
//     // reopen with original mode
//     db_io_.open(db_file, std::ios::binary | std::ios::in | std::ios::out);
//     if (!db_io_.is_open()) {
//       throw std::exception();
//     }
//   }
//   ReadPhysicalPage(META_PAGE_ID, meta_data_);
// }

// void DiskManager::Close() {
//   std::scoped_lock<std::recursive_mutex> lock(db_io_latch_);
//   WritePhysicalPage(META_PAGE_ID, meta_data_);
//   if (!closed) {
//     db_io_.close();
//     closed = true;
//   }
// }

// void DiskManager::ReadPage(page_id_t logical_page_id, char *page_data) {
//   ASSERT(logical_page_id >= 0, "Invalid page id.");
//   ReadPhysicalPage(MapPageId(logical_page_id), page_data);
// }

// void DiskManager::WritePage(page_id_t logical_page_id, const char *page_data) {
//   ASSERT(logical_page_id >= 0, "Invalid page id.");
//   WritePhysicalPage(MapPageId(logical_page_id), page_data);
// }

// /**
//  * TODO: Student Implement
//  * Done.
//  */
// page_id_t DiskManager::AllocatePage() {
//   DiskFileMetaPage *meta_page = reinterpret_cast<DiskFileMetaPage *>(meta_data_);
//   for (uint32_t i = 0; i <= meta_page->GetExtentNums(); i++) {
//     int flag = 0;
//     if (i == meta_page->GetExtentNums()) {
//       meta_page->num_extents_++;
//       meta_page->extent_used_page_[i] = 0;
//       flag = 1;
//       char bitmap_page[PAGE_SIZE];
//       memset(bitmap_page, 0, PAGE_SIZE);
//       WritePhysicalPage(1 + i * (1 + BITMAP_SIZE), bitmap_page);
//     }
//     if (meta_page->GetExtentUsedPage(i) < BITMAP_SIZE) {
//       uint32_t page_offset;
//       int bitmap_page_id = 1 + i * (1 + BITMAP_SIZE);
//       char bitmap_page[PAGE_SIZE];
//       ReadPhysicalPage(bitmap_page_id, bitmap_page);
//       BitmapPage<PAGE_SIZE> *bitmap_page_ = reinterpret_cast<BitmapPage<PAGE_SIZE> *>(bitmap_page);
//       if (bitmap_page_->AllocatePage(page_offset)) {
//         meta_page->num_allocated_pages_++;
//         meta_page->extent_used_page_[i]++;
//         WritePhysicalPage(bitmap_page_id, bitmap_page);
//         return i * BITMAP_SIZE + page_offset;
//       }
//     }
//     if (flag) 
//       break;
//   }
//   return INVALID_PAGE_ID;
// }

// /**
//  * TODO: Student Implement
//  * Done.
//  */
// void DiskManager::DeAllocatePage(page_id_t logical_page_id) {
//   DiskFileMetaPage *meta_page = reinterpret_cast<DiskFileMetaPage *>(meta_data_);
//   uint32_t extent_id = logical_page_id / BITMAP_SIZE;
//   uint32_t page_offset = logical_page_id % BITMAP_SIZE;
//   int bitmap_page_id = 1 + extent_id * (1 + BITMAP_SIZE);
//   char bitmap_page[PAGE_SIZE];
//   ReadPhysicalPage(bitmap_page_id, bitmap_page);
//   if (reinterpret_cast<BitmapPage<PAGE_SIZE> *>(bitmap_page)->DeAllocatePage(page_offset)) {
//     meta_page->num_allocated_pages_--;
//     meta_page->extent_used_page_[extent_id]--;
//     WritePhysicalPage(bitmap_page_id, bitmap_page);
//   }
// }

// /**
//  * TODO: Student Implement
//  * Done.
//  */
// bool DiskManager::IsPageFree(page_id_t logical_page_id) {
//   int bitmap_page_id = 1 + (logical_page_id / BITMAP_SIZE) * (1 + BITMAP_SIZE);
//   char bitmap_page[PAGE_SIZE];
//   ReadPhysicalPage(bitmap_page_id, bitmap_page);
//   BitmapPage<PAGE_SIZE> *bitmap = reinterpret_cast<BitmapPage<PAGE_SIZE> *>(bitmap_page);
//   return bitmap->IsPageFree(logical_page_id % BITMAP_SIZE);
// }

// /**
//  * TODO: Student Implement
//  * Done.
//  */
// page_id_t DiskManager::MapPageId(page_id_t logical_page_id) {
//   return 2 + logical_page_id + logical_page_id / BITMAP_SIZE;
// }

// int DiskManager::GetFileSize(const std::string &file_name) {
//   struct stat stat_buf;
//   int rc = stat(file_name.c_str(), &stat_buf);
//   return rc == 0 ? stat_buf.st_size : -1;
// }

// void DiskManager::ReadPhysicalPage(page_id_t physical_page_id, char *page_data) {
//   int offset = physical_page_id * PAGE_SIZE;
//   // check if read beyond file length
//   if (offset >= GetFileSize(file_name_)) {
// #ifdef ENABLE_BPM_DEBUG
//     LOG(INFO) << "Read less than a page" << std::endl;
// #endif
//     memset(page_data, 0, PAGE_SIZE);
//   } else {
//     // set read cursor to offset
//     db_io_.seekp(offset);
//     db_io_.read(page_data, PAGE_SIZE);
//     // if file ends before reading PAGE_SIZE
//     int read_count = db_io_.gcount();
//     if (read_count < PAGE_SIZE) {
// #ifdef ENABLE_BPM_DEBUG
//       LOG(INFO) << "Read less than a page" << std::endl;
// #endif
//       memset(page_data + read_count, 0, PAGE_SIZE - read_count);
//     }
//   }
// }

// void DiskManager::WritePhysicalPage(page_id_t physical_page_id, const char *page_data) {
//   size_t offset = static_cast<size_t>(physical_page_id) * PAGE_SIZE;
//   // set write cursor to offset
//   db_io_.seekp(offset);
//   db_io_.write(page_data, PAGE_SIZE);
//   // check for I/O error
//   if (db_io_.bad()) {
//     LOG(ERROR) << "I/O error while writing";
//     return;
//   }
//   // needs to flush to keep disk file in sync
//   db_io_.flush();
// }

#include "storage/disk_manager.h"

#include <sys/stat.h>

#include <filesystem>
#include <stdexcept>
#include <cstring>
#include <iostream>

#include "glog/logging.h"
#include "page/bitmap_page.h"

DiskManager::DiskManager(const std::string &db_file) : file_name_(db_file) {
  std::scoped_lock<std::recursive_mutex> lock(db_io_latch_);
  db_io_.open(db_file, std::ios::binary | std::ios::in | std::ios::out);
  // directory or file does not exist
  if (!db_io_.is_open()) {
    db_io_.clear();
    // create a new file
    std::filesystem::path p = db_file;
    if (p.has_parent_path()) std::filesystem::create_directories(p.parent_path());
    db_io_.open(db_file, std::ios::binary | std::ios::trunc | std::ios::out);
    db_io_.close();
    // reopen with original mode
    db_io_.open(db_file, std::ios::binary | std::ios::in | std::ios::out);
    if (!db_io_.is_open()) {
      throw std::exception();
    }
  }
  ReadPhysicalPage(META_PAGE_ID, meta_data_);
}

void DiskManager::Close() {
  std::scoped_lock<std::recursive_mutex> lock(db_io_latch_);
  WritePhysicalPage(META_PAGE_ID, meta_data_);
  if (!closed) {
    db_io_.close();
    closed = true;
  }
}

void DiskManager::ReadPage(page_id_t logical_page_id, char *page_data) {
  ASSERT(logical_page_id >= 0, "Invalid page id.");
  ReadPhysicalPage(MapPageId(logical_page_id), page_data);
}

void DiskManager::WritePage(page_id_t logical_page_id, const char *page_data) {
  ASSERT(logical_page_id >= 0, "Invalid page id.");
  WritePhysicalPage(MapPageId(logical_page_id), page_data);
}

/**
 * TODO: Student Implement
 */
page_id_t DiskManager::AllocatePage() {
	DiskFileMetaPage *meta_page = reinterpret_cast<DiskFileMetaPage *>(meta_data_);

	uint32_t MAX_VALID_EXTENT_ID = (PAGE_SIZE - 8) / 4;
	uint32_t extent_id = meta_page->GetExtentNums();
	for (uint32_t i = 0; i < meta_page->GetExtentNums(); i++) {
		if (meta_page->GetExtentUsedPage(i) < BITMAP_SIZE) {
			extent_id = i;
			break;
		}
	}
	if (extent_id == MAX_VALID_EXTENT_ID) {
  		return INVALID_PAGE_ID;
	}

	char bitmap_data[PAGE_SIZE];
	ReadPhysicalPage(extent_id * (BITMAP_SIZE + 1) + 1, bitmap_data);
	BitmapPage<PAGE_SIZE> *bitmap = reinterpret_cast<BitmapPage<PAGE_SIZE> *>(bitmap_data);

	uint32_t page_id;
	if (bitmap->AllocatePage(page_id)) {
		WritePhysicalPage(extent_id * (BITMAP_SIZE + 1) + 1, bitmap_data);

		meta_page->num_allocated_pages_++;
		meta_page->extent_used_page_[extent_id]++;
		if (extent_id == meta_page->GetExtentNums()) {
			meta_page->num_extents_++;
		}

		return extent_id * BITMAP_SIZE + page_id;
	}
}

/**
 * TODO: Student Implement
 */
void DiskManager::DeAllocatePage(page_id_t logical_page_id) {
	uint32_t extent_id = logical_page_id / BITMAP_SIZE;
	uint32_t page_id = logical_page_id % BITMAP_SIZE;

	char bitmap_data[PAGE_SIZE];
	ReadPhysicalPage(extent_id * (BITMAP_SIZE + 1) + 1, bitmap_data);
	BitmapPage<PAGE_SIZE> *bitmap = reinterpret_cast<BitmapPage<PAGE_SIZE> *>(bitmap_data);
	DiskFileMetaPage *meta_page = reinterpret_cast<DiskFileMetaPage *>(meta_data_);

	if (bitmap->DeAllocatePage(page_id)) {
		WritePhysicalPage(extent_id * (BITMAP_SIZE + 1) + 1, bitmap_data);

		meta_page->num_allocated_pages_--;
		meta_page->extent_used_page_[extent_id]--;
		while (meta_page->num_extents_ > 0 && meta_page->extent_used_page_[meta_page->num_extents_ - 1] == 0) {
			meta_page->num_extents_--;
		}
	}
}

/**
 * TODO: Student Implement
 */
bool DiskManager::IsPageFree(page_id_t logical_page_id) {
	uint32_t extent_id = logical_page_id / BITMAP_SIZE;
	uint32_t page_id = logical_page_id % BITMAP_SIZE;

	char bitmap_data[PAGE_SIZE];
	ReadPhysicalPage(extent_id * (BITMAP_SIZE + 1) + 1, bitmap_data);
	BitmapPage<PAGE_SIZE> *bitmap = reinterpret_cast<BitmapPage<PAGE_SIZE> *>(bitmap_data);

	return bitmap->IsPageFree(page_id);
}

/**
 * TODO: Student Implement
 */
page_id_t DiskManager::MapPageId(page_id_t logical_page_id) {
	uint32_t extent_id = logical_page_id / BITMAP_SIZE;
	uint32_t page_id = logical_page_id % BITMAP_SIZE;
	return extent_id * (BITMAP_SIZE + 1) + page_id + 2;
}

int DiskManager::GetFileSize(const std::string &file_name) {
  struct stat stat_buf;
  int rc = stat(file_name.c_str(), &stat_buf);
  return rc == 0 ? stat_buf.st_size : -1;
}

void DiskManager::ReadPhysicalPage(page_id_t physical_page_id, char *page_data) {
  int offset = physical_page_id * PAGE_SIZE;
  // check if read beyond file length
  if (offset >= GetFileSize(file_name_)) {
#ifdef ENABLE_BPM_DEBUG
    LOG(INFO) << "Read less than a page" << std::endl;
#endif
    memset(page_data, 0, PAGE_SIZE);
  } else {
    // set read cursor to offset
    db_io_.seekp(offset);
    db_io_.read(page_data, PAGE_SIZE);
    // if file ends before reading PAGE_SIZE
    int read_count = db_io_.gcount();
    if (read_count < PAGE_SIZE) {
#ifdef ENABLE_BPM_DEBUG
      LOG(INFO) << "Read less than a page" << std::endl;
#endif
      memset(page_data + read_count, 0, PAGE_SIZE - read_count);
    }
  }
}

void DiskManager::WritePhysicalPage(page_id_t physical_page_id, const char *page_data) {
  size_t offset = static_cast<size_t>(physical_page_id) * PAGE_SIZE;
  // set write cursor to offset
  db_io_.seekp(offset);
  db_io_.write(page_data, PAGE_SIZE);
  // check for I/O error
  if (db_io_.bad()) {
    LOG(ERROR) << "I/O error while writing";
    return;
  }
  // needs to flush to keep disk file in sync
  db_io_.flush();
}