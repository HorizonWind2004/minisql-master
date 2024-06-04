// #include "buffer/lru_replacer.h"

// LRUReplacer::LRUReplacer(size_t num_pages) : capacity_(num_pages) {}

// LRUReplacer::~LRUReplacer() = default;

// /**
//  * TODO: Student Implements
//  * Done.
//  */
// bool LRUReplacer::Victim(frame_id_t *frame_id) {
//   if (lru_list_.empty()) {
//     return false;
//   }
//   *frame_id = lru_list_.back();
//   lru_list_.pop_back();
//   lru_map_.erase(*frame_id);
//   return true;
// }

// /**
//  * TODO: Student Implement
//  * Done.
//  */
// void LRUReplacer::Pin(frame_id_t frame_id) {
//   auto it = lru_map_.find(frame_id);
//   if (it != lru_map_.end()) {
//     lru_list_.erase(it->second);
//     lru_map_.erase(it);
//   }
// }

// /**
//  * TODO: Student Implement
//  * Done.
//  */
// void LRUReplacer::Unpin(frame_id_t frame_id) {
//   auto it = lru_map_.find(frame_id);
//   if (it == lru_map_.end()) {
//     lru_list_.push_front(frame_id);
//     lru_map_[frame_id] = lru_list_.begin();
//   }
// }

// /**
//  * TODO: Student Implement
//  * Done.
//  */
// size_t LRUReplacer::Size() {
//   return lru_list_.size();
// }
#include "buffer/lru_replacer.h"

LRUReplacer::LRUReplacer(size_t num_pages){
	page_capacity_ = num_pages;
	page_list_.clear();
	page_map_.clear();
}

LRUReplacer::~LRUReplacer() = default;

/**
 * TODO: Student Implement
 */
bool LRUReplacer::Victim(frame_id_t *frame_id) {
	if (page_list_.empty()) {
		*frame_id = INVALID_FRAME_ID;
		return false;
	}

	*frame_id = page_list_.front();
	page_list_.pop_front();
	page_map_.erase(*frame_id);
	return true;
}

/**
 * TODO: Student Implement
 */
void LRUReplacer::Pin(frame_id_t frame_id) {
	if (page_map_.count(frame_id)) {
		for (auto it = page_list_.begin(); it != page_list_.end(); ++it) {
			if (*it == frame_id) {
				page_list_.erase(it);
				break;
			}
		}
		page_map_.erase(frame_id);
	}
}

/**
 * TODO: Student Implement
 */
void LRUReplacer::Unpin(frame_id_t frame_id) {
	if ((uint32_t)page_map_.size() == page_capacity_) {
		return;
	}

	if (!page_map_.count(frame_id)) {
		page_list_.push_back(frame_id);
		page_map_[frame_id] = true;
	}
}

/**
 * TODO: Student Implement
 */
size_t LRUReplacer::Size() {
	return (size_t)page_list_.size();
}