#ifndef BPTREE_HPP_BPTREE2_HPP
#define BPTREE_HPP_BPTREE2_HPP

#include <string>
#include "vector.hpp"
#include "MemoryRiver.hpp"

template<class Key, class T, int M = 100, int L = 100>
class BPTree {
private:
    struct val_vec {
        int now_size, next;
        T val[400] = {0};

        val_vec() : now_size(0), next(0) {};
        val_vec(const T &ind) : next(0) {
            now_size = 1;
            val[0] = ind;
        }

        int lower(const T &ind) {
            if (ind > val[now_size - 1]) return now_size;
            int l = 0, r = now_size - 1, mid;
            while (l < r) {
                mid = (l + r) >> 1;
                if (val[mid] < ind) l = mid + 1;
                else r = mid;
            }
            return l;
        }

        void insert(const T &ind) {
            int pos = lower(ind);
            for (int i = now_size; i > pos; --i) val[i] = val[i - 1];
            val[pos] = ind;
            ++now_size;
        }
        int erase(const T &ind) {
            int pos = lower(ind);
            if (pos == now_size) return -1;
            if (val[pos] != ind) return -2;
            --now_size;
            for (int i = pos; i < now_size; ++i) val[i] = val[i + 1];
            return 0;
        }
    };

    struct BPT_node {
        bool is_leaf;
        int son[std::max(M, L) + 1] = {0};
        int now_size, next;
        Key key[std::max(M, L) + 1];

        BPT_node() : is_leaf(false), now_size(0), next(0) {};
    };

    int root;
    int _size;
    MemoryRiver<BPT_node, 4> node_river;
    MemoryRiver<val_vec> val_river;

    int upper_key(const Key &ind, const BPT_node &rt) {
        if (ind >= rt.key[rt.now_size - 1]) return rt.now_size;
        int l = 0, r = rt.now_size - 1, mid;
        while (l < r) {
            mid = (l + r) >> 1;
            if (rt.key[mid] <= ind) l = mid + 1;
            else r = mid;
        }
        return l;
    }
    int lower_key(const Key &ind, const BPT_node &rt) {
        if (ind > rt.key[rt.now_size - 1]) return rt.now_size;
        int l = 0, r = rt.now_size - 1, mid;
        while (l < r) {
            mid = (l + r) >> 1;
            if (rt.key[mid] < ind) l = mid + 1;
            else r = mid;
        }
        return l;
    }

public:
    explicit BPTree(const std::string &name) : _size(0), node_river(name + "_node"), val_river(name + "_val") {
        std::fstream file;
        file.open(name + "_node", std::ios::in);
        if (!file) {
            node_river.initialise();
            val_river.initialise();
        } else {
            node_river.get_info(_size, 4);
            node_river.get_info(root, 3);
            file.close();
        }
    }

    ~BPTree() {
        node_river.write_info(_size, 4);
        node_river.write_info(root, 3);
    };

    int size() { return _size; }

