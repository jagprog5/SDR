#include <vector>
#include <iterator>

namespace sparse_distributed_representation {

namespace id_contiguous_container_objs {

// nearly identical to SDRElemReference (below), except it uses const pointers and does not have getters or setters
template<typename SDRElem_t>
class SDRElemConstReference {
    public:
        using id_type = const typename SDRElem_t::id_type;
        using data_type = const typename SDRElem_t::data_type;

        SDRElemConstReference(id_type* ids_ptr, data_type* datas_ptr)
            : id_(ids_ptr), data_(datas_ptr) {}

        explicit SDRElemConstReference(const SDRElem_t& elem) : id_(&elem.id()), data_(&elem.data()) {}

        explicit operator SDRElem_t() const { return SDRElem_t(id(), data()); }

        id_type& id() const { return *id_; }
        data_type& data() const { return *data_; }

        bool operator<(id_type o) const { return id() < o; }
        bool operator==(id_type o) const { return id() == o; }
        bool operator>(id_type o) const { return id() > o; }

        bool operator<(const SDRElemConstReference& o) const { return id() < o.id(); }
        bool operator==(const SDRElemConstReference& o) const { return id() == o.id(); }
        bool operator>(const SDRElemConstReference& o) const { return id() > o.id(); }

        template<typename id_other, typename data_other>
        bool operator<(const SDRElem<id_other, data_other>& o) const { return id() < o.id(); }
        template<typename id_other, typename data_other>
        bool operator==(const SDRElem<id_other, data_other>& o) const { return id() == o.id(); }
        template<typename id_other, typename data_other>
        bool operator>(const SDRElem<id_other, data_other>& o) const { return id() > o.id(); }

        template<typename id_other, typename data_other>
        friend bool operator<(const SDRElem<id_other, data_other>& a, const SDRElemConstReference& b) { return a.id() < b.id(); }
        template<typename id_other, typename data_other>
        friend bool operator==(const SDRElem<id_other, data_other>& a, const SDRElemConstReference& b) { return a.id() == b.id(); }
        template<typename id_other, typename data_other>
        friend bool operator>(const SDRElem<id_other, data_other>& a, const SDRElemConstReference& b) { return a.id() > b.id(); }

        friend std::ostream& operator<<(std::ostream& os, const SDRElemConstReference& o) {
            os << '*' << o.id();
            // NOLINTNEXTLINE
            if constexpr(sizeof(data_type) > 0) {
                os << "(" << o.data() << ")";
            }
            return os;
        }

    private:
        id_type* id_;
        data_type* data_;
};

// this mimics an SDRElem.
// it is a "reference" in the sense that it stores pointers to the id and data but you use the members directly.
// if you dereference an iterator to IDContiguousContainer, you get an instance of this type
template<typename SDRElem_t>
class SDRElemReference {
    public:
        using id_type = const typename SDRElem_t::id_type;
        using data_type = typename SDRElem_t::data_type;

        explicit SDRElemReference(SDRElem_t& elem) : id_(&elem.id()), data_(&elem.data()) {}

        SDRElemReference(id_type* ids_ptr, data_type* datas_ptr) : id_(ids_ptr), data_(datas_ptr) {}

        explicit operator SDRElem_t() const { return SDRElem_t(id(), data()); }

        // needed so that move iterator functions correctly
        explicit operator SDRElem_t&&() {
            SDRElem_t ret(id(), std::move(data()));
            return std::move(ret);
        }

        id_type& id() const { return *id_; }
        data_type& data() { return *data_; }
        const data_type& data() const { return *data_; }
        void data(data_type data) { *data_ = data; }

        SDRElemReference& operator=(const SDRElem_t& o) {
            const_cast<typename SDRElem_t::id_type&>(id()) = o.id();
            data() = o.data();
            return *this;
        }

        SDRElemReference& operator=(SDRElem_t&& o) noexcept {
            const_cast<typename SDRElem_t::id_type&>(id()) = o.id();
            data() = std::move(o.data());
            return *this;
        }

        SDRElemReference& operator=(const SDRElemConstReference<SDRElem_t>& o) {
            const_cast<typename SDRElem_t::id_type&>(id()) = o.id();
            data() = o.data();
            return *this;
        }

        SDRElemReference& operator=(SDRElemConstReference<SDRElem_t>&& o) noexcept {
            const_cast<typename SDRElem_t::id_type&>(id()) = o.id();
            data() = std::move(o.data());
            return *this;
        }

