#include "buffer/lru_replacer.h"

LRUReplacer::LRUReplacer(size_t num_pages) : capacity_(num_pages) {}

LRUReplacer::~LRUReplacer() = default;

/**
 * TODO: Student Implements
 * Done.
 */
bool LRUReplacer::Victim(frame_id_t *frame_id) {
  if (lru_list_.empty()) {
    return false;
  }
  *frame_id = lru_list_.back();
  lru_list_.pop_back();
  lru_map_.erase(*frame_id);
  return true;
}

/**
 * TODO: Student Implement
 * Done.
 */
void LRUReplacer::Pin(frame_id_t frame_id) {
  auto it = lru_map_.find(frame_id);
  if (it != lru_map_.end()) {
    lru_list_.erase(it->second);
    lru_map_.erase(it);
  }
}

/**
 * TODO: Student Implement
 * Done.
 */
void LRUReplacer::Unpin(frame_id_t frame_id) {
  auto it = lru_map_.find(frame_id);
  if (it == lru_map_.end()) {
    lru_list_.push_front(frame_id);
    lru_map_[frame_id] = lru_list_.begin();
  }
}

/**
 * TODO: Student Implement
 * Done.
 */
size_t LRUReplacer::Size() {
  return lru_list_.size();
}