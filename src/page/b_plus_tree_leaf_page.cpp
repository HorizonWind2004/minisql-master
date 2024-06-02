#include "page/b_plus_tree_leaf_page.h"

#include <algorithm>

#include "index/generic_key.h"

#define pairs_off (data_)
#define pair_size (GetKeySize() + sizeof(RowId))
#define key_off 0
#define val_off GetKeySize()
/*****************************************************************************
 * HELPER METHODS AND UTILITIES
 *****************************************************************************/

/**
 * TODO: Student Implement
 */
/**
 * Init method after creating a new leaf page
 * Including set page type, set current size to zero, set page id/parent id, set
 * next page id and set max size
 * 未初始化next_page_id
 */
void LeafPage::Init(page_id_t page_id, page_id_t parent_id, int key_size, int max_size) 
{
  SetPageType(IndexPageType::LEAF_PAGE);
  SetSize(0);
  SetKeySize(key_size);
  SetMaxSize(max_size);
//  LOG(INFO) << "LeafPage::Init(): " << "key_size = " << key_size << ", max_size = " << max_size << std::endl;
  SetPageId(page_id);
  SetParentPageId(parent_id);
  SetNextPageId(INVALID_PAGE_ID);

}

/**
 * Helper methods to set/get next page id
 */
page_id_t LeafPage::GetNextPageId() const {
  return next_page_id_;
}

void LeafPage::SetNextPageId(page_id_t next_page_id) {
  next_page_id_ = next_page_id;
  if (next_page_id == 0) {
    LOG(INFO) << "Fatal error";
  }
}

/**
 * TODO: Student Implement
 */
/**
 * Helper method to find the first index i so that pairs_[i].first >= key
 * NOTE: This method is only used when generating index iterator
 * 二分查找
 */
int LeafPage::KeyIndex(const GenericKey *key, const KeyManager &KM) {
  if(GetSize() == 0) {
    return 0;
  }
  int l = 0, r = GetSize() - 1, index = GetSize();
  // binary search
  while(l <= r) {
    int mid = (l + r) / 2;
    int cmp = KM.CompareKeys(key, KeyAt(mid));
    if(cmp == 0) {
      index = mid;
      break;
    }  else if(cmp < 0) { // key < KeyAt(mid)
      index = mid;
      r = mid - 1;
    } else {
      l = mid + 1;
    }
  }
  return index;
}

/*
 * Helper method to find and return the key associated with input "index"(a.k.a
 * array offset)
 */
GenericKey *LeafPage::KeyAt(int index) {
  return reinterpret_cast<GenericKey *>(pairs_off + index * pair_size + key_off);
}

void LeafPage::SetKeyAt(int index, GenericKey *key) {
  memcpy(pairs_off + index * pair_size + key_off, key, GetKeySize());
}

RowId LeafPage::ValueAt(int index) const {
  return *reinterpret_cast<const RowId *>(pairs_off + index * pair_size + val_off);
}

void LeafPage::SetValueAt(int index, RowId value) {
  *reinterpret_cast<RowId *>(pairs_off + index * pair_size + val_off) = value;
}

void *LeafPage::PairPtrAt(int index) {
  return KeyAt(index);
}

void LeafPage::PairCopy(void *dest, void *src, int pair_num) {
  memcpy(dest, src, pair_num * (GetKeySize() + sizeof(RowId)));
}
/*
 * Helper method to find and return the key & value pair associated with input
 * "index"(a.k.a. array offset)
 */
std::pair<GenericKey *, RowId> LeafPage::GetItem(int index) { return make_pair(KeyAt(index), ValueAt(index)); }

/*****************************************************************************
 * INSERTION
 *****************************************************************************/
/*
 * Insert key & value pair into leaf page ordered by key
 * @return page size after insertion
 */
int LeafPage::Insert(GenericKey *key, const RowId &value, const KeyManager &KM) {
  int index = KeyIndex(key, KM);
  //LOG(WARNING) << "KeyIndex() called in Insert() : " << "index = " << index << ", GetSize() = " << GetSize() << std::endl;
  // move all elements after index to the right
  memmove(pairs_off + (index + 1) * pair_size, pairs_off + index * pair_size, (GetSize() - index) * pair_size);
  // insert key and value
  SetKeyAt(index, key);
  SetValueAt(index, value);
  IncreaseSize(1);
  return GetSize();
}

/*****************************************************************************
 * SPLIT
 *****************************************************************************/
/*
 * Remove half of key & value pairs from this page to "recipient" page
 */
