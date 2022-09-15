#include <vector>
#include <iterator>

// instead of ordering the data like this:
//      id0 data0 id1 data1 id2 data2
// do it like this:
//      ids -> id0 id1 id2
//      datas -> data0 data1 data2
//
// this gives better cache access especially for ops like ors, which don't access the data elements
// the id is provided immediately, and only if the data is required then it will dereference to access it
//
// Only the neccessary functions have been implemented (for use in SDR).
template<typename SDRElem_t, typename ids_t = std::vector<typename SDRElem_t::id_type>, typename datas_t = std::vector<typename SDRElem_t::data_type>>
class IDContiguousContainer {
    private:
        ids_t ids;
        datas_t datas;
    public:
        // const; SDRElemReference without data getter or setter
        class SDRElemConstReference : SDRElem<const typename SDRElem_t::id_type*, const typename SDRElem_t::data_type*> {
            public:
                using id_type = const typename SDRElem_t::id_type;
                using data_type = const typename SDRElem_t::data_type;
                using parent = SDRElem<id_type*, data_type*>;

                SDRElemConstReference(id_type* ids_ptr, data_type* datas_ptr) : parent(ids_ptr, datas_ptr) {}

                // constexpr bool operator==(const parent& o) const { return id() == o.id(); }
                // friend constexpr bool operator==(const SDRElemConstReference& a, const parent& b) { return a.id() == b.id(); }

                // template<typename id_other, typename data_other>
                // constexpr bool operator==(const SDRElem<id_other*, data_other*>& o) const { return id() == *o.id(); }

                explicit operator SDRElem_t() const { return SDRElem_t(id(), data()); }

                constexpr const id_type& id() const { return *parent::id(); }
                constexpr const data_type& data() const { return *parent::data(); }
                
                template<typename id_other, typename data_other>
                constexpr bool operator<(const SDRElem<id_other, data_other>& o) const { return id() < o.id(); }
                template<typename id_other, typename data_other>
                constexpr bool operator==(const SDRElem<id_other, data_other>& o) const { return id() == o.id(); }
                template<typename id_other, typename data_other>
                constexpr bool operator>(const SDRElem<id_other, data_other>& o) const { return id() > o.id(); }

                constexpr bool operator<(const id_type& o) const { return id() < o; }
                constexpr bool operator==(const id_type& o) const { return id() == o; }
                constexpr bool operator>(const id_type& o) const { return id() > o; }
        };

        // acts like a SDRElem, but under the hood is a pointer to the id and data
        // it is a "reference" in the sense that it is actually a pointer, but you use it directly
        class SDRElemReference : SDRElem<typename SDRElem_t::id_type*, typename SDRElem_t::data_type*> {
            public:
                using id_type = typename SDRElem_t::id_type;
                using data_type = typename SDRElem_t::data_type;
                using parent = SDRElem<id_type*, data_type*>;

                SDRElemReference(id_type* ids_ptr, data_type* datas_ptr) : parent(ids_ptr, datas_ptr) {}

                explicit operator SDRElem_t() const { return SDRElem_t(id(), data()); }

                constexpr const id_type& id() const { return *parent::id(); }
                constexpr data_type& data() { return *parent::data(); }
                constexpr const data_type& data() const { return *parent::data(); }
                constexpr void data(data_type data) { *parent::data() = data; }

                constexpr bool operator<(parent p) { return id() < p.id(); }
                constexpr bool operator==(parent p) { return id() == p.id(); }
                constexpr bool operator>(parent p) { return id() > p.id(); }

                template<typename id_other, typename data_other>
                constexpr bool operator<(const SDRElem<id_other, data_other>& o) const { return id() < o.id(); }
                template<typename id_other, typename data_other>
                constexpr bool operator==(const SDRElem<id_other, data_other>& o) const { return id() == o.id(); }
                template<typename id_other, typename data_other>
                constexpr bool operator>(const SDRElem<id_other, data_other>& o) const { return id() > o.id(); }

                constexpr bool operator<(const id_type& o) const { return id() < o; }
                constexpr bool operator==(const id_type& o) const { return id() == o; }
                constexpr bool operator>(const id_type& o) const { return id() > o; }

                constexpr SDRElemReference& operator=(const SDRElem_t& o) {
                    const_cast<id_type&>(id()) = o.id();
                    data() = o.data();
                    return *this;
                }

                constexpr SDRElemReference& operator=(SDRElem_t&& o) noexcept {
                    const_cast<id_type&>(id()) = o.id();
                    data() = std::move(o.data());
                    return *this;
                }

                constexpr SDRElemReference& operator=(const SDRElemConstReference& o) {
                    const_cast<id_type&>(id()) = o.id();
                    data() = o.data();
                    return *this;
                }

                constexpr SDRElemReference& operator=(SDRElemConstReference&& o) noexcept {
                    const_cast<id_type&>(id()) = o.id();
                    data() = std::move(o.data());
                    return *this;
                }
        };

