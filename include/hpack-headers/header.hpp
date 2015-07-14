﻿/*
    Copyright (c) 2011-2015 Anton Matveenko <antmat@yandex-team.ru>
    Copyright (c) 2011-2015 Other contributors as noted in the AUTHORS file.

    This file is part of Cocaine.

    Cocaine is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    Cocaine is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <boost/mpl/contains.hpp>
#include <boost/mpl/find.hpp>
#include <boost/mpl/for_each.hpp>
#include <boost/mpl/size.hpp>
#include <boost/mpl/vector.hpp>

#include <array>
#include <cassert>
#include <cstring>
#include <iostream>
#include <vector>

#include "boost/mpl/vector/vector50.hpp"
#include "boost/mpl/aux_/config/ctps.hpp"
#include "boost/preprocessor/iterate.hpp"
#include "boost/config.hpp"

// Some woodoo magic of boost mpl.
// This allows to generate mpl vector larger than 50
// See http://comments.gmane.org/gmane.comp.lib.boost.user/6986

namespace boost { namespace mpl {
#define BOOST_PP_ITERATION_PARAMS_1 \
    (3,(51, 100, "boost/mpl/vector/aux_/numbered.hpp"))
#include BOOST_PP_ITERATE()
}}

namespace hpack {

struct init_header_t;

size_t
http2_integer_size(size_t sz, size_t bit_offset);

size_t
http2_integer_encode(unsigned char* dest, uint64_t source, size_t bit_offset, char prefix);

namespace header {

struct data_t {
    const char* blob;
    size_t size;
    bool operator==(const data_t& other) const;

    template<class To>
    To
    convert() {
        static_assert(std::is_pod<typename std::remove_reference<To>::type>::value &&
                      !std::is_pointer<typename std::remove_reference<To>::type>::value &&
                      !std::is_array<typename std::remove_reference<To>::type>::value,
                      "only POD non pointer, non array data type is allowed to convert header data"
                      );
        if(size != sizeof(typename std::remove_reference<To>::type)) {
            throw std::system_error(std::make_error_code(std::errc::invalid_argument), "invalid header data size");
        }
        return *(reinterpret_cast<const To*>(blob));
    }
};

template<size_t N>
inline
constexpr
data_t
create_data(char const (&source)[N]) {
    return data_t{source, N-1};
}

template<class From>
inline
constexpr
header::data_t
create_data(From&& source) {
    static_assert(std::is_lvalue_reference<From>::value &&
                  std::is_pod<typename std::remove_reference<From>::type>::value &&
                  !std::is_same<const char*, typename std::remove_reference<From>::type>::value,
                  "only lreference to POD is allowed to create header data"
                  );
    return data_t {
        reinterpret_cast<const char*>(&source),
        sizeof(typename std::remove_reference<From>::type)
    };
}

inline
constexpr
data_t
create_data(const char* source, size_t size) {
    return data_t{source, size};
}
} // namespace header

struct headers;

// Header class.
// Represents non-owning header referring to some external memory.
// Can be constructed only via header table
class header_t {
public:
    /**
     * @brief Very simple storage for non-owning headers,
     * which can be used to extend header_t validity.
     * Does not provide cleanup capbilities for performance and complexity reasons.
     */
    class zone_t {
    public:
        zone_t() = default;
        zone_t(zone_t&&) = default;
        zone_t& operator=(zone_t&&) = default;
        zone_t(const zone_t&) = delete;
        zone_t& operator=(const zone_t&) = delete;


        /**
         * @brief construct zone_t by copying header data to header_zone and point header to that data.
         * @param header - header to rebind.
         */
        zone_t(header_t& header);

        /**
         * @brief rebind pack of headers at once (single allocation)
         * @param headers
         */
        zone_t(std::vector<header_t>& headers);

    private:
        void
        reserve(size_t size);

        /**
         * @brief copy header data to header_zone and point header to that data.
         * @param header - header to rebind.
         */
        void
        rebind_header(header_t& header);

        /**
         * @brief rebind pack of headers in a more efficient way (single allocation)
         * @param headers - vector of headers.
         */
        void
        rebind_headers(std::vector<header_t>& headers);

        std::vector<char> storage;
    };

    header_t(const header_t&) = default;
    header_t& operator=(const header_t&) = default;
    header_t(): name(), value() {}

    // Create non-owning header on user-provided data
    template<class Header>
    static
    header_t
    create(header::data_t _value) {
        return header_t(Header::name, _value);
    }

    header::data_t
    get_name() const;

    header::data_t
    get_value() const;

    bool
    operator==(const header_t& other) const;

    bool
    name_equal(const header_t& other) const;

    // Returns size of the header entry in http2 header table
    size_t
    http2_size() const;

    friend struct msgpack_traits;
    friend struct headers;
    friend struct header_static_table_t;
    friend struct init_header_t;

private:
    friend class header_table_t;

    header_t(const header::data_t& name, const header::data_t& value) noexcept;

    header_t(const char* name, size_t name_sz, const char* value, size_t value_sz) noexcept;

    header::data_t name;
    header::data_t value;
};

} // namespace hpack

#include "header_definitions.hpp"

