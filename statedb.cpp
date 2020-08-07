#include <iostream>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/member.hpp>
#include "db_viewer.hpp"
#include "eos_db.hpp"

using namespace std;
using namespace chainbase;
using namespace boost::multi_index;

struct config
{
    boost::filesystem::path database_dir = "state";
    uint64_t cache_size = 340 * 1024 * 1024ll;
    chainbase::pinnable_mapped_file::map_mode db_map_mode = chainbase::pinnable_mapped_file::map_mode::mapped;
    vector<string> db_hugepage_paths;
};

int main(int argc, char **argv)
{
    config cfg;

    try
    {
        chainbase::database db(cfg.database_dir, database::read_write, cfg.cache_size);
        eos_db _eosdb(db);
        db.add_index<table_id_multi_index>();
        db.add_index<key_value_index>();
        db.add_index<index64_index>();
        db.add_index<index128_index>();
        db.add_index<index256_index>();

        auto receiver = name("receiver");
        auto table = name("table");
        auto user_public_key = fc::crypto::private_key::generate().get_public_key();
        auto scope = fc::sha256::hash<std::string>("scope");

         _eosdb.db_store_by_scope(receiver, scope, table, receiver, "100.0000 EOS", strlen("100.0000 EOS"));

        db_viewer<table_id_multi_index>::walk(db, [](const auto &row) {
            std::cout << row.id << ": " << row.payer << ": " << row.value << std::endl;
        });

        const auto &itr = _eosdb.find_or_create_table(receiver, scope, table, receiver);
         std::cout << itr.value << std::endl;

         _eosdb.db_update_by_scope(itr, receiver, "20.0000 EOS", strlen("100.0000 EOS"));
    
        db_viewer<table_id_multi_index>::walk(db, [](const auto &row) {
            std::cout << row.id << ": " << row.payer << ": " << row.value << std::endl;
        });
        
        // std::cout << itr.value << std::endl;
        // db_viewer<key_value_index>::walk(db, [](const auto &row) {
        //     std::cout << name(row.primary_key) << ": " << row.value << std::endl;
        // });


    }
    catch (...)
    {
        std::cout << "Error insert"
                  << "\n";
        throw;
    }
    return 0;
}