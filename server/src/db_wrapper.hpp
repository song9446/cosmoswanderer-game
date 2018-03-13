#include "lmdb.h"
#include <iostream>
#include <stdio.h>
#include <cstdint>
#include <cstring>
#include <cstdlib>


class ByteData {
public:
    size_t len;
    void* data;
};
class LMDBWrapper {
    int rc;
    MDB_env *env;
    MDB_dbi dbi;
    MDB_txn *txn;
public:
    LMDBWrapper(const char* path){
        char buf[255];
        sprintf(buf, "mkdir -p %s", path);
        int t = system(buf);
        rc = mdb_env_create(&env);
        rc = mdb_env_open(env, path, MDB_CREATE, 0664);
        if(rc) std::cout << mdb_strerror(rc)  << std::endl;
    }
    ~LMDBWrapper(){
        mdb_env_close(env);
    }
    bool exist(MDB_val key){
        bool result = false;
        MDB_val data;
        rc = mdb_txn_begin(env, NULL, 0, &txn);
        rc = mdb_open(txn, NULL, 0, &dbi);
        rc = mdb_get(txn, dbi, &key, &data);
        if(!rc && data.mv_size > 0) result = true;
        rc = mdb_txn_commit(txn);
        mdb_close(env, dbi);
        return result;
    }
    template<typename T>
    bool exist(size_t ks, const T* kd){
        MDB_val key={ks, reinterpret_cast<void*>(const_cast<T*>(kd))};
        return exist(key);
    }
    int put(MDB_val key, MDB_val value){
        rc = mdb_txn_begin(env, NULL, 0, &txn);
        rc = mdb_open(txn, NULL, 0, &dbi);
        rc = mdb_put(txn, dbi, &key, &value, 0);
        rc = mdb_txn_commit(txn);
        mdb_close(env, dbi);
        return rc;
    }
    template<typename T, typename T2>
    int put(size_t ks, const T* kd, size_t vs, const T2* vd){
        MDB_val key={ks, reinterpret_cast<void*>(const_cast<T*>(kd))}, value={vs, reinterpret_cast<void*>(const_cast<T2*>(vd))}; 
        return put(key, value);
    }
    template<typename T2>
    size_t get(MDB_val key, const T2* value){
        MDB_val data;
        rc = mdb_txn_begin(env, NULL, 0, &txn);
        rc = mdb_open(txn, NULL, 0, &dbi);
        rc = mdb_get(txn, dbi, &key, &data);
        size_t size = data.mv_size;
        std::memcpy(reinterpret_cast<void*>(const_cast<T2*>(value)), data.mv_data, size);
        rc = mdb_txn_commit(txn);
        mdb_close(env, dbi);
        return (!rc)*size;
    }
    template<typename T, typename T2>
    size_t get(size_t ks, const T* kd, const T2* value){
        MDB_val key={ks, reinterpret_cast<void*>(const_cast<T*>(kd))};
        return get<T2>(key, value);
    }
    template<typename T, typename T2>
    int compare(size_t ks, const T* kd, size_t vs, const T2* vd){
        int result=0;
        MDB_val key={ks, reinterpret_cast<void*>(const_cast<T*>(kd))}, data;
        rc = mdb_txn_begin(env, NULL, 0, &txn);
        rc = mdb_open(txn, NULL, 0, &dbi);
        rc = mdb_get(txn, dbi, &key, &data);
        //int size = data.mv_size;
        result = (vs > data.mv_size) - (data.mv_size < vs);
        result = (!result) * memcmp(vd, data.mv_data, vs);
        rc = mdb_txn_commit(txn);
        mdb_close(env, dbi);
        return result;
    }
};
/*
int main(){
    LMDBWrapper a("test");
    int c = 3;
    a.put(4, "abcd", 4, &c);
    int b=0;
    a.get(4, "abcd", &b);
    std::cout << b << std::endl;
}
*/
