//#pragma once
//#include<vector>
//#include<string>
//enum Cards {
//	CARD_1 = 0,
//	CARD_2 = 1,
//	CARD_3 = 2,
//	CARD_4 = 3,
//	CARD_5 = 4,
//	CARD_6 = 5,
//	CARD_7 = 6,
//	CARD_8 = 7,
//	CARD_9 = 8,
//	CARD_SKIP = 9,
//	CARD_REVERSE = 10,
//	CARD_DRAWTWO = 11,
//	CARD_WILDDRAW = 12,
//
//};
//const int COLORSIZE = 13;
//class Card {
//public:
//	int i = 0;
//	inline Card(int i) {
//		this->i = i;
//	}
//	Card(int col,int nmb) {
//		this->i = col* COLORSIZE +nmb;
//	}
//	int getColor() {
//		return i / COLORSIZE;
//	}
//	int getNumber() {
//		return i % COLORSIZE;
//	}
//	bool sameNumber(Card* c) {
//		return getNumber() == c->getNumber();
//	}
//	bool sameColor(Card* c) {
//		return getColor() == c->getColor();
//	}
//	std::string toStr() {
//		std::string out = "";
//		int n = getNumber();
//		if (n < CARD_WILDDRAW) {
//			out += "rgby"[getColor()];
//		}
//		if (0 < n < 9) {
//			out += std::to_string((n + 1));
//		}
//		else
//		if (n == CARD_SKIP) {
//			out += " Skip";
//		}
//		else
//		if (n == CARD_REVERSE) {
//			out += " Reverse";
//		}
//		else
//		if (n == CARD_DRAWTWO) {
//			out += " +2";
//		}
//		else
//		if (n == CARD_WILDDRAW) {
//			out += " +4";
//		}
//		return out;
//	}
//	
//
//};
//class CardGame
//{
//public:
//	std::deque<Card*> cards;
//	int cardstartcount = 5;
//	std::vector < std::vector<Card*>> players;
//	int playernow=0;
//	bool isreverse = false;
//	Card* cardnow;
//	int drawcards = 0;
//	CardGame() {
//		for (int i = 0; i < 52; i++)
//		{
//			cards.push_back(new Card(i));
//		}
//		mix();
//		cardnow = drawCard();
//	}
//	void mix() {
//		for (size_t i = 0; i < cards.size(); i++)
//		{
//			Card* c = cards[i];
//			size_t r = rand() % cards.size();
//			cards[i] = cards[r];
//			cards[r] = c;
//		}
//	}
//	Card* drawCard() {
//		Card* c=cards[cards.size()-1];
//		cards.pop_back();
//		return c;
//	}
//	bool playCard(Card* c) {
//
//		cards.push_front(cardnow);
//		cardnow = c;
//		return true;
//	}
//	int addplayer() {
//		std::vector<Card*> pcards;
//		if (cards.size() > cardstartcount) {
//			for (int i = 0; i < cardstartcount; i++) {
//				pcards.push_back(drawCard());
//			}
//			players.push_back(pcards);
//			return players.size() - 1;
//		}
//		return -1;
//	}
//	void nextPlayer() {
//		if (isreverse) {
//			playernow = (playernow - 1) % players.size();
//		}
//		else {
//			playernow = (playernow + 1) % players.size();
//		}
//	}
//	bool canPlayCard(size_t p, size_t c) {
//		Card* psc = players[p][c];
//		if (psc->getNumber() == CARD_WILDDRAW) {
//			return true;
//			
//		}
//		else {
//			if (psc->sameNumber(cardnow)) {
//				return true;
//				
//			}
//			if (psc->sameColor(cardnow)) {
//				return true;
//			}
//		}
//		return false;
//	}
//
//};
#pragma once
#include<vector>
#include<string>
#include<deque>
using namespace std;
const char* colors[] = { "red", "green", "blue", "yellow" };
const char* special[] = { "(/)","<->","+2" };
class Card {
public:
	int id;
	Card(int id) {
		this->id = id;
	}
	int getNumber() {
		return id % 13;
	}
	int getColor() {
		return id / 13;
	}

