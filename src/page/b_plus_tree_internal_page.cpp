#include "page/b_plus_tree_internal_page.h"

#include "index/generic_key.h"

#define pairs_off (data_) // key-value pairs offset，data_起始地址
#define pair_size (GetKeySize() + sizeof(page_id_t))
#define key_off 0
#define val_off GetKeySize()

/**
 * TODO: Student Implement
 */
/*****************************************************************************
 * HELPER METHODS AND UTILITIES
 *****************************************************************************/
/*
 * Init method after creating a new internal page
 * Including set page type, set current size, set page id, set parent id and set
 * max page size
 */
void InternalPage::Init(page_id_t page_id, page_id_t parent_id, int key_size, int max_size) {
  SetPageType(IndexPageType::INTERNAL_PAGE);
  SetSize(0);
  SetPageId(page_id);
  SetParentPageId(parent_id);
  SetMaxSize(max_size);
  SetKeySize(key_size);
  //SetLSN(INVALID_LSN);

}
/*
 * Helper method to get/set the key associated with input "index"(a.k.a
 * array offset)
 */
GenericKey *InternalPage::KeyAt(int index) {
  return reinterpret_cast<GenericKey *>(pairs_off + index * pair_size + key_off);
}

void InternalPage::SetKeyAt(int index, GenericKey *key) { //设置与给定索引关联的键。
  memcpy(pairs_off + index * pair_size + key_off, key, GetKeySize());
}

page_id_t InternalPage::ValueAt(int index) const {
  return *reinterpret_cast<const page_id_t *>(pairs_off + index * pair_size + val_off);
}

void InternalPage::SetValueAt(int index, page_id_t value) {
  *reinterpret_cast<page_id_t *>(pairs_off + index * pair_size + val_off) = value;
}

int InternalPage::ValueIndex(const page_id_t &value) const {
  for (int i = 0; i < GetSize(); ++i) {
    if (ValueAt(i) == value)
      return i;
  }
  return -1;
}

void *InternalPage::PairPtrAt(int index) { //返回与给定索引关联的键值对的键。
  return KeyAt(index);
}

void InternalPage::PairCopy(void *dest, void *src, int pair_num) {
  memcpy(dest, src, pair_num * (GetKeySize() + sizeof(page_id_t)));
}
/*****************************************************************************
 * LOOKUP
 *****************************************************************************/
/*
 * Find and return the child pointer(page_id) which points to the child page
 * that contains input "key"
 * Start the search from the second key(the first key should always be invalid)
 * 用了二分查找
 */
page_id_t InternalPage::Lookup(const GenericKey *key, const KeyManager &KM) {
  //return INVALID_PAGE_ID;
  int l = 1, r = GetSize() - 1, index = 0;
  int cmp;
  while (l <= r) {
    int mid = (l + r) / 2;
    cmp = KM.CompareKeys(key, KeyAt(mid));
    if (cmp == 0) {
      index = mid;
      break;
    } else if (cmp < 0) { //key < KeyAt(mid) 
      r = mid - 1;
    } else {  //key > KeyAt(mid)
      index = mid; //记录最后一个键不大于搜索键的位置
      l = mid + 1;
    }
  }
  return ValueAt(index);
}

/*****************************************************************************
 * INSERTION
 *****************************************************************************/
/*
 * Populate new root page with old_value + new_key & new_value
 * When the insertion cause overflow from leaf page all the way upto the root
 * page, you should create a new root page and populate its elements.
 * NOTE: This method is only called within InsertIntoParent()(b_plus_tree.cpp)
 */
void InternalPage::PopulateNewRoot(const page_id_t &old_value, GenericKey *new_key, const page_id_t &new_value) {
  
  SetValueAt(0, old_value);
  SetKeyAt(1, new_key);
  SetValueAt(1, new_value);
  
  SetSize(2);
}

/*
 * Insert new_key & new_value pair right after the pair with its value ==
 * old_value
 * @return:  new size after insertion
 */
int InternalPage::InsertNodeAfter(const page_id_t &old_value, GenericKey *new_key, const page_id_t &new_value) {
  int index;
  index = ValueIndex(old_value) + 1;

  PairCopy(PairPtrAt(index + 1), PairPtrAt(index), GetSize() - index);
  SetKeyAt(index, new_key);
  SetValueAt(index, new_value);
  IncreaseSize(1);
  return GetSize();
}

//TODO
/*****************************************************************************
 * SPLIT 
 *****************************************************************************/
/*
 * Remove half of key & value pairs from this page to "recipient" page
 * buffer_pool_manager 是干嘛的？传给CopyNFrom()用于Fetch数据页
 */
