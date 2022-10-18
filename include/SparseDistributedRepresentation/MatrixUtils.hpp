#pragma once

namespace sparse_distributed_representation {

namespace matrix_utils {

template<typename major_t>
// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
struct row_info {
    using major_type = major_t;

    // this stores information about each row
    typename major_t::id_type id;
    // pos must always point to a valid element
    typename major_t::data_type::const_iterator pos;
    typename major_t::data_type::const_iterator end;
    bool operator<(const row_info& other) const {
        return pos->id() > other.pos->id(); // intentional greater
    }
};

/**
 * this gives an iterator-like interface, which provides elements of a matrix in a view opposite to how it is stored
 * e.g. viewing the elements of a row-wise matrix in a column-wise format
 *  the elements are provided as follows
 *      [ 1 2 ]
 *      [ 3 4 ]  ->  1 3 2 4
 */
template<typename priority_queue_container_t>
class OtherMajorView {
    private:
        using row_info_type = typename priority_queue_container_t::value_type;
        using major_type = typename row_info_type::major_type;

        // store the infos in the same containers as the major rows are stored in the matrix
        priority_queue_container_t row_infos;

    public:
        // dereferencing OtherMajorView yields a Position
        struct Position {
            typename major_type::id_type major_id;
            // element includes both the minor id and the data itself
            typename major_type::data_type::const_pointer element;
        };

        /**
         * Add each row/column of the matrix to the view BEFORE doing anything else
         */
        void add_major(const major_type& row) {
            // should never happen since an SDR that is empty is irrelevant, and should have been ommitted
            assert(!row.data().empty());
            row_infos.push(row_info_type{row.id(), row.data().cbegin(), row.data().cend()});
        }

        operator bool() const {
            return !row_infos.empty();
        }

        Position operator*() const {
            const row_info_type& elem = row_infos.top();
            return Position{elem.id, &*elem.pos};
        }

        void operator++() {
            row_info_type elem = row_infos.top();
            row_infos.pop();
            ++elem.pos;
            if (elem.pos != elem.end) {
                row_infos.push(elem);
            }
        }
};

namespace {
// clang-7 was having trouble deducing the type for output_it/bucket_it (below)
struct Empty {
    char unused[0];
};

template<typename T>
auto get_output_it(const T& t) {
    if constexpr(flist_like<typename T::container_type>::value) {
        return t.cbefore_begin();
    } else {
        // making sure that the iterators here don't take up any space if they are not needed
        return Empty();
    }
}

} // namespace

/**
 * this should be used in conjunction with an OtherMajorView
 * It has a "bucket" and an "output".
 * Same id elements are accumulated in the bucket before being sent to the output (via ori).
 * 
 * @tparam T the matrix being outputted to.
 */
template<typename T>
class BucketOutputAccumulator {
    private:
        T& output;
        typename T::value_type bucket;

        void flush() {
            if (bucket.data().relevant()) {
                if constexpr(flist_like<typename T::container_type>::value) {
                    output_it = output.insert_after(output_it, std::move(bucket));
                } else {
                    output.push_back(std::move(bucket));
                }
            }
        }

        decltype(get_output_it(T())) output_it;

    public:
        BucketOutputAccumulator(T& output) : output(output),
                                             bucket(),
                                             output_it(get_output_it(output)) {}

        void operator()(typename T::value_type&& elem) {
            if (bucket.id() == elem.id()) {
                bucket.data().ori(elem.data());
            } else {
                flush();
                // bucket is moved-from after the call to flush
                bucket = std::move(elem);
            }
        }

        ~BucketOutputAccumulator() {
            flush();
        }
};

/**
 * this should be used in conjunction with an OtherMajorView
 * It has a "bucket" and an "output".
 * Same id elements are appended in the bucket before being sent to the output (via push_back/insert_after).
 * 
 * @tparam T the matrix being outputted to.
 */
template<typename T>
class BucketOutputAppender {
    private:
        T& output;
        typename T::value_type bucket;

        struct Empty {
            char unused[0];
        };

        void flush() {
            if (bucket.data().relevant()) {
                if constexpr(flist_like<typename T::container_type>::value) {
                    output_it = output.insert_after(output_it, std::move(bucket));
                } else {
                    output.push_back(std::move(bucket));
                }
            }
        }

        decltype(get_output_it(T())) output_it;
        decltype(get_output_it(typename T::value_type::data_type())) bucket_it;

    public:
        BucketOutputAppender(T& output) : output(output),
                                          bucket(),
                                          output_it(get_output_it(output)),
                                          bucket_it(get_output_it(bucket.data())) {}

        void send(typename T::value_type::id_type id, typename T::value_type::data_type::value_type&& data) {
            if (id != bucket.id()) {
                flush();
                // bucket is moved-from after the call to flush
                bucket = typename T::value_type(id);
                bucket_it = get_output_it(bucket.data());
            }
            if constexpr(flist_like<typename T::value_type::data_type::container_type>::value) {
                bucket_it = bucket.data().insert_after(bucket_it, std::move(data));
            } else {
                bucket.data().push_back(std::move(data));
            }
        }
        
        ~BucketOutputAppender() {
            flush();
        }
};

} // namespace

} // namespace