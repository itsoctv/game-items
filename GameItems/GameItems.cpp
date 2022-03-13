#include <iostream>
#include <vector>
#include <utility>
#include <string>

using namespace std;

enum TYPE { NT, MELEE, RANGE, ARMOUR };
enum RARITY { NR, COMMON, RARE, EPIC };
enum MOD_TYPE { NMT, DAMAGE_BUFF, PROTECTION_BUFF, SPEED_BUFF};

class Modifier;

//ITEM
class Item {
protected:
	string ident;
	TYPE type;
	size_t level;
	RARITY rarity;
	string modified;
public:
	Item(string ident, TYPE type, size_t level, RARITY rarity) : ident(ident), type(type), level(level), rarity(rarity), modified("no modifiers") {};
	~Item() {};

	Item(const Item&) = delete;
	Item& operator=(const Item&) = delete;

	Item(Item&&) = delete;
	Item& operator=(Item&&) = delete;
	
	TYPE getType() const { return type; }
	RARITY getRarity() const { return rarity; }
	size_t getLevel() const { return level; }

	void setModifier(const string& str) { modified = str; }

	virtual void damage_mod(double) = 0;
	virtual void speed_mod(double) = 0;
	virtual void protection_mod(double) = 0;

	//DEBUG
	virtual void info() = 0;
};

//WEAPON
class Weapon : public Item {
	double damage;
	double speed;
public:
	Weapon() = delete;
	Weapon(string ident, TYPE type, size_t level, RARITY rarity, double damage, double speed) : Item(ident, type, level, rarity), damage(damage), speed(speed) {};

	//damage/speed modification
	void damage_mod(double val) override { damage += val; }
	void speed_mod(double val) override { speed += val; }
	void protection_mod(double val) {};

	//DEBUG
	void info() override {
		cout << "ident: " << ident << ", type: weapon, level: " << level << ", rarity: ";
		switch (rarity) {
		case 1: { cout << "common, "; break; }
		case 2: { cout << "rare, "; break; }
		case 3: { cout << "epic! "; break; }
		}
		cout << "damage: " << damage << ", speed: " << speed << ", modified: " << modified << '\n';
	}
};

//ARMOUR
class Armour : public Item {
	double protection;
public:
	Armour() = delete;
	Armour(string ident, TYPE type, size_t level, RARITY rarity, double protection) : Item(ident, type, level, rarity), protection(protection) {};
	
	//protection modification
	void damage_mod(double val)  { }
	void speed_mod(double val)  { }
	void protection_mod(double val) override { protection += val; }

	//DEBUG
	void info() override {
		cout << "Ident: " << ident << ", type: armour, level: " << level << ", rarity: ";
		switch (rarity) {
		case 1: { cout << "common, "; break; }
		case 2: { cout << "rare, "; break; }
		case 3: { cout << "epic! "; break; }
		}
		cout << "protection: " << protection << ", modified: " << modified << '\n';
	}
};

//MODIFIER
struct Filter {
	size_t level;
	RARITY rarity;
	TYPE type;
	Filter() : level(0), rarity(NR), type(NT) {};
};

class Modifier {
	string ident;
	vector<Filter> filters;
	MOD_TYPE mod_type;
	double value;
public:
	Modifier() = delete;
	Modifier(string ident, MOD_TYPE mod_type, double value) : ident(ident), mod_type(mod_type), value(value) {};

	Modifier(const Modifier&) = delete;
	Modifier& operator=(const Modifier&) = delete;

	Modifier(Modifier&&) = delete;
	Modifier& operator=(Modifier&&) = delete;

	void add_filter(const Filter& fil) { filters.push_back(fil); }
	bool check_filters(const shared_ptr<Item>&) const;
	MOD_TYPE getMod_type() const { return mod_type; }
	double getValue() const { return value; }
	string getIdent() const { return ident; }

	//DEBUG
	void info() {
		cout << "ident: " << ident << ", filters: ";
		for (const auto& x : filters)
		{
			cout << "{level >= " << x.level << ", rarity: ";
			switch(x.rarity) {
			case NR: {cout << "<NONE>, "; break; }
			case COMMON: {cout << "common, "; break; }
			case RARE: {cout << "rare, "; break; }
			case EPIC: {cout << "epic!, "; break; }
			}
			switch (x.type) {
			case NT: {cout << "<NONE>"; break; }
			case RANGE: {cout << "range"; break; }
			case MELEE: {cout << "melee"; break; }
			case ARMOUR: {cout << "armour"; break; }
			}
			cout << "}, ";
		}
		cout << "type: ";
		switch (mod_type) {
		case NMT: { cout << "<NONE>, "; break; }
		case DAMAGE_BUFF: { cout << "DamageBuff, "; break; }
		case PROTECTION_BUFF: { cout << "ProtectionBuff, "; break; }
		case SPEED_BUFF: { cout << "SpeedBuff, "; break; }
		}
		cout << "value: " << value << '\n';
	}
};

bool Modifier::check_filters(const shared_ptr<Item>& item) const {
	bool check = false;
	for (const auto& x : filters) {
		if (item->getLevel() < x.level) continue;
		if ((item->getRarity() != x.rarity) && (x.rarity != NR)) continue;
		if ((item->getType() != x.type) && (x.type != NT)) continue;
		check = true;
	}
	return check;
}

