#include <websocketpp/config/asio.hpp>

#include <websocketpp/server.hpp>

#include <iostream>
#include <string>
#include <fstream>
#include "CardGame.h"
typedef websocketpp::server<websocketpp::config::asio> server;
//typedef websocketpp::server<websocketpp::config::asio_tls> server;

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

// pull out the type of messages sent by our config
typedef websocketpp::config::asio::message_type::ptr message_ptr;
typedef websocketpp::lib::shared_ptr<websocketpp::lib::asio::ssl::context> context_ptr;
std::string jsonmessagebuilder(std::string msg, std::string data) {
    string out = "{\"msg\":\"" + msg + "\"," + data;
    out = out.substr(0, out.size() - 1);
    out += "}";
    return out;
}
std::string tojson(std::string key, std::string val) {
    return "\"" + key + "\":\"" + val + "\",";
}
std::string rawtojson(std::string key, std::string val) {
    return "\"" + key + "\":" + val + ",";
}
std::string tojson(std::string key, int val) {
    return "\"" + key + "\":" + std::to_string(val) + ",";
}
std::string tojson(std::string key, vector<int> val) {
    string out = "[";
    for (int i : val) {
        out += to_string(i) + ",";
    }
    if (!val.empty())out = out.substr(0, out.size() - 1);
    out += "]";
    return rawtojson(key, out);
}
class GameSession;
class PlayerAdapter {
public:
    websocketpp::connection_hdl hdl;
    int i=-1;
    int token=-1;
    Player* player=NULL;
    GameSession* session= NULL;
    bool valid = true;
    PlayerAdapter(websocketpp::connection_hdl hdl,int i,int token) {
        this->hdl = hdl;
        this->i = i;
        this->token = token;
    }
    void send(server* s,  string msg) {
        if(valid)
        s->send(hdl, msg, websocketpp::frame::opcode::text);
    }
    void sendOthers(server* s,  vector<PlayerAdapter*> others, string msg) {
        for (auto& connection : others) {
            if(connection->valid)
            if (connection->hdl.lock() != hdl.lock()) {
                try
                {
                    s->send(connection->hdl,
                        msg
                        , websocketpp::frame::opcode::text);
                }
                catch (const std::exception&)
                {

                }
                
            }
        }
    }
};
inline bool isInteger(const std::string& s)
{
    if (s.empty() || ((!isdigit(s[0])) && (s[0] != '-') && (s[0] != '+'))) return false;

    char* p;
    strtol(s.c_str(), &p, 10);

    return (*p == 0);
}
template<typename T>
void vectorremove(vector<T> v, size_t i) {
    if(i>=0&&i<v.size())
    v.erase(v.begin() + i);
}
template<typename T>
T vectorfind(vector<T> v, T i) {
    for (auto e : v) {
        if (e == i) {
            return e;
        }
    }
    return NULL;
}
template<typename T>
size_t vectorgeti(vector<T> v, T c) {
    for (size_t i = 0; i < v.size(); i++)
    {
        if (v[i] == c) {
            return i;
        }
    }
    return -1;
}
class GameSession {
public:
    std::vector<PlayerAdapter*> players;
    CardGame* cardgame = new CardGame();
    int token=0;
    bool ispublic = true;
    GameSession(int token) {
        this->token=token;
        cardgame = new CardGame();
    }
    void onmessage(websocketpp::connection_hdl hdl, string cmd) {

    }
    PlayerAdapter* getPlayerNow() {
        for (auto p : players) {
            if (p->player == cardgame->getPlayerNow()) {
                return p;
            }
        }
        return NULL;
    }
    void addPlayer(PlayerAdapter* p) {
        
        if (vectorfind(players, p) == NULL) {
            p->player = cardgame->addPlayer();
            p->i = p->player->pid;
            p->session = this;
            p->token = newToken();
            players.push_back(p);
            
        }
    }
    void removeplayer(PlayerAdapter* p) {
        vectorremove(players, vectorgeti(players, p));
    }
    Card* playCard(PlayerAdapter* p, int c) {

        return cardgame->playCard(p->player, c);
    }
    int newToken() {
        int i = 0;
        bool found = true;
        while (found) {
            i = rand();
            for (auto e : players) {
                if (e->token == i) {
                    found = true;
                }
            }
            found = false;
        }
        return i;
    }
    PlayerAdapter* getByToken(int token) {
        return NULL;
    }
    void sendAll(server* s, string msg) {
        for (auto& connection : players) {
            	if(connection->valid){
                	if(!connection->hdl.expired()){
				if(connection->hdl.lock()){
            				try{
						s->send(
				connection->hdl,
                		msg,
				websocketpp::frame::opcode::text);
        				}catch(websocketpp::exception e){
				printf("send all exception\n");
					}
				}
			}
		}
	}
    }
};

