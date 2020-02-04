// mnl-misc-dict.hh -- dictionary ADT (with bounded behavior where std::map/set have UB)

/*    Copyright (C) 2018, 2019, 2020 Alexey Protasov (AKA Alex or rusini)

   This file is part of MANOOL.

   MANOOL is free software: you can redistribute it and/or modify it under the terms of the version 3 of the GNU General Public License
   as published by the Free Software Foundation (and only version 3).

   MANOOL is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along with MANOOL.  If not, see <https://www.gnu.org/licenses/>.  */


# ifndef MNL_INCLUDED_DICT
# define MNL_INCLUDED_DICT

# include <cstdlib>     // size_t, ptrdiff_t
# include <type_traits> // remove_const
# include <utility>     // move, pair, make_pair, rel_ops
# include <iterator>    // iterator, reverse_iterator
# include <algorithm>   // equal, lexicographical_compare
# include <functional>  // less
# include "mnl-aux-mnl0.hh"

namespace MNL_AUX_UUID {
   namespace aux {
      using std::size_t; using std::ptrdiff_t; // <cstdlib>
      using std::move; using std::pair; using std::make_pair; // <utility>
   }

namespace aux {
   // order falls back to std::less unless overloaded:
   template<typename Key> inline int order(const Key &lhs, const Key &rhs) noexcept(noexcept(std::less<Key>{}(lhs, rhs)))
      { return std::less<Key>{}(lhs, rhs) ? -1 : std::less<Key>{}(rhs, lhs); }
   // a dict-set is just an instance of dict-map with empty Val - no specific optimizations are attempted for simplicity:
   struct dict_val_empty {};
   inline bool operator==(dict_val_empty, dict_val_empty) noexcept { return true; }
   inline bool operator< (dict_val_empty, dict_val_empty) noexcept { return false; }
   // cannot be nested in dict due to template argument deduction issues (for operator==, etc.):
   template<typename Key, typename Val, typename Ord, typename RetVal> class dict_iterator;
} // namespace aux

namespace aux { namespace pub {
   template<typename Key> struct default_order
      { int operator()(const Key &lhs, const Key &rhs) const noexcept(noexcept(order(lhs, rhs))) { return order(lhs, rhs); } };