namespace hpack {

struct header_static_table_t {
    typedef boost::mpl::vector83<
        headers::detail::empty_placeholder, // 0. Reserved
        headers::authority<>,
        headers::method<headers::default_values_t::get_value_t>,
        headers::method<headers::default_values_t::post_value_t>,
        headers::path<headers::default_values_t::root_value_t>,
        headers::path<headers::default_values_t::index_value_t>,
        headers::scheme<headers::default_values_t::http_value_t>,
        headers::scheme<headers::default_values_t::https_value_t>,
        headers::status<headers::default_values_t::status_200_value_t>,
        headers::status<headers::default_values_t::status_204_value_t>,
        headers::status<headers::default_values_t::status_206_value_t>,
        headers::status<headers::default_values_t::status_304_value_t>,
        headers::status<headers::default_values_t::status_400_value_t>,
        headers::status<headers::default_values_t::status_404_value_t>,
        headers::status<headers::default_values_t::status_500_value_t>,
        headers::accept_charset<>,
        headers::accept_encoding<headers::default_values_t::gzip_value_t>,
        headers::accept_language<>,
        headers::accept_ranges<>,
        headers::accept<>,
        headers::access_control_allow_origin<>,
        headers::age<>,
        headers::allow<>,
        headers::authorization<>,
        headers::cache_control<>,
        headers::content_disposition<>,
        headers::content_encoding<>,
        headers::content_language<>,
        headers::content_length<>,
        headers::content_location<>,
        headers::content_range<>,
        headers::content_type<>,
        headers::cookie<>,
        headers::date<>,
        headers::etag<>,
        headers::expect<>,
        headers::expires<>,
        headers::from<>,
        headers::host<>,
        headers::if_match<>,
        headers::if_modified_since<>,
        headers::if_none_match<>,
        headers::if_range<>,
        headers::if_unmodified_since<>,
        headers::last_modified<>,
        headers::link<>,
        headers::location<>,
        headers::max_forwards<>,
        headers::proxy_authenticate<>,
        headers::proxy_authorization<>,
        headers::range<>,
        headers::referer<>,
        headers::refresh<>,
        headers::retry_after<>,
        headers::server<>,
        headers::set_cookie<>,
        headers::strict_transport_security<>,
        headers::transfer_encoding<>,
        headers::user_agent<>,
        headers::vary<>,
        headers::via<>,
        headers::www_authenticate<>, // 61
        // Reserved for http2 extensions
        headers::detail::empty_placeholder,
        headers::detail::empty_placeholder,
        headers::detail::empty_placeholder,
        headers::detail::empty_placeholder,
        headers::detail::empty_placeholder,
        headers::detail::empty_placeholder,
        headers::detail::empty_placeholder,
        headers::detail::empty_placeholder,
        headers::detail::empty_placeholder,
        headers::detail::empty_placeholder,
        headers::detail::empty_placeholder,
        headers::detail::empty_placeholder,
        headers::detail::empty_placeholder,
        headers::detail::empty_placeholder,
        headers::detail::empty_placeholder,
        headers::detail::empty_placeholder,
        headers::detail::empty_placeholder,
        headers::detail::empty_placeholder, // 79
        // Cocaine specific headers
        headers::trace_id<>,
        headers::span_id<>,
        headers::parent_id<>
    > headers_storage;

    static constexpr size_t size = boost::mpl::size<headers_storage>::type::value;
    typedef std::array<header_t, size> storage_t;

    static
    constexpr
    size_t
    get_size() {
        return size;
    }

    static
    const storage_t&
    get_headers();

    template<class Header>
    constexpr
    static
    size_t
    idx() {
        static_assert(boost::mpl::contains<headers_storage, Header>::type::value, "Could not find header in statis table");
        return boost::mpl::find<headers_storage, Header>::type::pos::value;
    }
};

// Header static and dynamic table as described in http2
// See https://tools.ietf.org/html/draft-ietf-httpbis-header-compression-12#section-2.3
class header_table_t {
public:
    header_table_t();

    const header_t&
    operator[](size_t idx);

    void
    push(const header_t& header);

    inline
    size_t
    find_by_full_match(const header_t& header) {
        return find(std::bind(&header_t::operator==, &header, std::placeholders::_1));
    }

    inline
    size_t
    find_by_name(const header_t& header) {
        return find(std::bind(&header_t::name_equal, &header, std::placeholders::_1));
    }

    size_t
    data_size() const;

    size_t
    size() const;

    size_t
    data_capacity() const;

    bool
    empty() const;

    static constexpr size_t max_data_capacity = 4096;
    static constexpr size_t http2_header_overhead = 32;
    //32 bytes overhead per record and 2 bytes for nil-nil header
    static constexpr size_t max_header_capacity = max_data_capacity / (http2_header_overhead + 2);

private:
    void
    pop();

    size_t
    find(const std::function<bool(const header_t&)> comp);

    // Header storage. Implemented as circular buffer
    std::array<header_t, max_header_capacity> headers;
    size_t header_lower_bound;
    size_t header_upper_bound;

    // Header data storage. Stores all data which headers can reference.
    // Implemented as a sort of circular buffer.
    // We multiply by 2 as data can be padded and we don't want to move it in memory.
    // 2 multiplier guarantee that we can add new value to the end or beginning without data overlap.
    std::array<char, max_data_capacity*2> header_data;
    size_t data_lower_bound;
    size_t data_lower_bound_end;
    size_t data_upper_bound;
    size_t capacity;
};

} // namespace hpack


