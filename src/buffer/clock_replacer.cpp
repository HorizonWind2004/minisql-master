#include "buffer/clock_replacer.h"
#include <cassert>

CLOCKReplacer::CLOCKReplacer(size_t num_pages) : capacity_(num_pages) {}

CLOCKReplacer::~CLOCKReplacer() = default;

void CLOCKReplacer::pushClock() {
  clock_pointer_++;
  if (clock_pointer_ == clock_list_.end()) {
    clock_pointer_ = clock_list_.begin();
  }
}

/**
 * TODO: Student Implements
 * Done.
 */
bool CLOCKReplacer::Victim(frame_id_t *frame_id) {
  if (clock_list_.empty()) {
    return false;
  }
  while (true) {
    if (clock_pointer_->second == false) {
      *frame_id = clock_pointer_->first;
      clock_pointer_ = clock_list_.erase(clock_pointer_);
      clock_map_.erase(*frame_id);
      if (clock_pointer_ == clock_list_.end()) {
        clock_pointer_ = clock_list_.begin();
      }
      return true;
    } else {
      clock_pointer_->second = false;
      pushClock();
    }
  }
}

/**
 * TODO: Student Implement
 * Done.
 */
void CLOCKReplacer::Pin(frame_id_t frame_id) {
  auto it = clock_map_.find(frame_id);
  if (it != clock_map_.end()) {
    if (it->second == clock_pointer_) {
      pushClock();
    }
    clock_list_.erase(it->second);
    clock_map_.erase(it);
  }
}

/**
 * TODO: Student Implement
 * Done.
 */
void CLOCKReplacer::Unpin(frame_id_t frame_id) {
  if (clock_map_.find(frame_id) == clock_map_.end()) {
    assert(clock_list_.size() < capacity_);
    list<pair<frame_id_t, bool> >::iterator it;
    if (clock_list_.empty()) {
      it = clock_pointer_ = clock_list_.insert(clock_list_.begin(), make_pair(frame_id, true));
    } else {
      it = clock_list_.insert(clock_pointer_, make_pair(frame_id, true));
    }
    clock_map_[frame_id] = it;
  } else {
    clock_map_[frame_id]->second = true;
  }
}

/**
 * TODO: Student Implement
 * Done.
 */
size_t CLOCKReplacer::Size() {
  return clock_list_.size();
}