class GameAdapter {
public:
    server* s;
    std::vector<GameSession*> sessions;
    
    vector<PlayerAdapter*> players;
    int newToken() {
        int i =0;
        bool found = true;
        while (found) {
            i = rand();
            for (auto e : sessions) {
                if (e->token == i) {
                    found = true;
                }
            }
            found = false;
        }
        return i;
    }
    GameSession* getByToken(int i) {
        for (auto e : sessions) {
            if (e->token == i) {
                return e;
            }
        }
        return NULL;
    }
    PlayerAdapter* getIdlePlayer(websocketpp::connection_hdl hdl) {
        for (auto e : players) {
            if (e->hdl.lock() == hdl.lock()) {
                return e;
            }
        }
        return NULL;
    }
    void checkFinished(PlayerAdapter* pa,GameSession* gs) {
        if (pa->player != NULL) {
            
            if (pa->player->deck.empty()) {
                if (gs->cardgame->players.size() == 1) {
                    gs->sendAll(s, jsonmessagebuilder("gameend", ""));

                }
                if (gs->cardgame->finished) {
                    gs->sendAll(s, jsonmessagebuilder("reset", ""));
                    for (auto p : gs->players) {
                        p->session = NULL;
                        
                    }
                    
                }


                s->send(pa->hdl, jsonmessagebuilder("youwon", ""), websocketpp::frame::opcode::text);
                string mesg = jsonmessagebuilder("plwon",
                    tojson("pl", pa->i));
                gs->sendAll(s, mesg);
                gs->removeplayer(pa);
                gs->getPlayerNow()->send(s, jsonmessagebuilder("itsyourturn", ""));
                pa->player = NULL;
            }
        }
    }