    void insert(const std::pair<Key, T> &val) {
        if (_size == 0) {
            BPT_node new_node;
            new_node.is_leaf = true;
            new_node.now_size = 1;
            val_vec new_val(val.second);
            new_node.key[0] = val.first;
            new_node.son[0] = val_river.write(new_val);
            root = node_river.write(new_node);
            _size++;
            return;
        }
        std::pair<int, Key> ret = insert(val.first, val.second, root);
        if (ret.first) {
            BPT_node new_node;
            new_node.is_leaf = false;
            new_node.now_size = 1;
            new_node.key[0] = ret.second;
            new_node.son[0] = root;
            new_node.son[1] = ret.first;
            new_node.next = 0;
            root = node_river.write(new_node);
        }
    }
    void val_insert(BPT_node &rt, int pos, const T &val) {
        val_vec now_val;
        val_river.read(now_val, rt.son[pos]);
        if (now_val.val[now_val.now_size - 1] > val) {
            if (now_val.now_size < 400) {
                now_val.insert(val);
                val_river.update(now_val, rt.son[pos]);
            } else {
                val_vec new_val;
                for (int i = 200; i < 400; ++i) new_val.val[i - 200] = now_val.val[i];
                new_val.now_size = 200;
                now_val.now_size = 200;
                if (val > now_val.val[199]) new_val.insert(val);
                else now_val.insert(val);
                new_val.next = now_val.next;
                now_val.next = val_river.write(new_val);
                val_river.update(now_val, rt.son[pos]);
            }
            return;
        }
        int now_pos = rt.son[pos];
        while (now_val.next) {
            now_pos = now_val.next;
            val_river.read(now_val, now_pos);
            if (now_val.val[now_val.now_size - 1] > val) {
                if (now_val.now_size < 400) {
                    now_val.insert(val);
                    val_river.update(now_val, now_pos);
                } else {
                    val_vec new_val;
                    for (int i = 200; i < 400; ++i) new_val.val[i - 200] = now_val.val[i];
                    new_val.now_size = 200;
                    now_val.now_size = 200;
                    if (val > now_val.val[199]) new_val.insert(val);
                    else now_val.insert(val);
                    new_val.next = now_val.next;
                    now_val.next = val_river.write(new_val);
                    val_river.update(now_val, now_pos);
                }
                return;
            }
        }
        if (now_val.now_size < 400) {
            now_val.val[now_val.now_size++] = val;
            val_river.update(now_val, now_pos);
            return;
        }
        val_vec new_val(val);
        now_val.next = val_river.write(new_val);
        val_river.update(now_val, now_pos);
    }
    std::pair<int, Key> insert(const Key &ind, const T &val, int &pos) {
        BPT_node rt;
        node_river.read(rt, pos);
        std::pair<int, Key> ret(0, ind);
        int i;
        if (rt.is_leaf) {
            i = lower_key(ind, rt);
            if (i < rt.now_size && ind == rt.key[i]) val_insert(rt, i, val);
            else {
                for (int j = rt.now_size; j > i; j--) {
                    rt.key[j] = rt.key[j - 1];
                    rt.son[j] = rt.son[j - 1];
                }
                rt.key[i] = ind;
                val_vec new_val(val);
                rt.son[i] = val_river.write(new_val);
                rt.now_size++;
                _size++;
                if (rt.now_size > M) {
                    BPT_node new_node;
                    int mid = rt.now_size >> 1;
                    for (int j = mid; j <= M; j++) {
                        new_node.key[j - mid] = rt.key[j];
                        new_node.son[j - mid] = rt.son[j];
                    }
                    new_node.is_leaf = true;
                    new_node.now_size = M - mid + 1;
                    rt.now_size = mid;
                    new_node.next = rt.next;
                    ret.first = node_river.write(new_node);
                    rt.next = ret.first;
                    ret.second = new_node.key[0];
                }
            }
            node_river.update(rt, pos);
            return ret;
        }
        i = upper_key(ind, rt);
        std::pair<int, Key> new_pos = insert(ind, val, rt.son[i]);
        if (new_pos.first == 0) return ret;
        for (int j = rt.now_size; j > i; j--) {
            rt.key[j] = rt.key[j - 1];
            rt.son[j + 1] = rt.son[j];
        }
        rt.son[i + 1] = new_pos.first;
        rt.key[i] = new_pos.second;
        rt.now_size++;
        if (rt.now_size == M) {
            BPT_node new_node;
            int mid = (rt.now_size + 1) >> 1; //2
            //前2后2
            for (int j = mid; j < M; j++) {
                new_node.key[j - mid] = rt.key[j];
                new_node.son[j - mid] = rt.son[j];
            }
            new_node.son[M - mid] = rt.son[M];
            new_node.now_size = M - mid;
            rt.now_size = mid - 1;
            ret.first = rt.next = node_river.write(new_node);
            BPT_node tmp;
            node_river.read(tmp, new_node.son[0]);
            ret.second = tmp.key[0];
        }
        node_river.update(rt, pos);
        return ret;
    }
    
