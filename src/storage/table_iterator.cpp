#include "storage/table_iterator.h"

#include "common/macros.h"
#include "storage/table_heap.h"

/**
 * TODO: Student Implement
 */
TableIterator::TableIterator(TableHeap *table_heap, RowId rid, Txn *txn):table_heap_(table_heap),rid_(rid),txn_(txn),cur_row_(nullptr) {
  if(table_heap!=nullptr)
  {
    cur_row_=new Row(rid);
    if(!table_heap->GetTuple(cur_row_, txn))
    {
      delete cur_row_;
      cur_row_=nullptr;
    }
  }
}

TableIterator::TableIterator(const TableIterator &other):table_heap_(other.table_heap_),rid_(other.rid_),txn_(other.txn_),cur_row_(nullptr) {
    if(other.cur_row_!=nullptr)
    {
      //deep copy cur_row_ pointer
      cur_row_=new Row(*(other.cur_row_));
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
    delete cur_row_;
    table_heap_=itr.table_heap_;
    rid_=itr.rid_;
    txn_=itr.txn_;
    cur_row_=nullptr;
    if(itr.cur_row_!=nullptr)
    {
      cur_row_=new Row(*(itr.cur_row_));
    }
  }
  return *this;
}

// ++iter
TableIterator &TableIterator::operator++() {
  ASSERT(cur_row_!=nullptr, "Incrementing a null iterator");
  RowId nxt_rid;
  if(table_heap_->GetNextTupleRid(rid_, &nxt_rid, txn_))
  {
    rid_=nxt_rid;
    delete cur_row_;
    cur_row_=new Row(rid_);
    if(!table_heap_->GetTuple(cur_row_, txn_))
    {
      delete cur_row_;
      cur_row_=nullptr;
    }
  }
  else
  {
    delete cur_row_;
    rid_=INVALID_ROWID;
    cur_row_=nullptr;
  }
  return *this;
}

// iter++
TableIterator TableIterator::operator++(int) {
  TableIterator tmp=TableIterator(*this);
  ++(*this);
  return TableIterator(tmp);
}