        // SDRElemDataConstReference is the by **value** access of an element in this container
        // the id is by value, but the data is by reference,
        // this is a const ref to prevent confusion from accidentally modifying the data,
        // rather than a copy of the data, which would be expected in typical use cases
        class SDRElemDataConstReference : SDRElem<typename SDRElem_t::id_type, const typename SDRElem_t::data_type*> {
            public:
                using id_type = typename SDRElem_t::id_type;
                using data_type = const typename SDRElem_t::data_type;
                using parent = SDRElem<typename SDRElem_t::id_type, const typename SDRElem_t::data_type*>;
                using parent::id;
                constexpr const data_type& data() const { return *parent::data(); }
                using parent::operator<;
                using parent::operator==;
                using parent::operator>;
        };

        class ConstIterator;

        class Iterator {
            public:
                using iterator_category = std::random_access_iterator_tag;
                using difference_type = std::ptrdiff_t;
                using value_type = SDRElemDataConstReference;
                using reference = SDRElemReference;
                using pointer = SDRElemReference*;

                Iterator(typename ids_t::iterator ids_ptr, typename datas_t::iterator datas_ptr)
                    : ids_ptr(ids_ptr), datas_ptr(datas_ptr) {}

                Iterator() {}

                reference operator*() const { return SDRElemReference(&*ids_ptr, &*datas_ptr); }
                pointer operator->() {
                    // this is fine since they have the exact same members
                    return (SDRElemReference*)this;
                }

                Iterator& operator++() { ids_ptr++; datas_ptr++; return *this; }
                Iterator operator++(int) { Iterator tmp = *this; ++(*this); return tmp; }
                Iterator& operator--() { ids_ptr--; datas_ptr--; return *this; }
                Iterator operator--(int) { Iterator tmp = *this; --(*this); return tmp; }
                Iterator& operator+=(difference_type n) { ids_ptr += n; datas_ptr += n; return *this; }
                Iterator& operator-=(difference_type n) { ids_ptr -= n; datas_ptr -= n; return *this; }
                difference_type operator-(Iterator o) { return ids_ptr - o.ids_ptr; }
                
                bool operator==(const Iterator& o) { return ids_ptr == o.ids_ptr; };
                bool operator!=(const Iterator& o) { return ids_ptr != o.ids_ptr; };
                friend class ConstIterator;
            
            private:
                typename ids_t::iterator ids_ptr;
                typename datas_t::iterator datas_ptr;
        };

        class ConstIterator {
            public:
                using iterator_category = std::random_access_iterator_tag;
                using difference_type = std::ptrdiff_t;
                using value_type = SDRElemDataConstReference; 
                using reference = SDRElemConstReference;
                using pointer = SDRElemConstReference*;

                ConstIterator(typename ids_t::const_iterator ids_ptr, typename datas_t::const_iterator datas_ptr)
                    : ids_ptr(ids_ptr), datas_ptr(datas_ptr) {}

                ConstIterator() {}

                reference operator*() const { return SDRElemConstReference(&*ids_ptr, &*datas_ptr); }
                pointer operator->() {
                    // this is fine since they have the exact same members
                    return (SDRElemConstReference*)this;
                }

                ConstIterator& operator++() { ids_ptr++; datas_ptr++; return *this; }
                ConstIterator operator++(int) { ConstIterator tmp = *this; ++(*this); return tmp; }
                ConstIterator& operator--() { ids_ptr--; datas_ptr--; return *this; }
                ConstIterator operator--(int) { ConstIterator tmp = *this; --(*this); return tmp; }
                ConstIterator& operator+=(difference_type n) { ids_ptr += n; datas_ptr += n; return *this; }
                ConstIterator& operator-=(difference_type n) { ids_ptr -= n; datas_ptr -= n; return *this; }
                difference_type operator-(ConstIterator o) { return ids_ptr - o.ids_ptr; }

                bool operator==(const ConstIterator& o) { return ids_ptr == o.ids_ptr; };
                bool operator!=(const ConstIterator& o) { return ids_ptr != o.ids_ptr; };
                bool operator!=(const Iterator& o) { return ids_ptr != o.ids_ptr; };
            private:
                typename ids_t::const_iterator ids_ptr;
                typename datas_t::const_iterator datas_ptr;
        };

        using size_type = typename ids_t::size_type;
        using iterator = Iterator;
        using const_iterator = ConstIterator;
        using reference = SDRElemReference;
        using const_reference = SDRElemConstReference;
        using pointer = Iterator;
        using const_pointer = ConstIterator;
        using value_type = SDRElemDataConstReference;

        size_type size() const { return ids.size(); }
        bool empty() const { return size() == 0; }

        iterator begin() { return Iterator{ids.begin(), datas.begin()}; }
        iterator end() { return Iterator{ids.end(), datas.end()}; }

        const_iterator cbegin() const { return ConstIterator(ids.cbegin(), datas.cbegin()); }
        const_iterator cend() const { return ConstIterator(ids.cend(), datas.cend()); }

        void push_back(SDRElem_t&& elem) {
            ids.push_back(std::move(elem.id()));
            datas.push_back(std::move(elem.data()));
        }

        void push_back(const SDRElem_t& elem) {
            ids.push_back(elem.id());
            datas.push_back(elem.data());
        }

       void pop_back() {
        ids.pop_back();
        datas.pop_back();
       }

        void resize(size_type size) {
            ids.resize(size);
            datas.resize(size);
        }

        void clear() {
            ids.clear();
            datas.clear();
        }

        void shrink_to_fit() {
            ids.shrink_to_fit();
            datas.shrink_to_fit();
        }
};