   // STL map and set replacement (not drop-in!) with bounded behavior in case of key ordering inconsistencies
   // and three-state comparator, which is more adequate for large keys:
   template<typename Key, typename Val = dict_val_empty, typename Ord = default_order<Key>> class dict/*ionary*/ {
   public: // Typedefs mostly conforming to STL associative containers (most are unused in MANOOL) - no allocator_type
      typedef Key                                            key_type;
      typedef Val                                            mapped_type;
      typedef pair<const key_type, mapped_type>              value_type;
      typedef Ord                                            key_order; // dict-specific - instead of key_compare and value_compare
      typedef value_type                                     &reference;
      typedef const value_type                               &const_reference;
      typedef dict_iterator<Key, Val, Ord, value_type>       iterator;
      typedef dict_iterator<Key, Val, Ord, const value_type> const_iterator;
      typedef size_t                                         size_type;
      typedef ptrdiff_t                                      difference_type;
      typedef value_type                                     *pointer;
      typedef const value_type                               *const_pointer;
      typedef std::reverse_iterator<iterator>                reverse_iterator;
      typedef std::reverse_iterator<const_iterator>          const_reverse_iterator;
   public: // Standard operations
      dict() = default;
   public:
      dict(const dict &rhs): ord(rhs.ord), root(clone(rhs.root)), count(rhs.count), most{find_most<left>(), find_most<right>()} {
         if (root) root->parent = {};
      }
      dict(dict &&rhs): ord((move)(rhs.ord)), root(rhs.root), count(rhs.count), most{rhs.most[left], rhs.most[right]} {
         rhs.root = {}; rhs.count = {}; rhs.most[left] = {}; // left in an indeterminate but completely consistent state, as in STL
      }
      dict &operator=(const dict &rhs) {
         if (&rhs != this) {
            auto root = clone(rhs.root); try { ord = rhs.ord; } catch (...) { dispose(root), throw; }
            (dispose(dict::root), dict::root) = root; count = rhs.count; most[left] = find_most<left>(), most[right] = find_most<right>();
         }
         return *this;
      }
      dict &operator=(dict &&rhs) {
         swap(rhs);
         return *this;
      }
      ~dict() {
         dispose(root);
      }
      void swap(dict &rhs) {
         using std::swap; swap(ord, rhs.ord); swap(root, rhs.root); swap(count, rhs.count); swap(most, rhs.most);
      }
   public: // Dict-specific operations
      explicit dict(Ord ord)
         : ord((move)(ord)) {}
      void set(pair<Key, Val> data)
         { set_root(insert(root, move(data))); most[left] = find_most<left>(), most[right] = find_most<right>(); }
      void set(Key key, Val val)
         { set(make_pair((move)(key), (move)(val))); }
      void set(Key key)
         { set(make_pair((move)(key), Val{})); }
      void unset(const Key &key)
         { set_root(remove(root, key)); if (!most[left]) most[left] = find_most<left>(); else if (!most[right]) most[right] = find_most<right>(); }
   public: // Strictly conforming to STL associative containers
      iterator find(const key_type &);
      const_iterator find(const key_type &) const;
      iterator begin() noexcept, end() noexcept;
      const_iterator begin() const noexcept, end() const noexcept;
      reverse_iterator rbegin() noexcept, rend() noexcept;
      const_reverse_iterator rbegin() const noexcept, rend() const noexcept;
      const_iterator cbegin() const noexcept, cend() const noexcept;
      const_reverse_iterator crbegin() const noexcept, crend() const noexcept;
   public: // Mostly conforming to STL associative containers
      Ord key_ord() const { return ord; }
      Val &operator[](const Key &key) { return find(key)->second; }
      const Val &operator[](const Key & key) const { return find(key)->second; }
   private: // Concrete representation
      Ord ord = {};
      struct node;
      node *root = {};
      size_type count = {};
      node *most[2]{};
      static const auto left = false, right = true;
      friend iterator;
      friend const_iterator;
   private: // AVL tree helpers
      static int max(int, int);
      static int height(const node *);
      static void update_height(node *);
      static int balance(const node *);
      void set_root(node *);
      template<bool> static void set_child(node *, node *);
      template<bool> static node *rotate(node *);
      template<bool> static node *rotate_twice(node *);
      template<bool> static node *rebalance(node *);
      static node *rebalance(node *);
      node *insert(node *, pair<Key, Val> &&data);
      static node *remove_min(node *);
      node *remove(node *, const Key &);
      node *find(node *, const Key &) const;
      static node *clone(node *);
      static void dispose(node *) noexcept;
      template<bool> node *find_most();
   public: // Strictly conforming to STL containers
      bool empty() const noexcept { return !size(); }
      size_type size() const noexcept { return count; }
      size_type max_size() const noexcept { return size_type(-1) / sizeof(node); }
      void clear() noexcept { (dispose(root), root) = {}; count = {}; most[left] = {}; }
   };
   template<typename Key, typename Val, typename Ord> inline void swap(dict<Key, Val, Ord> &lhs, dict<Key, Val, Ord> &rhs)
      { lhs.swap(rhs); }
   template<typename Key, typename Val, typename Ord> inline bool operator==(const dict<Key, Val, Ord> &lhs, const dict<Key, Val, Ord> &rhs)
      { return lhs.size() == rhs.size() && std::equal(lhs.begin(), lhs.end(), rhs.begin()); }
   template<typename Key, typename Val, typename Ord> inline bool operator< (const dict<Key, Val, Ord> &lhs, const dict<Key, Val, Ord> &rhs)
      { return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end()); }
   template<typename Key, typename Val, typename Ord> inline bool operator!=(const dict<Key, Val, Ord> &lhs, const dict<Key, Val, Ord> &rhs)
      { return std::rel_ops::operator!=(lhs, rhs); }
   template<typename Key, typename Val, typename Ord> inline bool operator> (const dict<Key, Val, Ord> &lhs, const dict<Key, Val, Ord> &rhs)
      { return std::rel_ops::operator> (lhs, rhs); }
   template<typename Key, typename Val, typename Ord> inline bool operator<=(const dict<Key, Val, Ord> &lhs, const dict<Key, Val, Ord> &rhs)
      { return std::rel_ops::operator<=(lhs, rhs); }
   template<typename Key, typename Val, typename Ord> inline bool operator>=(const dict<Key, Val, Ord> &lhs, const dict<Key, Val, Ord> &rhs)
      { return std::rel_ops::operator>=(lhs, rhs); }