    void onmessageint(websocketpp::connection_hdl hdl, string cmd,int val) {
        
        if (cmd.find("join")==0) {
            print("onmessageint" + cmd + to_string(val));
            PlayerAdapter* pa = getIdlePlayer(hdl);
            if (pa!=NULL) {
                if (pa->session == NULL) {
                    GameSession* gs = getByToken(val);
                    print("got here!");

                    if (gs != NULL) {
                        print("and over here!");
                        gs->addPlayer(pa);
                        pa->token = gs->newToken();
                        pa->send(s,
                            jsonmessagebuilder("youjoined",
                                tojson("token", pa->token)
                            )
                        );
                        pa->sendOthers(s, gs->players, jsonmessagebuilder("joined",

                            tojson("player", pa->i)));
                    }
                    else {

                    }
                }
                else {
                    pa->send(s,
                        jsonmessagebuilder("alreadyjoined",""
                        )
                    );
                }
            }
        }
        else if (cmd.find("choose") == 0) {
            PlayerAdapter* pa = getIdlePlayer(hdl);
            if (pa) {
                GameSession* gs = pa->session;
                if (gs) {
                    if (gs->cardgame->playerchoosing) {
                        
                                
                                gs->cardgame->chooseColor(val);
                                gs->sendAll(s, jsonmessagebuilder("chose", tojson("id", val)));
                                
                                checkFinished(pa, gs);
                                gs->getPlayerNow()->send(s, jsonmessagebuilder("itsyourturn", ""));
                        
                       
                    }
                }
            }
        }
        else
            if (cmd.find("reconnect") == 0) {
                
               
                    for (size_t i = 0; i < players.size(); i++)
                    {
                        auto p = players[i];
                    
                    if (!p->valid) {
                        if (p->token = val) {
                            p->hdl = hdl;
                            p->valid = true;
                            p->session->sendAll(s, jsonmessagebuilder("rejoined", tojson("pl", p->i)));
                        }
                    }
                    vectorremove(players, i);
                }
            }

            else
            if (cmd.find("play") == 0) {
                PlayerAdapter* pa = getIdlePlayer(hdl);
                if (pa) {
                    GameSession* gs = pa->session;
                    if (gs) {
                        if (!gs->cardgame->finished) {
                            Card* c = gs->playCard(pa, val);
                            if (c != NULL) {

                                if (c->getNumber() == 12) {
                                    s->send(pa->hdl, jsonmessagebuilder("choose", ""), websocketpp::frame::opcode::text);

                                    gs->sendAll(s, jsonmessagebuilder("cardnow",
                                        tojson("cardnow", gs->cardgame->cardnow->id) + tojson("pl", pa->i)
                                    ));
                                }

                                if (c->getNumber() == 9) {
                                    //s->send(hdl, jsonmessagebuilder("itsyourturn", ""), websocketpp::frame::opcode::text);
                                }
                                if (c->getNumber() == 10) {
                                    if (gs->cardgame->players.size() == 2) {
                                        s->send(hdl, jsonmessagebuilder("itsyourturn", ""), websocketpp::frame::opcode::text);
                                    }
                                }

                                if (!gs->cardgame->playerchoosing) {
                                    gs->sendAll(s, jsonmessagebuilder("cardnow",
                                        tojson("cardnow", gs->cardgame->cardnow->id) + tojson("pl", pa->i)
                                    ));
                                    gs->getPlayerNow()->send(s, jsonmessagebuilder("itsyourturn", ""));

                                }

                            }
                            else {

                                if (gs->cardgame->canPlayerPlay(gs->cardgame->playernow))
                                    gs->getPlayerNow()->send(s, jsonmessagebuilder("itsyourturn", ""));
                            }
                            if (pa->player != NULL)
                                pa->send(s, jsonmessagebuilder("update",
                                    tojson("cards", pa->player->getIntArr())
                                ));


                            checkFinished(pa, gs);
                            gs->getPlayerNow()->send(s, jsonmessagebuilder("itsyourturn", ""));
                            //s->send(hdl, s->get_con_from_hdl(hdl)->get_remote_endpoint() + msg->get_payload(), msg->get_opcode());
                        }
                    }
                }
            }
            else if (cmd.find("start") == 0) {
                PlayerAdapter* pa = getIdlePlayer(hdl);
                if (pa) {
                    GameSession* gs = pa->session;
                    if (gs) {
                        if (pa->i == 1) {
                            if (gs->cardgame->start(val).empty()) {

                                gs->sendAll(s, jsonmessagebuilder("cardnow",
                                    tojson("cardnow", gs->cardgame->cardnow->id)));
                                for (auto& pl : gs->players) {
                                    pl->send(s, jsonmessagebuilder("update",
                                        tojson("cards", pl->player->getIntArr())));

                                }
                                pa = gs->getPlayerNow();
                                if (pa) {
                                    if (gs->cardgame->playerchoosing) {

                                        pa->send(s, jsonmessagebuilder("choose", ""));
                                    }
                                    else {

                                        pa->send(s, jsonmessagebuilder("itsyourturn", ""));

                                    }
                                }
                            }
                            else {
                                pa->send(s, "toofewplayers");
                            }
                        }
                    }
                }
            }
    
        
    }
    void onmessage(websocketpp::connection_hdl hdl, string cmd) {
        if (!cmd.empty()) {
            
            if (isdigit(cmd[cmd.size() - 1])) {
                int i = cmd.size() - 1;
                while (isdigit(cmd[i]) && i >= 0) {
                    i--;
                }
                i++;
                if (i < cmd.size()) {
                    std::string last = cmd.substr(i);
                    print("cmdin:" + cmd + last);
                    if (!last.empty()) {
                        if (isInteger(last)) {
                            int i = atoi(last.c_str());
                            onmessageint(hdl, cmd, i);
                        }
                    }
                }
            }
            else
                if (cmd == "newgame") {
                    int token = newToken();
                    PlayerAdapter* pa = getIdlePlayer(hdl);
                    if (pa) {
                        if (pa->session == NULL) {
                            
                            sessions.push_back(new GameSession(token));
                            onmessageint(hdl, "join", token);
                            s->send(hdl, jsonmessagebuilder("newgame", tojson("gid", token)), websocketpp::frame::opcode::text);
                        }
                    }
                    
                }else
            if (cmd == "list") {
                vector<int> out;
                for (auto e : sessions) {
                    if (e->ispublic) {
                        if (e->cardgame->opentojoin) {
                            out.push_back(e->token);
                        }
                    }
                    
                }
                s->send(hdl, jsonmessagebuilder("sesslist", tojson("list", out)), websocketpp::frame::opcode::text);
                
            }
            else if (cmd.find("joinrandom") == 0) {
                int opengamescount = 0;
                for (auto e : sessions) {
                    if (e->ispublic) {
                        if (e->cardgame->opentojoin) {
                            opengamescount++;
                        }
                    }
                }
                if (opengamescount > 0) {
                    int random = rand() % opengamescount;
                    opengamescount = 0;
                    for (auto e : sessions) {
                        if (e->ispublic) {
                            if (e->cardgame->opentojoin) {
                                if (opengamescount == random) {
                                    onmessageint(hdl, "join", e->token);
                                }
                            }
                        }
                    }
                }
                
            }
            else if (cmd.find("leave") == 0) {
                PlayerAdapter* pa = getIdlePlayer(hdl);
                if (pa) {
                    GameSession* gs = pa->session;
                    if (gs) {
                        gs->removeplayer(pa);
                        
                        pa->session = NULL;
                        pa->player = NULL;
                        pa->send(s, jsonmessagebuilder("reset", ""));
                    }
                }
            }
            else if (cmd.find("reset") == 0) {
                PlayerAdapter* pa = getIdlePlayer(hdl);
                if (pa) {
                    GameSession* gs = pa->session;
                    if (gs) {
                        if (pa->i == 1) {
                            gs->cardgame->reset();
                            onmessageint(hdl, "start", gs->token);
                        }
                    }
                }

            }
               
        }
        
    }
    void open(websocketpp::connection_hdl hdl) {
        //PlayerAdapter* p = new PlayerAdapter(hdl, -1,newToken());
        //players.push_back(p);
        players.push_back(new PlayerAdapter(hdl,-1,0));
    }
    
