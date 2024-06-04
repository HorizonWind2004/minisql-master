#ifndef MINISQL_TABLE_ITERATOR_H
#define MINISQL_TABLE_ITERATOR_H

#include "common/rowid.h"
#include "concurrency/txn.h"
#include "record/row.h"

class TableHeap;

class TableIterator {
public:
 // you may define your own constructor based on your member variables
 explicit TableIterator(TableHeap *table_heap, RowId rid, Txn *txn);

 explicit TableIterator(const TableIterator &other);

  virtual ~TableIterator();

  bool operator==(const TableIterator &itr) const;

  bool operator!=(const TableIterator &itr) const;

  const Row &operator*();

  Row *operator->();

  TableIterator &operator=(const TableIterator &itr) noexcept;

  TableIterator &operator++();

  TableIterator operator++(int);

  TableHeap* get_Table_Heap()
  {
    return table_heap_;
  }

  RowId get_Rid()
  {
    return rid_;
  }

  Txn* get_Txn()
  {
    return txn_;
  }

  Row* get_Row()
  {
    return cur_row_;
  }

private:
  TableHeap *table_heap_;
  RowId rid_;
  Txn *txn_;
  Row *cur_row_;
};

#endif  // MINISQL_TABLE_ITERATOR_H
