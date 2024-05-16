#include "mongoConnector.hpp"

mongoConnector::mongoConnector(const std::string& url, int L, std::string db_name, bool is_access)
        :inst(), uri(url), conn(uri){

    // mongocxx::client conn{mongocxx::uri{"mongodb://localhost:27017"}};
    db_name = "bOMAP_" + db_name + "_" + std::to_string(L);
    if(is_access == false) conn[db_name].drop();
    coll.push_back(conn[db_name]["leaforam"]);
    coll.push_back(conn[db_name]["mid2oram"]);
    for(int i = 2; i < L - 1; i++)
    {
        std::string coll_name = "mid1oram-" + std::to_string(i-2);
        coll.push_back(conn[db_name][coll_name]);
    }
    coll.push_back(conn[db_name]["root"]);
    if(is_access == false){
        mongocxx::options::index index_options{};
        index_options.unique(true);
        for(int i = 0; i < L; i++) {
            // coll[i].drop();
            coll[i].create_index(make_document(kvp("pos", 1)), index_options);
        }
    }
}

mongoConnector::~mongoConnector()
{
}

void mongoConnector::Insert(int oram_index, int pos, std::string cipher)
{
    coll[oram_index].insert_one(
            make_document(kvp("pos", pos),kvp("cipher", cipher)));
}

void mongoConnector::Bulk_Insert(int oram_index, std::vector<int> pos, std::vector<std::string> cipher)
{
    size_t num = pos.size();
    auto bulk = coll[oram_index].create_bulk_write();
    for(size_t i = 0; i < num; i++)
    {
        auto doc = make_document(kvp("pos", pos[i]), kvp("cipher", cipher[i]));
        mongocxx::model::insert_one insert_op{doc.view()};
        bulk.append(insert_op);
    }
    auto result = bulk.execute();
    std::cout << "Upserted IDs" << std::endl;
    for (const auto& id : result->upserted_ids()) {
        std::cout << "Bulk write index: " << id.first << std::endl
                  << (id.second.get_oid().value.to_string()) << std::endl;
    }
}

std::string mongoConnector::Query(int oram_index, int pos)
{
    mongocxx::options::find opts{};
    opts.projection(make_document(kvp("cipher", 1),kvp("_id", 0)));
    auto cursor = coll[oram_index].find_one(make_document(kvp("pos", pos)), opts);
    auto doc_view = cursor->view();
    auto ret_str_view = doc_view["cipher"].get_string().value;
    // boost::string_view ret_str_view = doc_view["cipher"].get_string().value;
    // std::string_view strv(ret_str_view.data(), ret_str_view.size());
    // std::string ret{strv};
    std::string strv(ret_str_view.data(), ret_str_view.size());
    return strv;
}

void mongoConnector::Update(int oram_index, int pos, std::string cipher)
{
    coll[oram_index].update_one(
            make_document(kvp("pos", pos)),
            make_document(kvp("$set", make_document(kvp("cipher", cipher)))));
}