#include <iostream>
#include <utility>
#include <vector>
#include <iterator>
#include <stdexcept>
#include <algorithm>

// Hash Table with Separate Chaining using std::vector<std::vector<>>.
// Dynamic rehashing with doubling and halving of the table size.
// https://en.wikipedia.org/wiki/Hash_table#Separate_chaining
template<typename KeyType, typename ValueType, typename Hash = std::hash<KeyType>>
class HashMap {
  private:
    typedef std::pair<KeyType, ValueType> ElementType;
    ElementType end_ = ElementType();
    typedef std::vector<std::vector<ElementType>> HashMapType;
  public:
    static constexpr size_t INCREASING_SIZE_COEFFICIENT = 1;
    static constexpr size_t CHANGING_SIZE_COEFFICIENT = 2;
    static constexpr size_t DECREASING_SIZE_COEFFICIENT = 4;

    class iterator;
    class const_iterator;

    // The default constructor (with hasher).
    explicit HashMap(Hash hasher = Hash());
    // The constructor by begin-end iterators of std::pair<key, value> sequence (with hasher).
    HashMap(std::initializer_list<ElementType> hash_map, Hash hasher = Hash());
    // The constructor by a std::initializer_list of std::pair<key, value> (with hasher).
    template<typename Iter>
    HashMap(Iter start, Iter end, Hash hasher = Hash());

    // O(1)
    size_t size() const;
    // O(1)
    bool empty() const;
    // Method returns hasher by value. O(1)
    Hash hash_function() const;

    // Method inserts the element.
    // If such key already exists, method does nothing.
    // Amortized O(1).
    void insert(const ElementType &element);
    // Method erases the element with specific key.
    // If such key doesn't exist, method does nothing.
    // Amortized O(1).
    void erase(const KeyType &key);

    // Methods return (const_)iterator on the element with specific key. Amortized O(1).
    iterator find(const KeyType &key);
    const_iterator find(const KeyType &key) const;

    // Method returns value by key.
    // If such key doesn't exist, insert default value by the key. Amortized O(1).
    ValueType &operator[](const KeyType &key);
    // Method returns value by key.
    // If such key doesn't exist, throw std::out_of_range. Amortized O(1).
    const ValueType &at(const KeyType &key) const;

    // O(1).
    void clear();

    // Method returns an (const_)iterator to the beginning. O(1)
    iterator begin();
    const_iterator begin() const;

    // Method returns an (const_)iterator to the end. O(1)
    iterator end();
    const_iterator end() const;

  private:
    HashMapType hash_map_ = {{}, {end_}};
    Hash hasher_;
    size_t sz_ = 0;
    size_t capacity_ = 1;

    size_t hash_key(const KeyType &key) const;
    void rehash_if_necessary();
};

// Hash Table (const_)iterator with 2 fields: iterator to element into bucket and iterator to bucket.
template<typename KeyType, typename ValueType, typename Hash>
class HashMap<KeyType, ValueType, Hash>::iterator {
  private:
    typedef typename std::vector<ElementType>::iterator BucketIterator;
    typedef typename HashMapType::iterator HashMapIterator;
  public:
    using iterator_category = std::forward_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = std::pair<const KeyType, ValueType>;
    using pointer = value_type *;
    using reference = value_type &;

    // The default constructor.
    iterator() = default;
    // The constructor by two iterators.
    iterator(HashMapIterator i, BucketIterator j) : i_(i), j_(j) {
        next_not_empty();
    }

    // Different forward_iterator operators are supported. Amortized O(1).
    iterator &operator++() {
        ++j_;
        next_not_empty();
        return *this;
    }
    iterator operator++(int) {
        iterator it = *this;
        ++(*this);
        return it;
    }
    bool operator==(const iterator &other) const {
        return i_ == other.i_ && j_ == other.j_;
    }
    bool operator!=(const iterator &other) const {
        return !(*this == other);
    }
    reference operator*() {
        return reinterpret_cast<reference>(*j_);
    }
    pointer operator->() {
        return reinterpret_cast<pointer>(&(*j_));
    }

