#include "index/index_iterator.h"

#include "index/basic_comparator.h"
#include "index/generic_key.h"

IndexIterator::IndexIterator() = default;

IndexIterator::IndexIterator(page_id_t page_id, BufferPoolManager *bpm, int index)
    : current_page_id(page_id), item_index(index), buffer_pool_manager(bpm) {
  page = reinterpret_cast<LeafPage *>(buffer_pool_manager->FetchPage(current_page_id)->GetData());
}

IndexIterator::~IndexIterator() {
  if (current_page_id != INVALID_PAGE_ID)
    buffer_pool_manager->UnpinPage(current_page_id, false);
}

std::pair<GenericKey *, RowId> IndexIterator::operator*() {
  //ASSERT(false, "Not implemented yet.");
  return page->GetItem(item_index);
}

IndexIterator &IndexIterator::operator++() {
  // Increase the current item index
  ++item_index;

  // Check if we need to move to the next page
  if (item_index >= page->GetSize()) {
    page_id_t next_page_id = page->GetNextPageId();
    
    // If there is a next page
    if (next_page_id != INVALID_PAGE_ID) {
      // Unpin the current page
      buffer_pool_manager->UnpinPage(current_page_id, false);
      
      // Move to the next page
      current_page_id = next_page_id;
      auto *next_page = buffer_pool_manager->FetchPage(next_page_id);
      
      // If the next page is not null
      if (next_page != nullptr) {
        // Cast the data to a LeafPage pointer
        auto *next_leaf_page = reinterpret_cast<LeafPage *>(next_page->GetData());
        
        // Update the page pointer and reset the item index
        page = next_leaf_page;
        item_index = 0;
      }
    } else {
      // If there is no next page, set the iterator to its default state
      buffer_pool_manager->UnpinPage(current_page_id, false);
      current_page_id = INVALID_PAGE_ID;
      page = nullptr;
      item_index = 0;
      *this = IndexIterator();
    }
  }

  return *this;
}

bool IndexIterator::operator==(const IndexIterator &itr) const {
  return current_page_id == itr.current_page_id && item_index == itr.item_index;
}

bool IndexIterator::operator!=(const IndexIterator &itr) const {
  return !(*this == itr);
}