        bool operator<(id_type o) const { return id() < o; }
        bool operator==(id_type o) const { return id() == o; }
        bool operator>(id_type o) const { return id() > o; }

        bool operator<(const SDRElemReference& o) const { return id() < o.id(); }
        bool operator==(const SDRElemReference& o) const { return id() == o.id(); }
        bool operator>(const SDRElemReference& o) const { return id() > o.id(); }

        template<typename id_other, typename data_other>
        bool operator<(const SDRElem<id_other, data_other>& o) const { return id() < o.id(); }
        template<typename id_other, typename data_other>
        bool operator==(const SDRElem<id_other, data_other>& o) const { return id() == o.id(); }
        template<typename id_other, typename data_other>
        bool operator>(const SDRElem<id_other, data_other>& o) const { return id() > o.id(); }

        template<typename id_other, typename data_other>
        friend bool operator<(const SDRElem<id_other, data_other>& a, const SDRElemReference& b) { return a.id() < b.id(); }
        template<typename id_other, typename data_other>
        friend bool operator==(const SDRElem<id_other, data_other>& a, const SDRElemReference& b) { return a.id() == b.id(); }
        template<typename id_other, typename data_other>
        friend bool operator>(const SDRElem<id_other, data_other>& a, const SDRElemReference& b) { return a.id() > b.id(); }

        friend std::ostream& operator<<(std::ostream& os, const SDRElemReference& o) {
            os << '*' << o.id();
            // NOLINTNEXTLINE
            if constexpr(sizeof(data_type) > 0) {
                os << "(" << o.data() << ")";
            }
            return os;
        }

    private:
        id_type* id_;
        data_type* data_;
};

template<typename SDRElem_t, typename ids_t, typename datas_t>
class Iterator;

template<typename SDRElem_t, typename ids_t, typename datas_t>
class ConstIterator;

template<typename SDRElem_t, typename ids_t, typename datas_t>
class Iterator {
    public:
        using iterator_category = std::random_access_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = SDRElem_t;
        using reference = SDRElemReference<SDRElem_t>;
        using pointer = SDRElemReference<SDRElem_t>*;

        Iterator(typename ids_t::iterator ids_ptr, typename datas_t::iterator datas_ptr)
            : ids_ptr(ids_ptr), datas_ptr(datas_ptr) {}

        Iterator() {}

        reference operator*() const {
            // this is fine since they have the exact same members
            return *(SDRElemReference<SDRElem_t>*)this;
        }
        
        pointer operator->() { return (pointer)this; }

        Iterator& operator++() { ids_ptr++; datas_ptr++; return *this; }
        Iterator operator++(int) { Iterator tmp = *this; ++(*this); return tmp; }
        Iterator& operator--() { ids_ptr--; datas_ptr--; return *this; }
        Iterator operator--(int) { Iterator tmp = *this; --(*this); return tmp; }
        Iterator& operator+=(difference_type n) { ids_ptr += n; datas_ptr += n; return *this; }
        Iterator& operator-=(difference_type n) { ids_ptr -= n; datas_ptr -= n; return *this; }
        difference_type operator-(Iterator o) { return ids_ptr - o.ids_ptr; }
        
        bool operator==(const Iterator& o) { return ids_ptr == o.ids_ptr; };
        bool operator!=(const Iterator& o) { return ids_ptr != o.ids_ptr; };

        template<typename SDRElem_t_inner, typename ids_t_inner, typename datas_t_inner>
        friend class ConstIterator;
    
    private:
        typename ids_t::iterator ids_ptr;
        typename datas_t::iterator datas_ptr;
};

// "curiously recurring template pattern", but I don't have access to the base
template<typename SDRElem_t, typename ids_t, typename datas_t>
struct CRTPWorkaround : public Iterator<SDRElem_t, ids_t, datas_t> { };

template<typename SDRElem_t, typename ids_t, typename datas_t>
class ConstIterator {
    public:
        using iterator_category = std::random_access_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = const SDRElem_t; 
        using reference = SDRElemConstReference<SDRElem_t>;
        using pointer = SDRElemConstReference<SDRElem_t>*;

        ConstIterator(typename ids_t::const_iterator ids_ptr, typename datas_t::const_iterator datas_ptr)
            : ids_ptr(ids_ptr), datas_ptr(datas_ptr) {}

        ConstIterator() {}