  private:
    HashMapIterator i_;
    BucketIterator j_;
    void next_not_empty() {
        while (j_ == i_->end()) {
            ++i_;
            j_ = i_->begin();
        }
    }
};

template<typename KeyType, typename ValueType, typename Hash>
class HashMap<KeyType, ValueType, Hash>::const_iterator {
  private:
    typedef typename std::vector<ElementType>::const_iterator BucketConstIterator;
    typedef typename HashMapType::const_iterator HashMapConstIterator;
  public:
    using iterator_category = std::forward_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = const std::pair<const KeyType, ValueType>;
    using pointer = const value_type *;
    using reference = const value_type &;

    const_iterator() = default;
    const_iterator(HashMapConstIterator i, BucketConstIterator j) : i_(i), j_(j) {
        next_not_empty();
    }

    const_iterator &operator++() {
        ++j_;
        next_not_empty();
        return *this;
    }
    const_iterator operator++(int) {
        const_iterator it = *this;
        ++(*this);
        return it;
    }
    bool operator==(const const_iterator &other) const {
        return i_ == other.i_ && j_ == other.j_;
    }
    bool operator!=(const const_iterator &other) const {
        return !(*this == other);
    }
    reference operator*() {
        return reinterpret_cast<reference>(*j_);
    }
    pointer operator->() {
        return reinterpret_cast<pointer>(&(*j_));
    }

  private:
    HashMapConstIterator i_;
    BucketConstIterator j_;

    void next_not_empty() {
        while (j_ == i_->end()) {
            ++i_;
            j_ = i_->begin();
        }
    }
};

template<typename KeyType, typename ValueType, typename Hash>
HashMap<KeyType, ValueType, Hash>::HashMap(Hash hasher) : hasher_(hasher) {
}

template<typename KeyType, typename ValueType, typename Hash>
template<typename Iter>
HashMap<KeyType, ValueType, Hash>::HashMap(Iter start, Iter end, Hash hasher) : hasher_(hasher) {
    for (auto it = start; it != end; ++it) {
        insert(*it);
    }
}

template<typename KeyType, typename ValueType, typename Hash>
HashMap<KeyType, ValueType, Hash>::HashMap(std::initializer_list<ElementType> hash_map,
                                           Hash hasher) : HashMap(hash_map.begin(), hash_map.end(), hasher) {
}

template<typename KeyType, typename ValueType, typename Hash>
size_t HashMap<KeyType, ValueType, Hash>::size() const {
    return sz_;
}

template<typename KeyType, typename ValueType, typename Hash>
bool HashMap<KeyType, ValueType, Hash>::empty() const {
    return sz_ == 0;
}

template<typename KeyType, typename ValueType, typename Hash>
Hash HashMap<KeyType, ValueType, Hash>::hash_function() const {
    return hasher_;
}

template<typename KeyType, typename ValueType, typename Hash>
void HashMap<KeyType, ValueType, Hash>::insert(const ElementType &element) {
    if (find(element.first) != end()) {
        return;
    }

    hash_map_[hash_key(element.first)].push_back(element);
    ++sz_;

    rehash_if_necessary();
}

template<typename KeyType, typename ValueType, typename Hash>
void HashMap<KeyType, ValueType, Hash>::erase(const KeyType &key) {
    size_t hashed_key = hash_key(key);
    auto it = std::find_if(hash_map_[hashed_key].begin(), hash_map_[hashed_key].end(),
                           [key](const ElementType &element) {
                               return element.first == key;
                           });
    if (it == hash_map_[hashed_key].end()) {
        return;
    }

    hash_map_[hashed_key].erase(it);
    --sz_;

    rehash_if_necessary();
}

template<typename KeyType, typename ValueType, typename Hash>
typename HashMap<KeyType, ValueType, Hash>::iterator HashMap<KeyType, ValueType, Hash>::find(const KeyType &key) {
    size_t hashed_key = hash_key(key);
    auto it = std::find_if(hash_map_[hashed_key].begin(), hash_map_[hashed_key].end(),
                           [key](const ElementType &element) {
                               return element.first == key;
                           });
    if (it == hash_map_[hashed_key].end()) {
        return end();
    }
    return iterator(hash_map_.begin() + hashed_key, it);
}