    void removePlayer(websocketpp::connection_hdl hdl) {
        PlayerAdapter* pa = getIdlePlayer(hdl);
        pa->valid = true;
        
        if (pa->session) {
            pa->session->cardgame->removePlayer(pa->session->cardgame->getPlayerI(pa->player));
            pa->sendOthers(s, pa->session->players, jsonmessagebuilder("left", tojson("pl", pa->i)));
            pa->session->removeplayer(pa);
            if (pa->session->players.empty()) {
                vectorremove(sessions, vectorgeti(sessions, pa->session));
            }
            checkFinished(pa, pa->session);
            pa->session->cardgame->nextPlayer();
            PlayerAdapter* now = pa->session->getPlayerNow();
            if(now)
            now->send(s, jsonmessagebuilder("itsyourturn", ""));
        }
        
        pa->session = NULL;
        pa->player = NULL;
        
       
    }
    void close(websocketpp::connection_hdl hdl) {
        removePlayer(hdl);
        for (size_t i = 0; i < players.size(); i++)
        {
            if (players[i]->hdl.lock() == hdl.lock()) {
                players.erase(players.begin() + i);
                return;
            }
        }
        
    }
   
    std::vector<int> getCards(PlayerAdapter* p) {
        return p->player->getIntArr();
    }
    std::string strcards(PlayerAdapter* p) {
        std::string out = "";
        return out;
    }
    PlayerAdapter* getPlayer(websocketpp::connection_hdl hdl) {
        for (auto e : players) {
            if (e->hdl.lock() == hdl.lock()) {
                return e;
            }
        }
        return NULL;
    }
    PlayerAdapter* getPlayerByToken(int t) {
        for (auto e : players) {
            if (e->token == t) {
                return e;
            }
        }
        return NULL;
    }
   
    
    
};