	string toStr() {
		int n = getNumber();
		if (n < 12) {
			if (n < 9) {
				return  string(colors[getColor()]) + " " + to_string(n);
			}

			return  string(colors[getColor()]) + " " + string(special[n - 9]);
		}
		return "+4";
	}
};
class Player {
public:
	vector<Card*> deck;
	int pid = 0;
	void addCard(Card* c) {
		deck.push_back(c);
	}
	Card* removeCard(int i) {
		Card* c = deck[i];
		deck.erase(deck.begin() + i);
		return c;
	}
	int getCardI(Card* c) {
		for (size_t i = 0; i < deck.size(); i++)
		{
			if (c == deck[i])return i;
		}
		return NULL;
	}
	Card* getCard(int i) {
		return deck[i];
	}
	string toStr() {
		string out = "";
		for (auto c : deck) {
			out += c->toStr() + ",";
		}
		return out;
	}
	vector<int> getIntArr() {
		vector<int> out;
		for (auto e : deck) {
			out.push_back(e->id);
		}
		return out;
	}
	string strIntArr() {
		string out;
		for (auto e : deck) {
			out+=to_string(e->id)+",";
		}
		return out;
	}
};
void print(string str) {
	printf("%s\n", str.c_str());
}
class CardGame
{
public:
	bool opentojoin = true;
	vector<Player*> players;
	Card* cardnow;
	deque<Card*> cardstack;
	deque<Card*> laydownstack;
	int playernow = 1;