    std::pair<bool, T> find(const Key &key) {
        if (_size == 0) return std::make_pair(false, T());
        BPT_node rt;
        node_river.read(rt, root);
        while (!rt.is_leaf) {
            int i = lower_key(key, rt);
            if (i == rt.now_size || rt.key[i] != key) return std::make_pair(false, T());
            node_river.read(rt, rt.son[i]);
        }
        int i = lower_key(key, rt);
        if (i == rt.now_size || rt.key[i] != key) return std::make_pair(false, T());
        int now = rt.son[i];
        val_vec search;
        val_river.read(search, now);
        return std::make_pair(true, search.val[0]);
    }
    sjtu::vector<T> Find(const Key &key) {
        sjtu::vector<T> ret;
        if (_size == 0) return ret;
        int pos = root;
        while (true) {
            BPT_node rt;
            node_river.read(rt, pos);
            if (rt.is_leaf) {
                int i = lower_key(key, rt);
                if (i < rt.now_size && rt.key[i] == key) {
                    int now = rt.son[i];
                    while (now) {
                        val_vec get;
                        val_river.read(get, now);
                        for (int j = 0; j < get.now_size; j++) ret.push_back(get.val[j]);
                        now = get.next;
                    }
                }
                return ret;
            }
            int i = upper_key(key, rt);
            pos = rt.son[i];
        }
    }