void InternalPage::MoveHalfTo(InternalPage *recipient, BufferPoolManager *buffer_pool_manager) {
 //左叶子结点包含前m/2个记录
  //int half = (GetSize() + 1) / 2; //不过是把size/2的key给到parent
  int half = GetSize() / 2;
  int first_to_move = GetSize() - half;
  recipient->CopyNFrom(PairPtrAt(first_to_move), half, buffer_pool_manager);

  // for (int i = first_to_move; i < GetSize(); i++) {
  //   recipient->SetKeyAt(i - first_to_move, KeyAt(i)); //key[half]
  //   recipient->SetValueAt(i - first_to_move, ValueAt(i));
  // }
  // recipient->SetSize(half); //key[0]存放了要传上去的key
  
  // //update parent page id of the child of the moved pages
  // /*
  // 每当需要对B+树的数据页进行读写时，首先需要从BufferPoolManager中获取（Fetch）这个页
  // */
  // for(int index = first_to_move; index < GetSize(); index++){
  //   auto *page = buffer_pool_manager->FetchPage(ValueAt(index));
  //   if(page != nullptr)
  //   {
  //     auto *child = reinterpret_cast<BPlusTreePage *>(page->GetData());
  //     child->SetParentPageId(recipient->GetPageId());

  //     assert(child->GetParentPageId() == recipient->GetPageId());
  //     buffer_pool_manager->UnpinPage(child->GetPageId(), true);
  //   }
    
  // }
  IncreaseSize(-half);

}

/* Copy entries into me, starting from {items} and copy {size} entries.
 * Since it is an internal page, for all entries (pages) moved, their parents page now changes to me.
 * So I need to 'adopt' them by changing their parent page id, which needs to be persisted with BufferPoolManger
 *
 */
//用于处理进入到recipient的key-value对
void InternalPage::CopyNFrom(void *src, int size, BufferPoolManager *buffer_pool_manager) {
  PairCopy(PairPtrAt(GetSize()), src, size); 
  int new_pairs_offset = GetSize();
  IncreaseSize(size);
  //update parent page id of the child of the moved pages
  for (int index = new_pairs_offset; index < GetSize(); index++) {
    auto *page = buffer_pool_manager->FetchPage(ValueAt(index));
    if (page != nullptr) {
      auto *child = reinterpret_cast<BPlusTreePage *>(page->GetData());
      child->SetParentPageId(GetPageId());
      buffer_pool_manager->UnpinPage(child->GetPageId(), true);
    }
  }
}

/*****************************************************************************
 * REMOVE
 *****************************************************************************/
/*
 * Remove the key & value pair in internal page according to input index(a.k.a
 * array offset)
 * NOTE: store key&value pair continuously after deletion
 */
void InternalPage::Remove(int index) {
  PairCopy(PairPtrAt(index), PairPtrAt(index + 1), GetSize() - index - 1);
  IncreaseSize(-1);
}

/*
 * Remove the only key & value pair in internal page and return the value
 * NOTE: only call this method within AdjustRoot()(in b_plus_tree.cpp)
 */
//当根节点只有一个子节点时，我们需要将根节点设置为这个唯一的子节点。
page_id_t InternalPage::RemoveAndReturnOnlyChild() {
  IncreaseSize(-1);
  assert(GetSize() == 1);
  return ValueAt(0);
}

/*****************************************************************************
 * MERGE
 *****************************************************************************/
/*
 * Remove all of key & value pairs from this page to "recipient" page.
 * The middle_key is the separation key you should get from the parent. You need
 * to make sure the middle key is added to the recipient to maintain the invariant.
 * You also need to use BufferPoolManager to persist changes to the parent page id for those
 * pages that are moved to the recipient
 */
void InternalPage::MoveAllTo(InternalPage *recipient, GenericKey *middle_key, BufferPoolManager *buffer_pool_manager) {
  // auto page = buffer_pool_manager->FetchPage(recipient->GetParentPageId());
  // if (page != nullptr) {
  //   auto *parent = reinterpret_cast<InternalPage *>(page->GetData());
  //   SetKeyAt(0, parent->KeyAt(index));
  //   buffer_pool_manager->UnpinPage(parent->GetPageId(), false);
  // }
  // //copy the key-value pairs to recipient from the index 0
  // int recipient_size = recipient->GetSize();
  // for (int i = 0; i < GetSize(); i++) {
  //   recipient->SetKeyAt(recipient_size + i, KeyAt(i));
  //   recipient->SetValueAt(recipient_size + i, ValueAt(i));
  // }
  // recipient->IncreaseSize(GetSize());
  
  // //update parent page id of the child of the moved pages
  // for (int index = 0; index < GetSize(); index++)
  // {
  //   auto *page = buffer_pool_manager->FetchPage(ValueAt(index));
  //   if (page != nullptr)
  //   {
  //     auto *child = reinterpret_cast<BPlusTreeInternalPage *>(page->GetData());
  //     child->SetParentPageId(recipient->GetPageId());
  //     buffer_pool_manager->UnpinPage(child->GetPageId(), true);
  //   }
  // }
  SetKeyAt(0, middle_key);
  recipient->CopyNFrom(pairs_off, GetSize(), buffer_pool_manager);
  SetSize(0);


}