template<typename KeyType, typename ValueType, typename Hash>
typename HashMap<KeyType, ValueType, Hash>::const_iterator HashMap<KeyType,
                                                                   ValueType,
                                                                   Hash>::find(const KeyType &key) const {
    size_t hashed_key = hash_key(key);
    auto it = std::find_if(hash_map_[hashed_key].begin(), hash_map_[hashed_key].end(),
                           [key](const ElementType &element) {
                               return element.first == key;
                           });
    if (it == hash_map_[hashed_key].end()) {
        return end();
    }
    return const_iterator(hash_map_.begin() + hashed_key, it);
}

template<typename KeyType, typename ValueType, typename Hash>
ValueType &HashMap<KeyType, ValueType, Hash>::operator[](const KeyType &key) {
    insert({key, ValueType()});
    return find(key)->second;
}

template<typename KeyType, typename ValueType, typename Hash>
const ValueType &HashMap<KeyType, ValueType, Hash>::at(const KeyType &key) const {
    auto it = find(key);
    if (it == end()) {
        throw std::out_of_range("HashMap<KeyType, ValueType, Hash>::at() : key is not exist");
    }
    return it->second;
}

template<typename KeyType, typename ValueType, typename Hash>
void HashMap<KeyType, ValueType, Hash>::clear() {
    hash_map_ = {{}, {end_}};
    sz_ = 0;
    capacity_ = 1;
}

template<typename KeyType, typename ValueType, typename Hash>
void HashMap<KeyType, ValueType, Hash>::rehash_if_necessary() {
    if (sz_ * INCREASING_SIZE_COEFFICIENT != capacity_ && sz_ * DECREASING_SIZE_COEFFICIENT != capacity_) {
        return;
    }

    size_t new_capacity = sz_ * CHANGING_SIZE_COEFFICIENT;
    HashMapType new_hash_map(new_capacity);

    for (size_t hashed_key = 0; hashed_key < capacity_; ++hashed_key) {
        for (const ElementType &element : hash_map_[hashed_key]) {
            new_hash_map[hasher_(element.first) % new_capacity].push_back(element);
        }
    }

    new_hash_map.push_back({end_});
    hash_map_ = new_hash_map;
    capacity_ = new_capacity;
}

template<typename KeyType, typename ValueType, typename Hash>
size_t HashMap<KeyType, ValueType, Hash>::hash_key(const KeyType &key) const {
    return hasher_(key) % capacity_;
}

template<typename KeyType, typename ValueType, typename Hash>
typename HashMap<KeyType, ValueType, Hash>::iterator HashMap<KeyType, ValueType, Hash>::begin() {
    auto it = std::find_if(hash_map_.begin(), hash_map_.end(), [](const std::vector<ElementType> &bucket) {
        return !bucket.empty();
    });
    return iterator(it, it->begin());
}

template<typename KeyType, typename ValueType, typename Hash>
typename HashMap<KeyType, ValueType, Hash>::iterator HashMap<KeyType, ValueType, Hash>::end() {
    return iterator(--hash_map_.end(), (--hash_map_.end())->begin());
}

template<typename KeyType, typename ValueType, typename Hash>
typename HashMap<KeyType, ValueType, Hash>::const_iterator HashMap<KeyType, ValueType, Hash>::begin() const {
    auto it = std::find_if(hash_map_.begin(), hash_map_.end(), [](const std::vector<ElementType> &bucket) {
        return !bucket.empty();
    });
    return const_iterator(it, it->begin());
}

template<typename KeyType, typename ValueType, typename Hash>
typename HashMap<KeyType, ValueType, Hash>::const_iterator HashMap<KeyType, ValueType, Hash>::end() const {
    return const_iterator(--hash_map_.end(), (--hash_map_.end())->begin());
}