   template<typename Key, typename Val, typename Ord> struct dict<Key, Val, Ord>::node {
      pair<const Key, Val> data;
      node                 *child[2];
      int                  height;
      node                 *parent;
   };
   template<typename Key, typename Val, typename Ord> inline int dict<Key, Val, Ord>::max(int lhs, int rhs) {
      return lhs >= rhs ? lhs : rhs;
   }
   template<typename Key, typename Val, typename Ord> inline int dict<Key, Val, Ord>::height(const node *root) {
      return root ? root->height : 0;
   }
   template<typename Key, typename Val, typename Ord> inline void dict<Key, Val, Ord>::update_height(node *root) {
      root->height = max(height(root->child[left]), height(root->child[right])) + 1;
   }
   template<typename Key, typename Val, typename Ord> inline int dict<Key, Val, Ord>::balance(const node *root) {
      return height(root->child[right]) - height(root->child[left]);
   }
   template<typename Key, typename Val, typename Ord> inline void dict<Key, Val, Ord>::set_root(node *root) {
      if (this->root = root) root->parent = {};
   }
   template<typename Key, typename Val, typename Ord> template<bool dir> inline void dict<Key, Val, Ord>::set_child(node *root, node *child) {
      if (root->child[dir] = child) child->parent = root;
   }
   template<typename Key, typename Val, typename Ord> template<bool dir> inline typename dict<Key, Val, Ord>::node *dict<Key, Val, Ord>::rotate(node *root) {
      auto res = root->child[!dir];
      set_child<!dir>(root, res->child[dir]), set_child<dir>(res, root);
      update_height(root), update_height(res);
      return res;
   }
   template<typename Key, typename Val, typename Ord> template<bool dir> inline typename dict<Key, Val, Ord>::node *dict<Key, Val, Ord>::rotate_twice(node *root) {
      set_child<!dir>(root, rotate<!dir>(root->child[!dir]));
      return rotate<dir>(root);
   }
   template<typename Key, typename Val, typename Ord> template<bool dir> inline typename dict<Key, Val, Ord>::node *dict<Key, Val, Ord>::rebalance(node *root) {
      return (height(root->child[!dir]->child[!dir]) < height(root->child[!dir]->child[dir]) ? rotate_twice<dir> : rotate<dir>)(root);
   }
   template<typename Key, typename Val, typename Ord> inline typename dict<Key, Val, Ord>::node *dict<Key, Val, Ord>::rebalance(node *root) {
      update_height(root);
      return balance(root) == 2 ? rebalance<left>(root) : balance(root) == -2 ? rebalance<right>(root) : root;
   }
   template<typename Key, typename Val, typename Ord> typename dict<Key, Val, Ord>::node *dict<Key, Val, Ord>::insert(node *root, pair<Key, Val> &&data) {
      if (!root) return ++count, new node{move(data), {}, 1};
      auto ord = dict::ord(data.first, root->data.first);
      if (ord < 0) return set_child<left> (root, insert(root->child[left],  move(data))), rebalance(root);
      if (ord > 0) return set_child<right>(root, insert(root->child[right], move(data))), rebalance(root);
      return root->data.second = (move)(data.second), root;
   }
   template<typename Key, typename Val, typename Ord> typename dict<Key, Val, Ord>::node *dict<Key, Val, Ord>::remove_min(node *root) {
      return root->child[left] ? (set_child<left>(root, remove_min(root->child[left])), rebalance(root)) : root->child[right];
   }
   template<typename Key, typename Val, typename Ord> typename dict<Key, Val, Ord>::node *dict<Key, Val, Ord>::remove(node *root, const Key &key) {
      if (!root) return {};
      auto ord = dict::ord(key, root->data.first);
      if (ord < 0) return set_child<left> (root, remove(root->child[left],  key)), rebalance(root);
      if (ord > 0) return set_child<right>(root, remove(root->child[right], key)), rebalance(root);
      --count; if (root == most[left]) most[left] = {}; else if (root == most[right]) most[right] = {}; // most[right] may be dangling and it's OK
      node *child[]{root->child[left], root->child[right]};
      delete root;
      if (!child[right]) return child[left];
      auto min = child[right];
      while (min->child[left]) min = min->child[left];
      return set_child<right>(min, remove_min(child[right])), set_child<left>(min, child[left]), rebalance(min);
   }
   template<typename Key, typename Val, typename Ord> typename dict<Key, Val, Ord>::node *dict<Key, Val, Ord>::find(node *root, const Key &key) const {
      if (!root) return {};
      auto ord = dict::ord(key, root->data.first);
      return ord < 0 ? find(root->child[left], key) : ord > 0 ? find(root->child[right], key) : root;
   }
   template<typename Key, typename Val, typename Ord> typename dict<Key, Val, Ord>::node *dict<Key, Val, Ord>::clone(node *root) {
      if (!root) return {};
      auto res = new node{root->data, {}, root->height};
      try { set_child<left>(res, clone(root->child[left])), set_child<right>(res, clone(root->child[right])); } catch (...) { dispose(res), throw; }
      return res;
   }
   template<typename Key, typename Val, typename Ord> void dict<Key, Val, Ord>::dispose(node *root) noexcept {
      if (root) dispose(root->child[left]), dispose(root->child[right]), delete root;
   }
   template<typename Key, typename Val, typename Ord> template<bool dir> inline typename dict<Key, Val, Ord>::node *dict<Key, Val, Ord>::find_most() {
      if (!root) return {};
      auto res = root;
      while (res->child[dir]) res = res->child[dir];
      return res;
   }
}} // namespace aux::pub