	bool reverse = false;
	int drawcount = 0;
	bool playerchoosing = false;
	int chosencolor = -1;
	bool finished = false;
	bool gameend = false;
	CardGame() {

	}
	void reset() {
		opentojoin = true;
		cardstack.clear();
		cardnow = NULL;
		for (auto p : players) {
			p->deck.clear();
		}
		players.clear();
		playernow = 1;
		drawcount = 0;
		playerchoosing = false;
		chosencolor = -1;
		finished = false;
	}
	Player* addPlayer() {
		if (getPlayerCount() < 10) {
			Player* p = new Player();
			players.push_back(p);
			p->pid = players.size();
			return p;
		}
		else {
			print("round is full");
		}
		return NULL;
	}
	int getPlayerCount() {
		return players.size();
	}
	int getPlayerI(Player* p) {
		for (size_t i = 0; i < players.size(); i++)
		{
			if (p == players[i]) {
				return i;
			}
		}
		return -1;
	}
	Player* getPlayerNow() {
		return players.empty()?NULL:players[playernow%players.size()];
	}
	Card* drawCard() {
		if (cardstack.empty()) {
			while (!laydownstack.empty()) {
				cardstack.push_front(laydownstack.back());
				laydownstack.pop_back();
				

			}
			mixup();
			printf("mixed\n");
		}
		if (cardstack.empty()) { printf("cardstack empty\n"); return NULL; }
		Card* c = cardstack.back();
		cardstack.pop_back();
		return c;
	}
	void notifyPlayer() {
		print("it's player " + to_string(playernow) + " turn");
	}
	void mixup() {
		Card* swap;
		size_t random = 0;
		for (size_t i = 0; i < cardstack.size(); i++) {
			random = rand() % cardstack.size();
			swap = cardstack[i];
			cardstack[i] = cardstack[random];
			cardstack[random] = swap;
		}
	}
	string start(int startcardcount) {
		if (getPlayerCount() > 1) {
			for (int i = 0; i < 52; i++) {
				cardstack.push_back(new Card(i));
			}
			mixup();
			
			/*int startcardcount = (
				getPlayerCount() < 8 ? 7 :
				(getPlayerCount() < 9 ? 6 : 5)
				);
			startcardcount = 3;*/
			for (auto p : players) {
				for (size_t i = 0; i < startcardcount; i++) {
					p->addCard(drawCard());
				}

			}
			cardnow = drawCard();
			playCard(players[playernow], cardnow);
			playernow = 1;
			notifyPlayer();
			finished = false;
			opentojoin = false;
			return "";
		}
		else {
			return "fewplayers";
		}
	}
	int nextPlayer() {
		if (reverse) {
			playernow = (playernow + players.size() - 1) % players.size();
			
		}
		else {
			playernow = (playernow + 1) % players.size();
		}
		return 0;
	}
	void chooseColor(int col) {
		if (col >= 0 && 4 > col) {
			if (players[playernow]->deck.empty()) {
				removePlayer(playernow);
			}
			nextPlayer();
			notifyPlayer();
			chosencolor = col;
			playerchoosing = false;
			
		}
	}
	void playCard(Player* p, Card* c) {
		if (c->getNumber() == 9) {
			nextPlayer();

		}
		if (c->getNumber() == 10) {
			reverse = !reverse;
			if (players.size() == 2) {
				nextPlayer();
			}
		}
		if (c->getNumber() == 11) {
			drawcount += 2;
		}
		else {
			while (drawcount > 0) {
				p->addCard(drawCard());
				drawcount--;
			}
		}
		if (c->getNumber() == 12) {
			drawcount += 4;
			playerchoosing = true;
		}
		if (!playerchoosing) {
			nextPlayer();
			notifyPlayer();
		}

	}
	void removePlayer(int playeri) {
		if (playeri >= 0 && playeri < players.size()) {
			players.erase(players.begin() + playeri);
			if (players.size() == 1) {
				gameend = true;
			}
			if (players.empty()) {
				finished = true;
			}
			else {
				nextPlayer();
			}
		}
		
		return;
	}
	bool extradraw = false;
	void playerDraw(int playeri) {
		
			if (drawcount > 0) {
				while (drawcount > 0) {
					Card* nc = drawCard();
					if (nc != NULL)players[playeri]->addCard(nc);
					drawcount--;
				}
			}
			else {
				if (!extradraw) {
					Card* nc = drawCard();
					if (nc != NULL)players[playeri]->addCard(nc);
					extradraw = true;
				}
				else {
					if (!playerchoosing) {
						nextPlayer();
						notifyPlayer();
					}
					extradraw = false;
				}
			}
	}
	Card* playCard(int playeri, int cardi) {
		if (!playerchoosing) {
			if (playeri == playernow) {
				if (cardi >= 0) {
					if (cardi < players[playeri]->deck.size()) {

						print("playing card " + players[playeri]->deck[cardi]->toStr());
						if (canPlayCard(players[playeri]->deck[cardi])) {
							laydownstack.push_front(cardnow);
							cardnow = players[playeri]->removeCard(cardi);
							playCard(players[playeri], cardnow);
							if (players[playeri]->deck.empty()) {
								print("player " + to_string(players[playeri]->pid) + " has won!");
								if(!playerchoosing)
								removePlayer(playeri);
							}
							extradraw = false;
							return cardnow;
						}
						else {
							print("cant play card");
							playerDraw(playeri);
							
						}
						
					}
					else {
						if (cardi == players[playeri]->deck.size()) {
							playerDraw(playeri);
							
						}
					}
				}
			}
			else {
				print("player " + to_string(playeri) + " is not found");
				return NULL;
			}
		}
		return NULL;
	}
	Card* playCard(Player* p, int cardi) {
		return playCard(getPlayerI(p),cardi);
		
	}
	bool canPlayCard(Card* card) {
		int c = card->getNumber();
		int n = cardnow->getNumber();
		int colorc = card->getColor();
		int colorn = cardnow->getColor();
		if (c == 12 && n == 12) {
			return false;
		}
		if (c == 12 && n != 12) {
			return true;
		}else if (n == 12) {
			if (chosencolor == colorc)return true;
		}
		else if (c == n) {
			return true;
		}
		else if (colorc == colorn) {
			return true;
		}
		return false;
	}
	vector<Card*> getPossible(int p) {
		vector<Card*> out;
		for (auto c : players[p]->deck) {
			if (canPlayCard(c)) {
				out.push_back(c);
			}
		}
		return out;
	}
	bool canPlayerPlay(int p) {
		if(!players.empty())if(p>=0&&p<players.size())
		for (auto c : players[p]->deck) {
			if (!canPlayCard(c)) {
				return true;
			}
		}
		return false;
	}
};