void on_message(server* s, GameAdapter* gameadapter, websocketpp::connection_hdl hdl, message_ptr msg) {
    std::cout << "on_message called with hdl: " << hdl.lock().get()
        << " and message: " << msg->get_payload()
        << std::endl;

    try {
        std::string cmd = msg->get_payload();
        gameadapter->onmessage(hdl, cmd);
        /*if (cmd.find("test") == 0) {

            s->send(hdl,
                jsonmessagebuilder("update",
                    tojson("cardnow", "40") +
                    rawtojson("cards", "[0,1,2,3,4,5,6,7,8,9]")
                ), msg->get_opcode());
        }*/
        
        //s->send(hdl, s->get_con_from_hdl(hdl)->get_remote_endpoint() + msg->get_payload(), msg->get_opcode());
       // for (auto& connection : *connections) {
            //if (connection.lock() != hdl.lock()) {
         //   	s->send(connection->hdl, s->get_con_from_hdl(hdl)->get_remote_endpoint()+":"+msg->get_payload(),websocketpp::frame::opcode::text);
            				
            //}
        //}
    }
    catch (websocketpp::exception const& e) {
        std::cout << "Echo failed because: "
            << "(" << e.what() << ")" << std::endl;
    }
}


void remove_connection(GameAdapter* gameadapter,
    websocketpp::connection_hdl& hdl) {
    gameadapter->close(hdl);
    
}
    
void on_close(server* server, GameAdapter* connections,
    websocketpp::connection_hdl hdl) {
    std::cout << "on_close" << std::endl;
    remove_connection(connections, hdl);
}
void on_open(server* s, GameAdapter* gameadapter,websocketpp::connection_hdl hdl) {
    std::cout << "opened connection to:" << hdl.lock().get()<<std::endl;
    
    gameadapter->open(hdl);

}

std::string htmlpage = "";
void on_http(server* s, websocketpp::connection_hdl hdl) {
    server::connection_ptr con = s->get_con_from_hdl(hdl);
    con->append_header("Location", "https://localhost");
    //con->add_subprotocol("html");
    con->append_header("Content-Type", "text/html;charset=UTF-8");
    std::ifstream ifs("ws.html");
    
    std::stringstream buffer;
    buffer << ifs.rdbuf();
    htmlpage = buffer.str();
    ifs.close();
    con->set_body(htmlpage);
    con->set_status(websocketpp::http::status_code::ok);
}

std::string get_password() {
    return "test";
}

// See https://wiki.mozilla.org/Security/Server_Side_TLS for more details about
// the TLS modes. The code below demonstrates how to implement both the modern
enum tls_mode {
    MOZILLA_INTERMEDIATE = 1,
    MOZILLA_MODERN = 2
};

