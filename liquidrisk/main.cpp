#include "../dappservices/multi_index.hpp"
#include "../dappservices/log.hpp"
#include "../dappservices/oracle.hpp"
#include "../dappservices/cron.hpp"
#include "../dappservices/vaccounts.hpp"
#include "../dappservices/readfn.hpp"
#include "../dappservices/vcpu.hpp"
#include "../dappservices/multi_index.hpp"

#define DAPPSERVICES_ACTIONS() \
  XSIGNAL_DAPPSERVICE_ACTION \
  IPFS_DAPPSERVICE_ACTIONS \
  VACCOUNTS_DAPPSERVICE_ACTIONS \
  LOG_DAPPSERVICE_ACTIONS \
  CRON_DAPPSERVICE_ACTIONS \
  ORACLE_DAPPSERVICE_ACTIONS \
  VCPU_DAPPSERVICE_ACTIONS \
  READFN_DAPPSERVICE_ACTIONS

#define DAPPSERVICE_ACTIONS_COMMANDS() \
  IPFS_SVC_COMMANDS() \
  ORACLE_SVC_COMMANDS() \
  CRON_SVC_COMMANDS() \
  VACCOUNTS_SVC_COMMANDS() \
  LOG_SVC_COMMANDS() \
  READFN_SVC_COMMANDS() \
  VCPU_SVC_COMMANDS()

#define CONTRACT_NAME() liquidrisk


#define EOS_SYMBOL symbol(symbol_code("EOS"), 4)
#define DAPP_SYMBOL symbol(symbol_code("DAPP"), 4)

using std::string;

using namespace std;
using namespace eosio;


using std::string;

using namespace eosio;

CONTRACT_START()

      // globals
      // double alphatest = 0.95; // 0.95 for 95% CVaR
      // TODO: work on population of the vector and erasion of elements within the vector
     

      bool timer_callback(name timer, std::vector<char> payload, uint32_t seconds){
          return false;
      }

     [[eosio::action]] void testschedule() {
        std::vector<char> payload;
        schedule_timer(_self, payload, 2);
      }

      struct uri_string {
          // a URI string with hardcoded parameters
          string uriStr = "risk://cvar/montecarlo/EOS:IQ;0.5:0.5;0.95";
      };



      TABLE users_s {
         name user;
         asset balance;
         string CVaR;
         vector<asset> coll_basket; // a basket of diverse tokens
         uint64_t primary_key()const { return balance.contract.value; }
      };

      typedef dapp::multi_index<"vaccounts"_n, users_s> users_table;
      typedef eosio::multi_index<".vaccounts"_n, users_s> users_table_t_v_abi;
      TABLE shardbucket {
          std::vector<char> shard_uri;
          uint64_t shard;
          uint64_t primary_key() const { return shard; }
      };
      typedef eosio::multi_index<"vaccounts"_n, shardbucket> users_table_t_abi;


     [[eosio::action]] void withdraw( name to, name token_contract){

            require_auth( to );
            require_recipient( to );
            auto received = sub_all_cold_balance(to, token_contract);
            action(permission_level{_self, "active"_n}, token_contract, "transfer"_n,
               std::make_tuple(_self, to, received, std::string("withdraw")))
            .send();
      }

     void transfer( name from,
                     name to,
                     asset        quantity,
                     string       memo ){
        require_auth(from);
        if(to != _self || from == _self || from == "eosio"_n || from == "eosio.stake"_n || from == to)
            return;
        if(memo == "seed transfer")
            return;
        if (memo.size() > 0){
          name to_act = name(memo.c_str());
          eosio::check(is_account(to_act), "The account name supplied is not valid");
          require_recipient(to_act);
          from = to_act;
        }
        asset received(quantity, get_first_receiver());
        add_cold_balance(from, received);
     }
 

      // the getCVaR function is passed a default value
      string getCVaR(string uri_str){
             std::vector<char> uri = std::vector<char>(uri_str.begin(), uri_str.end());
             getURI(uri, [&](auto& results){
                res = string( results[0].result.begin(), results[0].result.end() );
                return results[0].result;
          });
      }
      
      // this action updates the CVaR value
      ACTION setCVaR(){
        uri_struct uri_;
        auto _cvar = getCVaR(uri_.uriStr);
        user_table _user( _self, _self.value );
        auto itr = _user.find( _self.value );
        _user.modify(itr, eosio::same_payer, [&](auto &s) { 
            s.CVaR = _cvar; 
          }); 
      }

      
   private:
      asset sub_all_cold_balance( name owner, name token_contract){
           cold_accounts_t from_acnts( _self, owner.value );
           const auto& from = from_acnts.get( token_contract.value, "no balance object found" );
           auto res = from.balance;
           from_acnts.erase( from );
           return res;
      }

      void add_cold_balance( name owner, asset value){
           cold_accounts_t to_acnts( _self, owner.value );
           auto to = to_acnts.find( value.contract.value );
           if( to == to_acnts.end() ) {
              to_acnts.emplace(_self, [&]( auto& a ){
                a.balance = value;
              });
           } else {
              to_acnts.modify( *to, eosio::same_payer, [&]( auto& a ) {
                a.balance += value;
              });
           }
      }

    VACCOUNTS_APPLY(((dummy_action_hello)(hello))((dummy_action_hello)(hello2)))

};
EOSIO_DISPATCH_SVC_TRX(CONTRACT_NAME(), (withdraw)(hello)(hello2)(regaccount)(testschedule))