    void delete_node(const Key &ind, const T &val, BPT_node &fa, int &pos) {
        int i = upper_key(ind, fa);
        BPT_node i_son;
        node_river.read(i_son, fa.son[i]);
        if (i_son.is_leaf) {
            int j = lower_key(ind, i_son);
            if (j == i_son.now_size || i_son.key[j] != ind) return;
            int now = i_son.son[j], pre_pos;
            val_vec search, pre_val;
            while (now) {
                val_river.read(search, now);
                int ret = search.erase(val);
                if (ret == 0) {
                    if (search.now_size == 0) {
                        val_river.Delete(now);
                        if (now == i_son.son[j]) {
                            i_son.son[j] = search.next;
                            if (!search.next) {
                                i_son.now_size--;
                                _size--;
                                if (i_son.now_size < (L + 1) >> 1) {
                                    BPT_node next_node, pre_node;
                                    if (i != fa.now_size) node_river.read(next_node, fa.son[i + 1]);
                                    if (i != 0) node_river.read(pre_node, fa.son[i - 1]);
                                    if (i != fa.now_size && next_node.now_size > ((L + 1) >> 1)) {
                                        for (int jj = j; jj < i_son.now_size; jj++) {
                                            i_son.key[jj] = i_son.key[jj + 1];
                                            i_son.son[jj] = i_son.son[jj + 1];
                                        }
                                        i_son.key[i_son.now_size] = next_node.key[0];
                                        i_son.son[i_son.now_size] = next_node.son[0];
                                        next_node.now_size--;
                                        for (int jj = 0; jj < next_node.now_size; jj++) {
                                            next_node.key[jj] = next_node.key[jj + 1];
                                            next_node.son[jj] = next_node.son[jj + 1];
                                        }
                                        i_son.now_size++;
                                        fa.key[i] = next_node.key[0];
                                        node_river.update(next_node, fa.son[i + 1]);
                                        node_river.update(i_son, fa.son[i]);
                                    } else if (i != 0 && pre_node.now_size > ((L + 1) >> 1)) {
                                        for (int jj = j; jj > 0; jj--) {
                                            i_son.key[jj] = i_son.key[jj - 1];
                                            i_son.son[jj] = i_son.son[jj - 1];
                                        }
                                        i_son.key[0] = pre_node.key[--pre_node.now_size];
                                        i_son.son[0] = pre_node.son[pre_node.now_size];
                                        i_son.now_size++;
                                        fa.key[i - 1] = i_son.key[0];
                                        node_river.update(pre_node, fa.son[i - 1]);
                                        node_river.update(i_son, fa.son[i]);
                                    } else if (i != fa.now_size) {
                                        for (int jj = j; jj < i_son.now_size; jj++) {
                                            i_son.key[jj] = i_son.key[jj + 1];
                                            i_son.son[jj] = i_son.son[jj + 1];
                                        }
                                        for (int jj = 0; jj < next_node.now_size; jj++) {
                                            i_son.key[i_son.now_size + jj] = next_node.key[jj];
                                            i_son.son[i_son.now_size + jj] = next_node.son[jj];
                                        }
                                        i_son.now_size += next_node.now_size;
                                        node_river.Delete(fa.son[i + 1]);
                                        fa.now_size--;
                                        fa.key[i] = fa.key[i + 1];
                                        for (int jj = i + 1; jj < fa.now_size; jj++) {
                                            fa.key[jj] = fa.key[jj + 1];
                                            fa.son[jj] = fa.son[jj + 1];
                                        }
                                        fa.son[fa.now_size] = fa.son[fa.now_size + 1];
                                        node_river.update(i_son, fa.son[i]);
                                    } else {
                                        for (int jj = 0; jj < j; jj++) {
                                            pre_node.key[pre_node.now_size + jj] = i_son.key[jj];
                                            pre_node.son[pre_node.now_size + jj] = i_son.son[jj];
                                        }
                                        for (int jj = j; jj < i_son.now_size; jj++) {
                                            pre_node.key[pre_node.now_size + jj] = i_son.key[jj + 1];
                                            pre_node.son[pre_node.now_size + jj] = i_son.son[jj + 1];
                                        }
                                        pre_node.now_size += i_son.now_size;
                                        node_river.Delete(fa.son[i]);
                                        fa.now_size--;
                                        fa.key[i - 1] = fa.key[i];
                                        for (int jj = i; jj < fa.now_size; jj++) {
                                            fa.key[jj] = fa.key[jj + 1];
                                            fa.son[jj] = fa.son[jj + 1];
                                        }
                                        fa.son[fa.now_size] = fa.son[fa.now_size + 1];
                                        node_river.update(pre_node, fa.son[i - 1]);
                                    }
                                    if (pos == root) node_river.update(fa, pos);
                                    return;
                                } else {
                                    for (int jj = j; jj < i_son.now_size; jj++) {
                                        i_son.key[jj] = i_son.key[jj + 1];
                                        i_son.son[jj] = i_son.son[jj + 1];
                                    }
                                    node_river.update(i_son, fa.son[i]);
                                }
                                return;
                            }
                            node_river.update(i_son, fa.son[i]);
                            return;
                        } else {
                            pre_val.next = search.next;
                            val_river.update(pre_val, pre_pos);
                            return;
                        }
                    } else val_river.update(search, now);
                    return;
                } else if (ret == -1) {
                    pre_pos = now;
                    pre_val = search;
                    now = search.next;
                } else break;
            }
            return;
        }
        delete_node(ind, val, i_son, fa.son[i]);
        if (pos == root) {
            if (i_son.now_size < ((M + 1) >> 1) - 1) {
                BPT_node next_node, pre_node;
                if (i != fa.now_size) node_river.read(next_node, fa.son[i + 1]);
                if (i != 0) node_river.read(pre_node, fa.son[i - 1]);
                if (i != fa.now_size && next_node.now_size > ((M + 1) >> 1) - 1) {
                    BPT_node extra_node;
                    node_river.read(extra_node, next_node.son[0]);
                    i_son.key[i_son.now_size] = extra_node.key[0];
                    i_son.son[++i_son.now_size] = next_node.son[0];
                    fa.key[i] = next_node.key[0];
                    next_node.now_size--;
                    for (int jj = 0; jj < next_node.now_size; jj++) {
                        next_node.key[jj] = next_node.key[jj + 1];
                        next_node.son[jj] = next_node.son[jj + 1];
                    }
                    next_node.son[next_node.now_size] = next_node.son[next_node.now_size + 1];
                    node_river.update(next_node, fa.son[i + 1]);
                    node_river.update(i_son, fa.son[i]);
                } else if (i != 0 && pre_node.now_size > ((M + 1) >> 1) - 1) {
                    BPT_node extra_node;
                    node_river.read(extra_node, i_son.son[0]);
                    i_son.son[i_son.now_size + 1] = i_son.son[i_son.now_size];
                    for (int jj = i_son.now_size++; jj > 0; jj--) {
                        i_son.key[jj] = i_son.key[jj - 1];
                        i_son.son[jj] = i_son.son[jj - 1];
                    }
                    i_son.key[0] = extra_node.key[0];
                    i_son.son[0] = pre_node.son[pre_node.now_size];
                    fa.key[i - 1] = pre_node.key[pre_node.now_size--];
                    node_river.update(pre_node, fa.son[i - 1]);
                    node_river.update(i_son, fa.son[i]);
                } else if (i != fa.now_size) {
                    BPT_node extra_node;
                    node_river.read(extra_node, next_node.son[0]);
                    i_son.key[i_son.now_size++] = extra_node.key[0];
                    for (int jj = 0; jj < next_node.now_size; jj++) {
                        i_son.key[i_son.now_size + jj] = next_node.key[jj];
                        i_son.son[i_son.now_size + jj] = next_node.son[jj];
                    }
                    i_son.now_size += next_node.now_size;
                    i_son.son[i_son.now_size] = next_node.son[next_node.now_size];
                    node_river.Delete(fa.son[i + 1]);
                    fa.now_size--;
                    fa.key[i] = fa.key[i + 1];
                    for (int jj = i + 1; jj < fa.now_size; jj++) {
                        fa.key[jj] = fa.key[jj + 1];
                        fa.son[jj] = fa.son[jj + 1];
                    }
                    fa.son[fa.now_size] = fa.son[fa.now_size + 1];
                    node_river.update(i_son, fa.son[i]);
                } else {
                    BPT_node extra_node;
                    node_river.read(extra_node, i_son.son[0]);
                    pre_node.key[pre_node.now_size++] = extra_node.key[0];
                    for (int jj = 0; jj < i_son.now_size; jj++) {
                        pre_node.key[pre_node.now_size + jj] = i_son.key[jj];
                        pre_node.son[pre_node.now_size + jj] = i_son.son[jj];
                    }
                    pre_node.now_size += i_son.now_size;
                    pre_node.son[pre_node.now_size] = i_son.son[i_son.now_size];
                    node_river.Delete(fa.son[i]);
                    fa.now_size--;
                    fa.key[i - 1] = fa.key[i];
                    for (int jj = i; jj < fa.now_size; jj++) {
                        fa.key[jj] = fa.key[jj + 1];
                        fa.son[jj] = fa.son[jj + 1];
                    }
                    fa.son[fa.now_size] = fa.son[fa.now_size + 1];
                    node_river.update(pre_node, fa.son[i - 1]);
                }
                node_river.update(fa, pos);
                return;
            }
            node_river.update(i_son, fa.son[i]);
            return;
        }
        if (i_son.now_size < ((M + 1) >> 1) - 1) {
            BPT_node next_node, pre_node;
            if (i != fa.now_size) node_river.read(next_node, fa.son[i + 1]);
            if (i != 0) node_river.read(pre_node, fa.son[i - 1]);
            if (i != fa.now_size && next_node.now_size > ((M + 1) >> 1) - 1) {
                BPT_node extra_node;
                node_river.read(extra_node, next_node.son[0]);
                i_son.key[i_son.now_size] = extra_node.key[0];
                i_son.son[++i_son.now_size] = next_node.son[0];
                fa.key[i] = next_node.key[0];
                next_node.now_size--;
                for (int jj = 0; jj < next_node.now_size; jj++) {
                    next_node.key[jj] = next_node.key[jj + 1];
                    next_node.son[jj] = next_node.son[jj + 1];
                }
                next_node.son[next_node.now_size] = next_node.son[next_node.now_size + 1];
                node_river.update(next_node, fa.son[i + 1]);
                node_river.update(i_son, fa.son[i]);
            } else if (i != 0 && pre_node.now_size > ((M + 1) >> 1) - 1) {
                BPT_node extra_node;
                node_river.read(extra_node, i_son.son[0]);
                i_son.son[i_son.now_size + 1] = i_son.son[i_son.now_size];
                for (int jj = i_son.now_size++; jj > 0; jj--) {
                    i_son.key[jj] = i_son.key[jj - 1];
                    i_son.son[jj] = i_son.son[jj - 1];
                }
                i_son.key[0] = extra_node.key[0];
                i_son.son[0] = pre_node.son[pre_node.now_size];
                fa.key[i - 1] = pre_node.key[pre_node.now_size--];
                node_river.update(pre_node, fa.son[i - 1]);
                node_river.update(i_son, fa.son[i]);
            } else if (i != fa.now_size) {
                BPT_node extra_node;
                node_river.read(extra_node, next_node.son[0]);
                i_son.key[i_son.now_size++] = extra_node.key[0];
                for (int jj = 0; jj < next_node.now_size; jj++) {
                    i_son.key[i_son.now_size + jj] = next_node.key[jj];
                    i_son.son[i_son.now_size + jj] = next_node.son[jj];
                }
                i_son.now_size += next_node.now_size;
                i_son.son[i_son.now_size] = next_node.son[next_node.now_size];
                node_river.Delete(fa.son[i + 1]);
                fa.now_size--;
                fa.key[i] = fa.key[i + 1];
                for (int jj = i + 1; jj < fa.now_size; jj++) {
                    fa.key[jj] = fa.key[jj + 1];
                    fa.son[jj] = fa.son[jj + 1];
                }
                fa.son[fa.now_size] = fa.son[fa.now_size + 1];
                node_river.update(i_son, fa.son[i]);
            } else {
                BPT_node extra_node;
                node_river.read(extra_node, i_son.son[0]);
                pre_node.key[pre_node.now_size++] = extra_node.key[0];
                for (int jj = 0; jj < i_son.now_size; jj++) {
                    pre_node.key[pre_node.now_size + jj] = i_son.key[jj];
                    pre_node.son[pre_node.now_size + jj] = i_son.son[jj];
                }
                pre_node.now_size += i_son.now_size;
                pre_node.son[pre_node.now_size] = i_son.son[i_son.now_size];
                node_river.Delete(fa.son[i]);
                fa.now_size--;
                fa.key[i - 1] = fa.key[i];
                for (int jj = i; jj < fa.now_size; jj++) {
                    fa.key[jj] = fa.key[jj + 1];
                    fa.son[jj] = fa.son[jj + 1];
                }
                fa.son[fa.now_size] = fa.son[fa.now_size + 1];
                node_river.update(pre_node, fa.son[i - 1]);
            }
            return;
        }
        node_river.update(i_son, fa.son[i]);
    }
    void remove(const std::pair<Key, T> &val) {
        if (_size == 0) return;
        BPT_node rt;
        node_river.read(rt, root);
        if (rt.is_leaf) {
            int i = lower_key(val.first, rt);
            if (i == rt.now_size || rt.key[i] != val.first) return;
            int now = rt.son[i], pre_pos;
            val_vec search, pre_val;
            while (now) {
                val_river.read(search, now);
                int ret = search.erase(val.second);
                if (ret == 0) {
                    if (search.now_size == 0) {
                        val_river.Delete(now);
                        if (now == rt.son[i]) {
                            rt.son[i] = search.next;
                            if (!search.next) {
                                rt.now_size--;
                                _size--;
                                if (!_size) {
                                    node_river.initialise();
                                    val_river.initialise();
                                    return;
                                }
                                for (int j = i; j < rt.now_size; j++) {
                                    rt.key[j] = rt.key[j + 1];
                                    rt.son[j] = rt.son[j + 1];
                                }
                                node_river.update(rt, root);
                                return;
                            }
                            node_river.update(rt, root);
                            return;
                        } else {
                            pre_val.next = search.next;
                            val_river.update(pre_val, pre_pos);
                            return;
                        }
                    } else val_river.update(search, now);
                    return;
                } else if (ret == -1) {
                    pre_pos = now;
                    pre_val = search;
                    now = search.next;
                } else return;
            }
            return;
        }
        delete_node(val.first, val.second, rt, root);
        if (rt.now_size == 0) {
            node_river.Delete(root);
            root = rt.son[0];
        }
    }

    void clear() {
        _size = 0;
        node_river.initialise();
        val_river.initialise();
    }

    void modify(const std::pair<Key, T> &val, T new_val) {
        if (_size == 0) return;
        BPT_node rt;
        node_river.read(rt, root);
        while (!rt.is_leaf) {
            int i = lower_key(val.first, rt);
            if (i == rt.now_size || rt.key[i] != val.first) return;
            node_river.read(rt, rt.son[i]);
        }
        int i = lower_key(val.first, rt);
        if (i == rt.now_size || rt.key[i] != val.first) return;
        int now = rt.son[i];
        val_vec search;
        while (now) {
            val_river.read(search, now);
            if (val.second > search.val[search.now_size - 1]) now = search.next;
            else {
                int pos = search.lower_val(val.second);
                if (pos == search.now_size || search.val[pos] != val.second) return;
                search.val[pos] = new_val;
                val_river.update(search, now);
                return;
            }
        }
    }
};

#endif //BPTREE_HPP_BPTREE2_HPP
