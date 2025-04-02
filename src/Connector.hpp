#include <string>
#include <typeinfo>
#include <vector>
#include <boost/utility/string_view.hpp>
#include <bsoncxx/json.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>

using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::make_array;
using bsoncxx::builder::basic::make_document;

class Connector
{
private:
    mongocxx::instance inst;
    mongocxx::uri uri;
    mongocxx::client conn;
    std::vector<mongocxx::collection> coll;
    std::string db_name;
public:
    Connector(const std::string& url, int L, std::string db_name, bool id_access);
    ~Connector();
    void Insert(int oram_index, int pos, std::string cipher);
    std::string Query(int oram_index, int pos);
    void Update(int oram_index, int pos, std::string cipher);
    void Bulk_Insert(int oram_index, std::vector<int> pos, std::vector<std::string> cipher);
    void drop();
};
