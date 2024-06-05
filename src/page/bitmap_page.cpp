// #include "page/bitmap_page.h"

// #include "glog/logging.h"

// /**
//  * TODO: Student Implement
//  * Done.
//  */
// template <size_t PageSize>
// bool BitmapPage<PageSize>::AllocatePage(uint32_t &page_offset) {
//   if (page_allocated_ == GetMaxSupportedSize()) {
//     return false;
//   }
//   for (size_t i = next_free_page_; i < GetMaxSupportedSize(); i++) {
//     int byte_index = i / 8;
//     int bit_index = i % 8;
//     if (IsPageFreeLow(byte_index, bit_index)) {
//       page_allocated_++;
//       bytes[byte_index] |= (1 << bit_index);
//       next_free_page_ = i + 1;
//       page_offset = i;
//       return true;
//     }
//   }
//   return false;
// }

// /**
//  * TODO: Student Implement
//  * Done.
//  */
// template <size_t PageSize>
// bool BitmapPage<PageSize>::DeAllocatePage(uint32_t page_offset) {
//   int byte_index = page_offset / 8;
//   int bit_index = page_offset % 8;
//   if (!IsPageFreeLow(byte_index, bit_index)) {
//     page_allocated_--;
//     bytes[byte_index] &= ~(1 << bit_index);
//     if (next_free_page_ > page_offset) {
//       next_free_page_ = page_offset;
//     }
//     return true;
//   } else {
//     return false;
//   }
// }

// /**
//  * TODO: Student Implement
//  * Done.
//  */
// template <size_t PageSize>
// bool BitmapPage<PageSize>::IsPageFree(uint32_t page_offset) const {
//   int byte_index = page_offset / 8;
//   int bit_index = page_offset % 8;
//   if (IsPageFreeLow(byte_index, bit_index)) {
//     return true;
//   }
//   return false;
// }

// template <size_t PageSize>
// bool BitmapPage<PageSize>::IsPageFreeLow(uint32_t byte_index, uint8_t bit_index) const {
//   if ((bytes[byte_index] & (1 << bit_index)) == 0) {
//     return true;
//   }
//   return false;
// }

// template class BitmapPage<64>;

// template class BitmapPage<128>;

// template class BitmapPage<256>;

// template class BitmapPage<512>;

// template class BitmapPage<1024>;

// template class BitmapPage<2048>;

// template class BitmapPage<4096>;

#include "page/bitmap_page.h"

#include "glog/logging.h"

/**
 * TODO: Student Implement
 */
template <size_t PageSize>
bool BitmapPage<PageSize>::AllocatePage(uint32_t &page_offset) {
	uint32_t MaxSupportedSize = GetMaxSupportedSize();

	if (page_allocated_ == MaxSupportedSize) {
		return false;
	}

	bytes[next_free_page_ / 8] |= 1 << (next_free_page_ % 8);
	page_offset = next_free_page_;
	++page_allocated_;

	if (page_allocated_ < MaxSupportedSize) {
		while (!IsPageFree(next_free_page_)) {
			next_free_page_ = (next_free_page_ + 1) % MaxSupportedSize;
		}
	}

	return true;
}

/**
 * TODO: Student Implement
 */
template <size_t PageSize>
bool BitmapPage<PageSize>::DeAllocatePage(uint32_t page_offset) {
	if (page_offset >= GetMaxSupportedSize() || IsPageFree(page_offset)) {
		return false;
	}

	bytes[page_offset / 8] ^= 1 << (page_offset % 8);
	next_free_page_ = page_offset;
	--page_allocated_;

	return true;
}

/**
 * TODO: Student Implement
 */
template <size_t PageSize>
bool BitmapPage<PageSize>::IsPageFree(uint32_t page_offset) const {
	if (page_offset >= GetMaxSupportedSize()) {
		return false;
	}

	return IsPageFreeLow(page_offset / 8, page_offset % 8);
}

template <size_t PageSize>
bool BitmapPage<PageSize>::IsPageFreeLow(uint32_t byte_index, uint8_t bit_index) const {
	return (bytes[byte_index] & (1 << bit_index)) == 0;
}

template class BitmapPage<64>;

template class BitmapPage<128>;

template class BitmapPage<256>;

template class BitmapPage<512>;

template class BitmapPage<1024>;

template class BitmapPage<2048>;

template class BitmapPage<4096>;

// #include "page/bitmap_page.h"

// #include "glog/logging.h"

// /**
//  * TODO: Student Implement
//  */
// template <size_t PageSize>
// bool BitmapPage<PageSize>::AllocatePage(uint32_t &page_offset) {
// 	uint32_t MaxSupportedSize = GetMaxSupportedSize();

// 	if (page_allocated_ == MaxSupportedSize) {
// 		return false;
// 	}

// 	bytes[next_free_page_ / 8] |= 1 << (next_free_page_ % 8);
// 	page_offset = next_free_page_;
// 	++page_allocated_;

// 	if (page_allocated_ < MaxSupportedSize) {
// 		while (!IsPageFree(next_free_page_)) {
// 			next_free_page_ = (next_free_page_ + 1) % MaxSupportedSize;
// 		}
// 	}

// 	return true;
// }

// /**
//  * TODO: Student Implement
//  */
// template <size_t PageSize>
// bool BitmapPage<PageSize>::DeAllocatePage(uint32_t page_offset) {
// 	if (page_offset >= GetMaxSupportedSize() || IsPageFree(page_offset)) {
// 		return false;
// 	}

// 	bytes[page_offset / 8] ^= 1 << (page_offset % 8);
// 	next_free_page_ = page_offset;
// 	--page_allocated_;

// 	return true;
// }

// /**
//  * TODO: Student Implement
//  */
// template <size_t PageSize>
// bool BitmapPage<PageSize>::IsPageFree(uint32_t page_offset) const {
// 	if (page_offset >= GetMaxSupportedSize()) {
// 		return false;
// 	}

// 	return IsPageFreeLow(page_offset / 8, page_offset % 8);
// }

// template <size_t PageSize>
// bool BitmapPage<PageSize>::IsPageFreeLow(uint32_t byte_index, uint8_t bit_index) const {
// 	return (bytes[byte_index] & (1 << bit_index)) == 0;
// }

// template class BitmapPage<64>;

// template class BitmapPage<128>;

// template class BitmapPage<256>;

// template class BitmapPage<512>;

// template class BitmapPage<1024>;

// template class BitmapPage<2048>;

// template class BitmapPage<4096>;