class ModifierList {
	vector<shared_ptr<Modifier>> mods;
public:
	ModifierList() = default;

	ModifierList(const ModifierList&) = delete;
	ModifierList& operator=(const ModifierList&) = delete;

	ModifierList(ModifierList&&) = delete;
	ModifierList& operator=(ModifierList&&) = delete;

	void add_modifier(const shared_ptr<Modifier>& m) { mods.push_back(m); }

	auto begin() { return mods.begin(); }
	auto end() { return mods.end(); }

	//DEBUG
	void show_modifiers() {
		for (const auto& x : mods)
			x->info();
	}
};

//INVENTORY
class Inventory {
	vector<shared_ptr<Item>> inv;
public:

	Inventory() = default;

	Inventory(const Inventory&) = delete;
	Inventory& operator=(const Inventory&) = delete;

	Inventory(Inventory&&) = delete;
	Inventory& operator=(Inventory&&) = delete;

	void add_item(const shared_ptr<Item>& i) { inv.push_back(i); }
	void use_modifier(const Modifier&);

	friend void init_inventory(Inventory&);

	//DEBUG
	void show_items() {
		for (const auto& x : inv) {
			x->info();
		}
	}
};

void Inventory::use_modifier(const Modifier& mod) {
	for (auto& x : inv) {
		//if one of filters matchs, use modifier
		if(mod.check_filters(x))	
			switch (mod.getMod_type()) {
			case DAMAGE_BUFF: { x->damage_mod(mod.getValue()); x->setModifier(mod.getIdent() + " (+" + to_string(mod.getValue()) + " DMG)"); break; }
			case SPEED_BUFF: { x->speed_mod(mod.getValue()); x->setModifier(mod.getIdent() + " (+" + to_string(mod.getValue()) + " SPEED)"); break; }
			case PROTECTION_BUFF: { x->protection_mod(mod.getValue()); x->setModifier(mod.getIdent() + " (+" + to_string(mod.getValue()) + " PROTECTION)"); break; }
			}
	}
}

//INIT INVENTORY (INPUT)
void init_inventory(Inventory& inv) {
	string ident;
	TYPE type;
	size_t level;
	RARITY rarity;
	double damage;
	double speed;
	double protection; //armour only

	ident = "axe_01"; type = MELEE; level = 3; rarity = COMMON; damage = 12.0; speed = 0.9;
	inv.add_item(make_shared<Weapon>(ident, type, level, rarity, damage, speed));

	ident = "revolver_01"; type = RANGE; level = 5; rarity = COMMON; damage = 42.0; speed = 2.0;
	inv.add_item(make_shared<Weapon>(ident, type, level, rarity, damage, speed));

	ident = "revolver_02"; type = RANGE; level = 3; rarity = RARE; damage = 50.0; speed = 2.1;
	inv.add_item(make_shared<Weapon>(ident, type, level, rarity, damage, speed));

	ident = "machinegun_01"; type = RANGE; level = 5; rarity = EPIC; damage = 83.1; speed = 10.0;
	inv.add_item(make_shared<Weapon>(ident, type, level, rarity, damage, speed));

	ident = "jacket_01"; type = ARMOUR; level = 2; rarity = COMMON; protection = 2.0;
	inv.add_item(make_shared<Armour>(ident, type, level, rarity, protection));

	ident = "bulletprof_vest_01"; type = ARMOUR; level = 5; rarity = RARE; protection = 30.0;
	inv.add_item(make_shared<Armour>(ident, type, level, rarity, protection));
}

//INIT MODIFIERS (INPUT)
void init_modifier_list(ModifierList& mods) {
	string ident;
	Filter fil;
	MOD_TYPE mod_type;
	double value;

	ident = "eagle_eye"; fil.level = 4; fil.type = RANGE; mod_type = DAMAGE_BUFF; value = 10;
	auto aboba = make_shared<Modifier>(ident, mod_type, value);
	//add 1 and more filters after base initialization
	aboba->add_filter(fil);
	mods.add_modifier(aboba);

	ident = "donatium_steel"; fil.level = 0; fil.type = NT; fil.rarity = RARE; mod_type = PROTECTION_BUFF; value = 15;
	aboba = make_shared<Modifier>(ident, mod_type, value);
	aboba->add_filter(fil);
	mods.add_modifier(aboba);
	
	ident = "rage_drink"; fil.level = 0; fil.type = RANGE; mod_type = SPEED_BUFF; value = 0.4;
	aboba = make_shared<Modifier>(ident, mod_type, value);
	aboba->add_filter(fil);
	fil.type = MELEE;
	aboba->add_filter(fil);
	mods.add_modifier(aboba);
}

//LET'S MODIFY!
void use_all_modifiers_with_all_inventory_items_fofun(Inventory& inv, ModifierList& mod_list) {
	for (const auto& x : mod_list) {
		inv.use_modifier(*x);
	}
}

int main()
{
	cout << "~~~~~~~~ INVENTORY ~~~~~~~~\n";
	Inventory inv;
	init_inventory(inv);
	inv.show_items();

	cout << "\n~~~~~~~~ MODIFIERS ~~~~~~~~\n";
	ModifierList mod_list;
	init_modifier_list(mod_list);
	mod_list.show_modifiers();

	cout << "\nUsing all modifiers...\n";
	use_all_modifiers_with_all_inventory_items_fofun(inv, mod_list);
	inv.show_items();

	system("pause");
}

