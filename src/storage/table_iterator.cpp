#include "storage/table_iterator.h"

#include "common/macros.h"
#include "storage/table_heap.h"

/**
 * TODO: Student Implement
 */
TableIterator::TableIterator(TableHeap *table_heap, RowId rid, Txn *txn):table_heap_(table_heap),rid_(rid),txn_(txn),cur_row_(nullptr) {
  cur_row_=new Row(rid);
  if(table_heap!=nullptr)
  {
    table_heap->GetTuple(cur_row_, txn);
  }
}

TableIterator::TableIterator(const TableIterator &other):table_heap_(other.table_heap_),rid_(other.rid_),txn_(other.txn_),cur_row_(nullptr) {
    cur_row_=new Row(rid_);
    if(other.cur_row_!=nullptr)
    {
      *cur_row_=*other.cur_row_;
    }
}

TableIterator::~TableIterator() {
  delete cur_row_;
}

bool TableIterator::operator==(const TableIterator &itr) const {
  return (table_heap_==itr.table_heap_)&&((rid_.GetPageId()==INVALID_PAGE_ID&&itr.rid_.GetPageId()==INVALID_PAGE_ID)||rid_==itr.rid_);
}

bool TableIterator::operator!=(const TableIterator &itr) const {
  return !(*this==itr);
}

const Row &TableIterator::operator*() {
  ASSERT(cur_row_!=nullptr,"Dereferencing a null iterator");
  return *cur_row_;
}

Row *TableIterator::operator->() {
  ASSERT(cur_row_!=nullptr, "Accessing a null iterator");
  return cur_row_;
}

TableIterator &TableIterator::operator=(const TableIterator &itr) noexcept {
  if(this!=&itr)
  {
    table_heap_=itr.table_heap_;
    rid_=itr.rid_;
    txn_=itr.txn_;
    cur_row_->SetRowId(rid_);
    if(itr.cur_row_!=nullptr)*cur_row_=*itr.cur_row_;
  }
  return *this;
}

// ++iter
TableIterator &TableIterator::operator++() {
  ASSERT(cur_row_!=nullptr, "Incrementing a null iterator");
  RowId nxt_rid;
  // std::cout<<222<<std::endl;
  // std::cout<<rid_.GetPageId()<<std::endl;
  if(table_heap_->GetNextTupleRid(rid_, &nxt_rid, txn_))
  {
  // std::cout<<111<<std::endl;
    rid_=nxt_rid;
    // std::cout<<nxt_rid.GetPageId()<<std::endl;
    cur_row_->SetRowId(rid_);
    if(table_heap_!=nullptr)table_heap_->GetTuple(cur_row_,txn_);

  // std::cout<<111<<std::endl<<std::endl;
  }
  else
  {
    *this=TableIterator(nullptr, RowId(INVALID_PAGE_ID, 0), nullptr);
    // rid_=INVALID_ROWID;
    // cur_row_->SetRowId(rid_);
  }
  // std::cout<<111<<std::endl;
  return *this;
}

// iter++
TableIterator TableIterator::operator++(int) {
  TableIterator tmp=TableIterator(*this);
  ++(*this);
  return TableIterator(tmp);
}