void LeafPage::MoveHalfTo(LeafPage *recipient) {
  int half = GetSize() / 2;
  int first_to_move = GetSize() - half;
  
  // for (int i = first_to_move; i < GetSize(); i++) {
  //   recipient->SetKeyAt(i - first_to_move, KeyAt(i));
  //   recipient->SetValueAt(i - first_to_move, ValueAt(i));
  // }
  // recipient->SetSize(half);

  recipient->CopyNFrom(PairPtrAt(first_to_move), half);
  IncreaseSize(-half);
}

/*
 * Copy starting from items, and copy {size} number of elements into me.
 */
void LeafPage::CopyNFrom(void *src, int size) {
  PairCopy(PairPtrAt(GetSize()), src, size);
  IncreaseSize(size);
}

/*****************************************************************************
 * LOOKUP
 *****************************************************************************/
/*
 * For the given key, check to see whether it exists in the leaf page. If it
 * does, then store its corresponding value in input "value" and return true.
 * If the key does not exist, then return false
 */
bool LeafPage::Lookup(const GenericKey *key, RowId &value, const KeyManager &KM) {
  int index = KeyIndex(key, KM);
  //LOG(INFO) << "KeyIndex() called in Lookup() : " << "index = " << index << ", GetSize() = " << GetSize() << std::endl;
  if (index < GetSize() && KM.CompareKeys(KeyAt(index), key) == 0) {
    value = ValueAt(index);
    return true;
  }
  return false;
}

/*****************************************************************************
 * REMOVE
 *****************************************************************************/
/*
 * First look through leaf page to see whether delete key exist or not. If
 * existed, perform deletion, otherwise return immediately.
 * NOTE: store key&value pair continuously after deletion
 * @return  page size after deletion
 */
int LeafPage::RemoveAndDeleteRecord(const GenericKey *key, const KeyManager &KM) {
  int index = KeyIndex(key, KM);
  if (index < GetSize() && KM.CompareKeys(KeyAt(index), key) == 0) {
    // IncreaseSize(-1);
    // memmove(pairs_off + index * pair_size, pairs_off + (index + 1) * pair_size, (GetSize() - index) * pair_size);
    PairCopy(PairPtrAt(index), PairPtrAt(index + 1), GetSize() - index - 1);
    IncreaseSize(-1);
    return GetSize();
  }
  LOG(ERROR) << "Key not found" << std::endl;
//ASSERT(false, "Key not found");
  return GetSize();
}

/*****************************************************************************
 * MERGE
 *****************************************************************************/
/*
 * Remove all key & value pairs from this page to "recipient" page. Don't forget
 * to update the next_page id in the sibling page
 */
void LeafPage::MoveAllTo(LeafPage *recipient) {
  // int recipient_size = recipient->GetSize();
  // for (int i = 0; i < GetSize(); i++) {
  //   recipient->SetKeyAt(recipient_size + i, KeyAt(i));
  //   recipient->SetValueAt(recipient_size + i, ValueAt(i));
  // }
  // recipient->IncreaseSize(GetSize());
  // recipient->SetNextPageId(GetNextPageId());
  recipient->CopyNFrom(pairs_off, GetSize());
  SetSize(0);

}

/*****************************************************************************
 * REDISTRIBUTE
 *****************************************************************************/
/*
 * Remove the first key & value pair from this page to end of "recipient" page.
 *
 */
void LeafPage::MoveFirstToEndOf(LeafPage *recipient) {
  std::pair<GenericKey *, RowId> item = GetItem(0);
  recipient->CopyLastFrom(item.first, item.second);
  PairCopy(PairPtrAt(0), PairPtrAt(1), GetSize() - 1);
  IncreaseSize(-1);
}

/*
 * Copy the item into the end of my item list. (Append item to my array)
 */
void LeafPage::CopyLastFrom(GenericKey *key, const RowId value) {
  SetKeyAt(GetSize(), key);
  SetValueAt(GetSize(), value);
  IncreaseSize(1);
}

/*
 * Remove the last key & value pair from this page to "recipient" page.
 */
void LeafPage::MoveLastToFrontOf(LeafPage *recipient) {
  std::pair<GenericKey *, RowId> item = GetItem(GetSize() - 1);
  IncreaseSize(-1);
  recipient->CopyFirstFrom(item.first, item.second);
}

/*
 * Insert item at the front of my items. Move items accordingly.
 *
 */
void LeafPage::CopyFirstFrom(GenericKey *key, const RowId value) {
  memmove(pairs_off + 1 * pair_size, pairs_off, GetSize() * pair_size);
  IncreaseSize(1);
  SetKeyAt(0, key);
  SetValueAt(0, value);
}