        reference operator*() const {
            // this is fine since they have the exact same members
            return *(SDRElemConstReference<SDRElem_t>*)this;
        }

        pointer operator->() { return (pointer)this; }

        ConstIterator& operator++() { ids_ptr++; datas_ptr++; return *this; }
        ConstIterator operator++(int) { ConstIterator tmp = *this; ++(*this); return tmp; }
        ConstIterator& operator--() { ids_ptr--; datas_ptr--; return *this; }
        ConstIterator operator--(int) { ConstIterator tmp = *this; --(*this); return tmp; }
        ConstIterator& operator+=(difference_type n) { ids_ptr += n; datas_ptr += n; return *this; }
        ConstIterator& operator-=(difference_type n) { ids_ptr -= n; datas_ptr -= n; return *this; }
        difference_type operator-(ConstIterator o) { return ids_ptr - o.ids_ptr; }

        bool operator==(const ConstIterator& o) { return ids_ptr == o.ids_ptr; };
        bool operator!=(const ConstIterator& o) { return ids_ptr != o.ids_ptr; };
        bool operator!=(const Iterator<SDRElem_t, ids_t, datas_t>& o) { return ids_ptr != o.ids_ptr; };
    private:
        typename ids_t::const_iterator ids_ptr;
        typename datas_t::const_iterator datas_ptr;
};

} // namespace id_contiguous_container_objs

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
        using size_type = typename ids_t::size_type;
        using iterator = id_contiguous_container_objs::Iterator<SDRElem_t, ids_t, datas_t>;
        using const_iterator = id_contiguous_container_objs::ConstIterator<SDRElem_t, ids_t, datas_t>;
        using reference = id_contiguous_container_objs::SDRElemReference<SDRElem_t>;
        using const_reference = id_contiguous_container_objs::SDRElemConstReference<SDRElem_t>;
        using pointer = id_contiguous_container_objs::Iterator<SDRElem_t, ids_t, datas_t>;
        using const_pointer = id_contiguous_container_objs::ConstIterator<SDRElem_t, ids_t, datas_t>;
        using value_type = SDRElem_t;
        using ids_type = ids_t;
        using datas_type = datas_t;

        size_type size() const { return ids.size(); }
        bool empty() const { return size() == 0; }

        iterator begin() { return iterator{ids.begin(), datas.begin()}; }
        iterator end() { return iterator{ids.end(), datas.end()}; }

        const_iterator cbegin() const { return const_iterator(ids.cbegin(), datas.cbegin()); }
        const_iterator cend() const { return const_iterator(ids.cend(), datas.cend()); }

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

} // namespace

namespace std {

template<typename SDRElem_t, typename ids_t, typename datas_t>
struct move_iterator<typename sparse_distributed_representation::id_contiguous_container_objs::Iterator<SDRElem_t, ids_t, datas_t>>
    : public move_iterator<typename sparse_distributed_representation::id_contiguous_container_objs::CRTPWorkaround<SDRElem_t, ids_t, datas_t>> {
    // under-the-hood, move_iterator casts the output from dereferencing the underlying iterator to move_iterator::reference.
    // move_iterator::reference is based on whatever the underlying iterator labels as a reference
    // if the underlying iterator's reference type is a value (in which case it is, SDRElemReference just acts like a ref but it is not),
    // then move_iterator::reference is also a value type. this is bad; it causes an unnecessary copy

    using parent_iter_t = typename sparse_distributed_representation::id_contiguous_container_objs::CRTPWorkaround<SDRElem_t, ids_t, datas_t>;
    using this_iter_t = typename sparse_distributed_representation::id_contiguous_container_objs::Iterator<SDRElem_t, ids_t, datas_t>;

    move_iterator(this_iter_t p) : move_iterator<parent_iter_t>(*(parent_iter_t*)&p) {}
    move_iterator(move_iterator<parent_iter_t> p) : move_iterator<parent_iter_t>(p) {}

    auto operator*() const {
        auto r = *this->base();
        SDRElem_t ret(r.id(), std::move(r.data()));
        return ret; // nrvo
    }

    auto operator[](typename sparse_distributed_representation::id_contiguous_container_objs::Iterator<SDRElem_t, ids_t, datas_t>::difference_type __n) const {
        auto r = this->base()[__n];
        SDRElem_t ret(r.id(), std::move(r.data()));
        return ret; // nrvo
    }
};

} // namespace