/*****************************************************************************
 * REDISTRIBUTE
 *****************************************************************************/
/*
 * Remove the first key & value pair from this page to tail of "recipient" page.
 *
 * The middle_key is the separation key you should get from the parent. You need
 * to make sure the middle key is added to the recipient to maintain the invariant.
 * You also need to use BufferPoolManager to persist changes to the parent page id for those
 * pages that are moved to the recipient
 */
void InternalPage::MoveFirstToEndOf(InternalPage *recipient, GenericKey *middle_key, BufferPoolManager *buffer_pool_manager) {

  SetKeyAt(0, middle_key);
  GenericKey* key = KeyAt(0); //middle_key
  page_id_t value = ValueAt(0);
  recipient->CopyLastFrom(key, value, buffer_pool_manager);
  Remove(0);
}

/*
      [17, 30]
     /    |    \
[5, 10, 15] [20] [35, 40, 45]
            rep  this
*/


/* Append an entry at the end.  
 * Since it is an internal page, the moved entry(page)'s parent needs to be updated.
 * So I need to 'adopt' it by changing its parent page id, which needs to be persisted with BufferPoolManger
 */
void InternalPage::CopyLastFrom(GenericKey *key, const page_id_t value, BufferPoolManager *buffer_pool_manager) {
  // auto *page = buffer_pool_manager->FetchPage(GetParentPageId());  //获取父节点 [17,30]
  // if (page != nullptr) {
  //   auto *parent = reinterpret_cast<BPlusTreeInternalPage *>(page->GetData()); //翻译
  //   int current_page_id = GetPageId(); //获取当前页的id
  //   int index = parent->ValueIndex(current_page_id); //获取当前页在父节点中的索引
  //   GenericKey *middle_key = parent->KeyAt(index + 1); //获取中间键,30
  //   SetKeyAt(GetSize(), middle_key); //设置当前页的最后一个键为中间键(父节点下降)
  //   SetValueAt(GetSize(), value); //value是调用函数传进来
  //   IncreaseSize(1);
  //   parent->SetKeyAt(index + 1, key); //设置父节点的中间键为key


  //   buffer_pool_manager->UnpinPage(parent->GetPageId(), true);
  // }

  // auto *page1 = buffer_pool_manager->FetchPage(value); //获取value对应的页
  // if (page1 != nullptr) {
  //   auto *child = reinterpret_cast<BPlusTreeInternalPage *>(page1->GetData());
  //   child->SetParentPageId(GetPageId());  //这里的父亲应该是recipient
  //   buffer_pool_manager->UnpinPage(child->GetPageId(), true);
  // }
  SetKeyAt(GetSize(), key);
  SetValueAt(GetSize(), value);
  auto child_page = reinterpret_cast<BPlusTreePage *>(buffer_pool_manager->FetchPage(value)->GetData());
  child_page->SetParentPageId(GetPageId());
  buffer_pool_manager->UnpinPage(child_page->GetPageId(), true);
  IncreaseSize(1);
}

/*
 * Remove the last key & value pair from this page to head of "recipient" page.
 * You need to handle the original dummy key properly, e.g. updating recipient’s array to position the middle_key at the
 * right place.
 * You also need to use BufferPoolManager to persist changes to the parent page id for those pages that are
 * moved to the recipient
 */
void InternalPage::MoveLastToFrontOf(InternalPage *recipient, GenericKey *middle_key, BufferPoolManager *buffer_pool_manager) {
  recipient->SetKeyAt(0, middle_key);
  recipient->CopyFirstFrom(ValueAt(GetSize() - 1), buffer_pool_manager);
  recipient->SetKeyAt(0, KeyAt(GetSize() - 1)); //原来的末尾的key应该给到父亲
  IncreaseSize(-1);
}

/* Append an entry at the beginning.
 * Since it is an internal page, the moved entry(page)'s parent needs to be updated.
 * So I need to 'adopt' it by changing its parent page id, which needs to be persisted with BufferPoolManger
 */
void InternalPage::CopyFirstFrom(const page_id_t value, BufferPoolManager *buffer_pool_manager) {
  //给recipient留出开头的空间,并且把原来的key[0]也就是middle_key给recipient
  PairCopy(PairPtrAt(1), PairPtrAt(0), GetSize());
  IncreaseSize(1);
  SetValueAt(0, value);
  //update parent page id of the child of the moved pages value is the child page id
  auto child_page = reinterpret_cast<BPlusTreePage *>(buffer_pool_manager->FetchPage(value)->GetData());
  child_page->SetParentPageId(GetPageId());
  buffer_pool_manager->UnpinPage(child_page->GetPageId(), true);
  
}