// Iterators ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace aux {
   template<typename Key, typename Val, typename Ord, typename IterVal>
   class dict_iterator: public std::iterator<std::bidirectional_iterator_tag, IterVal> { // Strictly conforming to STL bidirectional iterators
   public: // Standard operations
      dict_iterator() = default;
      template<typename Key_, typename Val_, typename Ord_, typename LRetVal_, typename RRetVal_> friend bool
         operator==(dict_iterator<Key_, Val_, Ord_, LRetVal_>, dict_iterator<Key_, Val_, Ord_, RRetVal_>) noexcept;
      template<typename Key_, typename Val_, typename Ord_, typename LRetVal_, typename RRetVal_> friend bool
         operator!=(dict_iterator<Key_, Val_, Ord_, LRetVal_>, dict_iterator<Key_, Val_, Ord_, RRetVal_>) noexcept;
   public: // Conforming to STL
      IterVal &operator*() const noexcept { return node->data; }
      IterVal *operator->() const noexcept { return &*(*this); }
      dict_iterator &operator++() noexcept { goto_next<dict<Key, Val, Ord>::right>(); return *this; }
      dict_iterator &operator--() noexcept { if (node) goto_next<dict<Key, Val, Ord>::left>(); else node = owner->most[dict<Key, Val, Ord>::right]; return *this; }
      dict_iterator operator++(int) noexcept { auto res = *this; ++(*this); return res; }
      dict_iterator operator--(int) noexcept { auto res = *this; --(*this); return res; }
      operator dict_iterator<Key, Val, Ord, const IterVal>() const noexcept { return {owner, node}; }
   private: // Concrete representation
      typename dict<Key, Val, Ord>::node *node;
      const dict<Key, Val, Ord> *owner;
      dict_iterator(decltype(owner) owner, decltype(node) node) noexcept: node(node), owner(owner) {}
      friend class dict<Key, Val, Ord>;
      friend class dict_iterator<Key, Val, Ord, typename std::remove_const<IterVal>::type>;
   private: // Implementation helpers
      template<bool dir> void goto_next() noexcept {
         if (node->child[dir]) for (node = node->child[dir]; node->child[!dir]; node = node->child[!dir]);
         else { decltype(node) prev; do prev = node, node = node->parent; while (node && node->child[dir] == prev); }
      }
   };
   template<typename Key, typename Val, typename Ord, typename LRetVal, typename RRetVal> inline bool
      operator==(dict_iterator<Key, Val, Ord, LRetVal> lhs, dict_iterator<Key, Val, Ord, RRetVal> rhs) noexcept { return lhs.node == rhs.node; }
   template<typename Key, typename Val, typename Ord, typename LRetVal, typename RRetVal> inline bool
      operator!=(dict_iterator<Key, Val, Ord, LRetVal> lhs, dict_iterator<Key, Val, Ord, RRetVal> rhs) noexcept { return lhs.node != rhs.node; }

   template<typename Key, typename Val, typename Ord> inline typename dict<Key, Val, Ord>::iterator
      dict<Key, Val, Ord>::find(const key_type &key)       { return {this, find(root, key)}; }
   template<typename Key, typename Val, typename Ord> inline typename dict<Key, Val, Ord>::const_iterator
      dict<Key, Val, Ord>::find(const key_type &key) const { return {this, find(root, key)}; }

   template<typename Key, typename Val, typename Ord> inline typename dict<Key, Val, Ord>::iterator
      dict<Key, Val, Ord>::begin() noexcept { return {this, most[left]}; }
   template<typename Key, typename Val, typename Ord> inline typename dict<Key, Val, Ord>::iterator
      dict<Key, Val, Ord>::end()   noexcept { return {this, {}}; }

   template<typename Key, typename Val, typename Ord> inline typename dict<Key, Val, Ord>::const_iterator
      dict<Key, Val, Ord>::begin() const noexcept { return {this, most[left]}; }
   template<typename Key, typename Val, typename Ord> inline typename dict<Key, Val, Ord>::const_iterator
      dict<Key, Val, Ord>::end()   const noexcept { return {this, {}}; }

   template<typename Key, typename Val, typename Ord> inline typename dict<Key, Val, Ord>::reverse_iterator
      dict<Key, Val, Ord>::rbegin() noexcept { return reverse_iterator{end()}; }
   template<typename Key, typename Val, typename Ord> inline typename dict<Key, Val, Ord>::reverse_iterator
      dict<Key, Val, Ord>::rend()   noexcept { return reverse_iterator{begin()}; }

   template<typename Key, typename Val, typename Ord> inline typename dict<Key, Val, Ord>::const_reverse_iterator
      dict<Key, Val, Ord>::rbegin() const noexcept { return const_reverse_iterator{end()}; }
   template<typename Key, typename Val, typename Ord> inline typename dict<Key, Val, Ord>::const_reverse_iterator
      dict<Key, Val, Ord>::rend()   const noexcept { return const_reverse_iterator{begin()}; }

   template<typename Key, typename Val, typename Ord> inline typename dict<Key, Val, Ord>::const_iterator
      dict<Key, Val, Ord>::cbegin() const noexcept { return begin(); }
   template<typename Key, typename Val, typename Ord> inline typename dict<Key, Val, Ord>::const_iterator
      dict<Key, Val, Ord>::cend()   const noexcept { return end(); }

   template<typename Key, typename Val, typename Ord> inline typename dict<Key, Val, Ord>::const_reverse_iterator
      dict<Key, Val, Ord>::crbegin() const noexcept { return rbegin(); }
   template<typename Key, typename Val, typename Ord> inline typename dict<Key, Val, Ord>::const_reverse_iterator
      dict<Key, Val, Ord>::crend()   const noexcept { return rend(); }
} // namespace aux

} // namespace MNL_AUX_UUID

# endif // # ifndef MNL_INCLUDED_DICT
