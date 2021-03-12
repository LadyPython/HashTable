#include <iostream>
#include <utility>
#include <vector>
#include <iterator>
#include <stdexcept>
#include <algorithm>

template<typename KeyType, typename ValueType, typename Hash = std::hash<KeyType>>
class HashMap {
  private:
    typedef std::pair<KeyType, ValueType> ElementType;
    ElementType END = ElementType();
    typedef std::vector<std::vector<ElementType>> HashMapType;
  public:
    class iterator;
    class const_iterator;

    explicit HashMap(Hash hasher = Hash());
    HashMap(std::initializer_list<ElementType> hash_map, Hash hasher = Hash());
    template<typename Iter>
    HashMap(Iter start, Iter end, Hash hasher = Hash());

    size_t size() const;
    bool empty() const;
    Hash hash_function() const;

    void insert(const ElementType& element);
    void erase(const KeyType& key);

    iterator find(const KeyType& key);
    const_iterator find(const KeyType& key) const;

    ValueType& operator[](const KeyType& key);
    const ValueType& at(const KeyType& key) const;

    void clear();

    iterator begin();
    iterator end();
    const_iterator begin() const;
    const_iterator end() const;

  private:
    HashMapType hash_map = {{}, {END}};
    Hash hasher;
    size_t sz = 0;
    size_t capacity = 1;

    size_t rehash_key(const KeyType& key) const;
    void rehash();
};

template<typename KeyType, typename ValueType, typename Hash>
class HashMap<KeyType, ValueType, Hash>::iterator {
  private:
    typedef typename std::vector<ElementType>::iterator BucketIterator;
    typedef typename HashMapType::iterator HashMapIterator;
  public:
    using iterator_category = std::forward_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = std::pair<const KeyType, ValueType>;
    using pointer = value_type*;
    using reference = value_type&;

    iterator() = default;
    iterator(HashMapIterator _i, BucketIterator _j) : i(_i), j(_j) {
        next_not_empty();
    }
    iterator& operator++() {
        ++j;
        next_not_empty();
        return *this;
    }
    iterator operator++(int) {
        iterator it = *this;
        ++(*this);
        return it;
    }
    bool operator==(const iterator& other) const {
        return i == other.i && j == other.j;
    }
    bool operator!=(const iterator& other) const {
        return !(*this == other);
    }
    reference operator*() {
        return reinterpret_cast<reference>(*j);
    }
    pointer operator->() {
        return reinterpret_cast<pointer>(&(*j));
    }