context_ptr on_tls_init(tls_mode mode, websocketpp::connection_hdl hdl) {
    namespace asio = websocketpp::lib::asio;

    std::cout << "on_tls_init called with hdl: " << hdl.lock().get() << std::endl;
    std::cout << "using TLS mode: " << (mode == MOZILLA_MODERN ? "Mozilla Modern" : "Mozilla Intermediate") << std::endl;

    context_ptr ctx = websocketpp::lib::make_shared<asio::ssl::context>(asio::ssl::context::sslv23);

    try {
        if (mode == MOZILLA_MODERN) {
            // Modern disables TLSv1
            ctx->set_options(asio::ssl::context::default_workarounds |
                asio::ssl::context::no_sslv2 |
                asio::ssl::context::no_sslv3 |
                asio::ssl::context::no_tlsv1 |
                asio::ssl::context::single_dh_use);
        }
        else {
            ctx->set_options(asio::ssl::context::default_workarounds |
                asio::ssl::context::no_sslv2 |
                asio::ssl::context::no_sslv3 |
                asio::ssl::context::single_dh_use);
        }
        ctx->set_password_callback(bind(&get_password));
        if (false) {
            ctx->use_certificate_chain_file("C:\\xampp\\apache\\conf\\ssl.crt\\server.crt");// ("server.pem");
            ctx->use_private_key_file("C:\\xampp\\apache\\conf\\ssl.key\\server.key", asio::ssl::context::pem);//("server.pem", asio::ssl::context::pem);
        }
        else {
            ctx->use_certificate_chain_file("server.pem");
            ctx->use_private_key_file("server.pem", asio::ssl::context::pem);

        }
        // Example method of generating this file:
        // `openssl dhparam -out dh.pem 2048`
        // Mozilla Intermediate suggests 1024 as the minimum size to use
        // Mozilla Modern suggests 2048 as the minimum size to use.
        ctx->use_tmp_dh_file("dh.pem");

        std::string ciphers;

        if (mode == MOZILLA_MODERN) {
            ciphers = "ECDHE-RSA-AES128-GCM-SHA256:ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-RSA-AES256-GCM-SHA384:ECDHE-ECDSA-AES256-GCM-SHA384:DHE-RSA-AES128-GCM-SHA256:DHE-DSS-AES128-GCM-SHA256:kEDH+AESGCM:ECDHE-RSA-AES128-SHA256:ECDHE-ECDSA-AES128-SHA256:ECDHE-RSA-AES128-SHA:ECDHE-ECDSA-AES128-SHA:ECDHE-RSA-AES256-SHA384:ECDHE-ECDSA-AES256-SHA384:ECDHE-RSA-AES256-SHA:ECDHE-ECDSA-AES256-SHA:DHE-RSA-AES128-SHA256:DHE-RSA-AES128-SHA:DHE-DSS-AES128-SHA256:DHE-RSA-AES256-SHA256:DHE-DSS-AES256-SHA:DHE-RSA-AES256-SHA:!aNULL:!eNULL:!EXPORT:!DES:!RC4:!3DES:!MD5:!PSK";
        }
        else {
            ciphers = "ECDHE-RSA-AES128-GCM-SHA256:ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-RSA-AES256-GCM-SHA384:ECDHE-ECDSA-AES256-GCM-SHA384:DHE-RSA-AES128-GCM-SHA256:DHE-DSS-AES128-GCM-SHA256:kEDH+AESGCM:ECDHE-RSA-AES128-SHA256:ECDHE-ECDSA-AES128-SHA256:ECDHE-RSA-AES128-SHA:ECDHE-ECDSA-AES128-SHA:ECDHE-RSA-AES256-SHA384:ECDHE-ECDSA-AES256-SHA384:ECDHE-RSA-AES256-SHA:ECDHE-ECDSA-AES256-SHA:DHE-RSA-AES128-SHA256:DHE-RSA-AES128-SHA:DHE-DSS-AES128-SHA256:DHE-RSA-AES256-SHA256:DHE-DSS-AES256-SHA:DHE-RSA-AES256-SHA:AES128-GCM-SHA256:AES256-GCM-SHA384:AES128-SHA256:AES256-SHA256:AES128-SHA:AES256-SHA:AES:CAMELLIA:DES-CBC3-SHA:!aNULL:!eNULL:!EXPORT:!DES:!RC4:!MD5:!PSK:!aECDH:!EDH-DSS-DES-CBC3-SHA:!EDH-RSA-DES-CBC3-SHA:!KRB5-DES-CBC3-SHA";
        }

        if (SSL_CTX_set_cipher_list(ctx->native_handle(), ciphers.c_str()) != 1) {
            std::cout << "Error setting cipher list" << std::endl;
        }
    }
    catch (std::exception & e) {
        std::cout << "Exception: " << e.what() << std::endl;
    }
    return ctx;
}

int main() {
    // Create a server endpoint
    
    srand(time(0));
    server echo_server;
    GameAdapter* gameadapter = new GameAdapter();
    // Initialize ASIO
    gameadapter->s = &echo_server;
    echo_server.init_asio();
printf("init asio\n");
    // Register our message handler
    echo_server.set_open_handler(websocketpp::lib::bind(&on_open, &echo_server, gameadapter, ::_1));
    
    		echo_server.set_close_handler(
    			websocketpp::lib::bind(&on_close, &echo_server, gameadapter, ::_1));
    		
    echo_server.set_message_handler(bind(&on_message, &echo_server, gameadapter, ::_1, ::_2));
    echo_server.set_http_handler(bind(&on_http, &echo_server, ::_1));
//    echo_server.set_tls_init_handler(bind(&on_tls_init, MOZILLA_INTERMEDIATE, ::_1));
   printf("set handlers\n");
    // Listen on port 9002
    echo_server.listen(9002);
printf("set listen port\n");
    // Start the server accept loop
    echo_server.start_accept();
printf("start accept\n");
    // Start the ASIO io_service run loop
    echo_server.run();

}