  private:
    HashMapIterator i;
    BucketIterator j;
    void next_not_empty() {
        while (j == i->end()) {
            ++i;
            j = i->begin();
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
    using pointer = const value_type*;
    using reference = const value_type&;

    const_iterator() = default;
    const_iterator(HashMapConstIterator _i, BucketConstIterator _j) : i(_i), j(_j) {
        next_not_empty();
    }
    const_iterator& operator++() {
        ++j;
        next_not_empty();
        return *this;
    }
    const_iterator operator++(int) {
        const_iterator it = *this;
        ++(*this);
        return it;
    }
    bool operator==(const const_iterator& other) const {
        return i == other.i && j == other.j;
    }
    bool operator!=(const const_iterator& other) const {
        return !(*this == other);
    }
    reference operator*() {
        return reinterpret_cast<reference>(*j);
    }
    pointer operator->(){
        return reinterpret_cast<pointer>(&(*j));
    }

  private:
    HashMapConstIterator i;
    BucketConstIterator j;

    void next_not_empty() {
        while (j == i->end()) {
            ++i;
            j = i->begin();
        }
    }
};

template<typename KeyType, typename ValueType, typename Hash>
HashMap<KeyType, ValueType, Hash>::HashMap(Hash hasher) : hasher(hasher) {
}

template<typename KeyType, typename ValueType, typename Hash>
template<typename Iter>
HashMap<KeyType, ValueType, Hash>::HashMap(Iter start, Iter end, Hash hasher) : hasher(hasher) {
    for (auto it = start; it != end; ++it)
        insert(*it);
}

template<typename KeyType, typename ValueType, typename Hash>
HashMap<KeyType, ValueType, Hash>::HashMap(std::initializer_list<ElementType> hash_map,
                                           Hash hasher) : HashMap(hash_map.begin(), hash_map.end(), hasher) {
}

template<typename KeyType, typename ValueType, typename Hash>
size_t HashMap<KeyType, ValueType, Hash>::size() const {
    return sz;
}

template<typename KeyType, typename ValueType, typename Hash>
bool HashMap<KeyType, ValueType, Hash>::empty() const {
    return sz == 0;
}

template<typename KeyType, typename ValueType, typename Hash>
Hash HashMap<KeyType, ValueType, Hash>::hash_function() const {
    return hasher;
}

template<typename KeyType, typename ValueType, typename Hash>
void HashMap<KeyType, ValueType, Hash>::insert(const ElementType& element) {
    if (find(element.first) != end())
        return;

    hash_map[rehash_key(element.first)].push_back(element);
    ++sz;

    if (sz == capacity)
        rehash();
}

template<typename KeyType, typename ValueType, typename Hash>
void HashMap<KeyType, ValueType, Hash>::erase(const KeyType& key) {
    auto hashed_key = rehash_key(key);
    auto it = std::find_if(hash_map[hashed_key].begin(), hash_map[hashed_key].end(),
                           [key](const ElementType &element) {
                               return element.first == key;
                           });
    if (it == hash_map[hashed_key].end())
        return;

    hash_map[hashed_key].erase(it);
    --sz;

    if (sz * 4 == capacity)
        rehash();
}

template<typename KeyType, typename ValueType, typename Hash>
typename HashMap<KeyType, ValueType, Hash>::iterator HashMap<KeyType, ValueType, Hash>::find(const KeyType &key) {
    auto hashed_key = rehash_key(key);
    auto it = std::find_if(hash_map[hashed_key].begin(), hash_map[hashed_key].end(),
                           [key](const ElementType &element) {
                               return element.first == key;
                           });
    if (it == hash_map[hashed_key].end())
        return end();
    return iterator(hash_map.begin() + hashed_key, it);
}

template<typename KeyType, typename ValueType, typename Hash>
typename HashMap<KeyType, ValueType, Hash>::const_iterator HashMap<KeyType, ValueType, Hash>::find(const KeyType &key) const {
    auto hashed_key = rehash_key(key);
    auto it = std::find_if(hash_map[hashed_key].begin(), hash_map[hashed_key].end(),
                           [key](const ElementType &element) {
                               return element.first == key;
                           });
    if (it == hash_map[hashed_key].end())
        return end();
    return const_iterator(hash_map.begin() + hashed_key, it);
}

template<typename KeyType, typename ValueType, typename Hash>
ValueType& HashMap<KeyType, ValueType, Hash>::operator[](const KeyType& key) {
    insert({key, ValueType()});
    return find(key)->second;
}

template<typename KeyType, typename ValueType, typename Hash>
const ValueType& HashMap<KeyType, ValueType, Hash>::at(const KeyType& key) const {
    auto it = find(key);
    if (it == end())
        throw std::out_of_range("HashMap<KeyType, ValueType, Hash>::at() : key is not exist");
    return it->second;
}

template<typename KeyType, typename ValueType, typename Hash>
void HashMap<KeyType, ValueType, Hash>::clear() {
    hash_map = {{}, {END}};
    sz = 0;
    capacity = 1;
}

template<typename KeyType, typename ValueType, typename Hash>
void HashMap<KeyType, ValueType, Hash>::rehash() {
    auto new_capacity = 2 * sz;
    HashMapType new_hash_map(new_capacity);

    for (auto hashed_key = 0ull; hashed_key < capacity; ++hashed_key) {
        for (const auto &element : hash_map[hashed_key])
            new_hash_map[hasher(element.first) % new_capacity].push_back(element);
    }

    new_hash_map.push_back({END});
    hash_map = new_hash_map;
    capacity = new_capacity;
}

template<typename KeyType, typename ValueType, typename Hash>
size_t HashMap<KeyType, ValueType, Hash>::rehash_key(const KeyType &key) const {
    return hasher(key) % capacity;
}

template<typename KeyType, typename ValueType, typename Hash>
typename HashMap<KeyType, ValueType, Hash>::iterator HashMap<KeyType, ValueType, Hash>::begin()  {
    auto it = std::find_if(hash_map.begin(), hash_map.end(), [](const std::vector<ElementType>& bucket) {
        return !bucket.empty();
    });
    return iterator(it, it->begin());
}

template<typename KeyType, typename ValueType, typename Hash>
typename HashMap<KeyType, ValueType, Hash>::iterator HashMap<KeyType, ValueType, Hash>::end() {
    return iterator(--hash_map.end(), (--hash_map.end())->begin());
}

template<typename KeyType, typename ValueType, typename Hash>
typename HashMap<KeyType, ValueType, Hash>::const_iterator HashMap<KeyType, ValueType, Hash>::begin() const {
    auto it = std::find_if(hash_map.begin(), hash_map.end(), [](const std::vector<ElementType>& bucket) {
        return !bucket.empty();
    });
    return const_iterator(it, it->begin());
}

template<typename KeyType, typename ValueType, typename Hash>
typename HashMap<KeyType, ValueType, Hash>::const_iterator HashMap<KeyType, ValueType, Hash>::end() const {
    return const_iterator(--hash_map.end(), (--hash_map.end())->begin());
}
