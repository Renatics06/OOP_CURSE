#define NOMINMAX

#include <iostream>
#include <windows.h>
#include <vector>
#include <memory>
#include <string>
#include <algorithm>
#include <iomanip>
#include <fstream>
#include <functional>
#include <unordered_map>
#include <cctype>

using namespace std;

namespace Color {
	enum TextColor {

		DEFAULT = 7,
		RED = 4,
		GREEN = 2,
		BLUE = 1,
		YELLOW = 6,
		MAGENTA = 5,
		CYAN = 3,
		WHITE = 15

	};

	void setTextColor(TextColor color) {
		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleTextAttribute(hConsole, color);
	}

	void resetTextColor() {
		setTextColor(DEFAULT);
	}
}

class Product {
protected:
	string name;
	double price;
public:
	Product(string& n, double p) : 
		name(n), price(p) {}
	string getName() {
		return this->name;
	}
	~Product() = default;
	double getPrice() {
		return this->price;
	}

	bool operator<(Product& other) {
		return name < other.name;
	}

	bool operator>(Product& other) {
		return name > other.name;
	}

	friend ostream& operator<<(ostream& os, Product& product) {
		os << product.name << "," << product.price;
		return os;
	}

	friend istream& operator>>(istream& is, Product& product) {

		string line;

		if (!getline(is, line)) {
			is.setstate(ios::failbit);
			return is;
		}
		size_t pos1 = line.find(',');
		size_t pos2 = line.find(',', pos1 + 1);
		if (pos1 == string::npos || pos2 == string::npos) {
			is.setstate(ios::failbit);
			return is;
		}
		product.name = line.substr(0, pos1);
		try {
			product.price = stod(line.substr(pos2 + 1));
		}
		catch (...) {
			is.setstate(ios::failbit);
		}

		return is;

	}

	virtual void calculateCost() = 0;

	virtual void display() {
		cout << "| " << setw(18) << left << this->name
			<< "| " << setw(14) << left << this->price;
	}
};

class CommonProduct : public Product {
public:
	CommonProduct(string& n, double p) : 
		Product(n, p) {}
	void calculateCost() override {
		price *= 1.05;
	}
};

class SpecificProduct : public Product {
public:
	SpecificProduct(string& n, double p) : Product(n, p) {}

	void calculateCost() override {
		price *= 1.3;
	}
};


class User {

	bool admin;
	string username;
	size_t passwordHash;

public:
	User() : 
		username(""), passwordHash(0), admin(false) {}

	User(string& user, string& pass, bool isAdmin)
		: username(user), passwordHash(hash<string>{}(pass)), admin(isAdmin) {
	}

	bool isAdmin() {
		return admin;
	}

	string getName() {
		return this->username;
	}

	bool authorize(string& pass) {
		return passwordHash == hash<string>{}(pass);
	}

	void addUser() {

		ofstream file("passwords.txt", ios::app);

		if (!file) {
			throw runtime_error("Не удалось открыть файл для записи");
		}

		file << username << "," << passwordHash << "," << admin << endl;

		file.close();
	}

	static User loadFromFile(string& user) {

		ifstream file("passwords.txt");

		if (!file) {
			throw runtime_error("Не удалось открыть файл для чтения");
		}

		string line;

		while (getline(file, line)) {

			size_t pos1 = line.find(',');
			size_t pos2 = line.find(',', pos1 + 1);

			if (pos1 == string::npos || pos2 == string::npos) {
				continue;
			}

			string storedUser = line.substr(0, pos1);

			if (storedUser == user) {
				size_t hash = stoull(line.substr(pos1 + 1, pos2 - pos1 - 1));
				bool isAdmin = stoi(line.substr(pos2 + 1));
				User loadedUser;
				loadedUser.username = storedUser;
				loadedUser.passwordHash = hash;
				loadedUser.admin = isAdmin;
				return loadedUser;
			}
		}

		return User();

	}


	static void deleteUser(string& user) {

		ifstream file("passwords.txt");

		if (!file) {
			throw runtime_error("Не удалось открыть файл для чтения");
		}

		vector<string> lines;
		string line;

		while (getline(file, line)) {
			if (line.find(user + ",") != 0) {
				lines.push_back(line);
			}
		}

		file.close();

		ofstream outFile("passwords.txt", ios::trunc);

		if (!outFile) {
			throw runtime_error("Не удалось открыть файл для записи");
		}

		for (const auto& l : lines) {
			outFile << l << endl;
		}

		outFile.close();

	}
};


class Order {

	vector<unique_ptr<CommonProduct>> products;
	vector<unique_ptr<SpecificProduct>> specificproducts;

public:

	void setAllProducts(vector<unique_ptr<CommonProduct>>& products, vector<unique_ptr<SpecificProduct>>& specificproducts) {
		this->products.clear();
		for (const auto& product : products) {
			this->products.push_back(make_unique<CommonProduct>(*product));
		}
		this->specificproducts.clear();
		for (const auto& specificproduct : specificproducts) {
			this->specificproducts.push_back(make_unique<SpecificProduct>(*specificproduct));
		}
	}

	template<typename T>

	void addProduct(unique_ptr<T> product) {
		products.push_back(move(product));
	}

	void generateReport(string& filename) {

		ofstream file(filename);

		if (!file) throw runtime_error("Не удалось открыть файл");

		for (auto& product : products) {
			product->display();
			file << product->getName() << "," << product->getPrice() << endl;
		}

		file << "/" << endl;

		for (auto& specificproduct : products) {
			specificproduct->display();
			file << specificproduct->getName() << "," << specificproduct->getPrice() << endl;
		}
	}
};

template<typename T>
void addItem(vector<unique_ptr<T>>& items, unique_ptr<T> item) {
	items.push_back(move(item));
}

template<typename T>
void deleteItem(vector<unique_ptr<T>>& items, string& name) {
	items.erase(remove_if(items.begin(), items.end(),
		[&name](unique_ptr<T>& item) {
			return item->getName() == name;
		}), items.end());
}

template<typename T>
auto findItem(vector<unique_ptr<T>>& products, string& name, bool isSpecific = false) {
	auto it = find_if(products.begin(), products.end(),
		[&name](unique_ptr<T>& product) {
			return product->getName() == name;
		});

	if (it != products.end()) {
		cout << "===================================================" << endl;
		cout << "| Продукт найден                                  |" << endl;
		cout << "---------------------------------------------------" << endl;
		cout << "| Имя               | Цена          | Специальный |" << endl;
		cout << "---------------------------------------------------" << endl;
		(*it)->display();
		cout << "| " << setw(12) << (isSpecific ? "+" : "-") << "|" << endl;
		cout << "===================================================" << endl;
	}
	else {
		Color::setTextColor(Color::RED);
		cout << "Продукт \"" << name << "\" не найден.\n";
		Color::resetTextColor();
	}

	return it;

}


template<typename T>
bool redactItem(vector<unique_ptr<T>>& items, string& name, string newName, double newPrice) {

	auto it = findItem(items, name);

	if (it == items.end()) {
		Color::setTextColor(Color::RED);
		cout << "Ошибка: продукт с именем \"" << name << "\" не найден.\n";
		Color::resetTextColor();
		return false;
	}

	cout << "Продукт найден, выполняется обновление...\n";
	*it = make_unique<T>(newName, newPrice);
	cout << "Продукт обновлен.\n";

	return true;
}


template<typename T>
void viewItems(vector<unique_ptr<T>>& items) {

	for (auto& item : items) {
		item->display();
		if (dynamic_cast<SpecificProduct*>(item.get())) {
			cout << "| " << setw(12) << "+" << "|" << endl;
		}
		else {
			cout << "| " << setw(12) << "-" << "|" << endl;
		}
	}
}

void displayMenu() {

	Color::setTextColor(Color::BLUE);
	cout << "1. Добавить деталь\n";
	cout << "2. Просмотреть детали\n";
	cout << "3. Редактировать деталь\n";
	cout << "4. Удалить деталь\n";
	cout << "5. Сгенерировать отчет\n";
	cout << "6. Поиск\n";
	cout << "7. Сортировка\n";
	cout << "8. Фильтрация\n";
	cout << "9. Итоговая стоимость продукта\n";
	Color::setTextColor(Color::DEFAULT);
}

void preDisplayMenu() {

	Color::setTextColor(Color::BLUE);
	cout << "1. Войти\n";
	cout << "0. Выход\n";
	Color::setTextColor(Color::DEFAULT);
}

void aDisplayMenu() {

	Color::setTextColor(Color::MAGENTA);
	cout << "a. Добавить юзеров\n";
	cout << "b. Удалить юзеров\n";
	cout << "c. Просмотреть всю инфу по юзерам\n";
	Color::setTextColor(Color::DEFAULT);
}

User loginUser(string& username, bool& tr) {

	string password;

	cout << "Введите пароль: ";
	cin >> password;

	User user = User::loadFromFile(username);

	if (user.getName().empty()) {
		Color::setTextColor(Color::RED);
		cout << "Ошибка: Пользователь \"" << username << "\" не найден.\n";
		Color::resetTextColor();
		return User();
	}

	if (user.authorize(password)) {

		cout << "Успешный вход!\n";
		tr = true;
		if (user.isAdmin()) {
			cout << "Вы вошли как администратор.\n";
		}
		else {
			cout << "Вы вошли как обычный пользователь.\n";
		}
		return user;
	}
	else {

		Color::setTextColor(Color::RED);
		cout << "Неверный пароль.\n";
		Color::resetTextColor();
		return User();
	}
}


template<typename T>
void addProduct(vector<unique_ptr<T>>& products) {

	string name;
	double price;
	cout << "Введите название продукта: ";
	cin >> name;
	cout << "Введите цену продукта: ";
	while (!(cin >> price)) {
		Color::setTextColor(Color::RED);
		cout << "Ошибка: цена должна быть числом. Попробуйте снова: ";
		Color::resetTextColor();
		cin.clear();
		cin.ignore(numeric_limits<streamsize>::max(), '\n'); 
	}

	products.push_back(make_unique<T>(name, price));
	cout << "Продукт добавлен.\n";
}

template<typename T>
double calculateTotalCost(vector<unique_ptr<T>>& items) {
	double total = 0.0;
	for (auto& item : items) {
		total += item->getPrice();
	}
	return total;
}

int extractNumber(string& s, size_t& pos) {
	int num = 0;
	while (pos < s.size() && isdigit(s[pos])) {
		num = num * 10 + (s[pos] - '0');
		++pos;
	}
	return num;
}

bool naturalCompare(const string& a, const string& b) {
	size_t i = 0, j = 0;

	while (i < a.size() && j < b.size()) {
		if (isdigit(a[i]) && isdigit(b[j])) {
			int numA = extractNumber(const_cast<string&>(a), i);
			int numB = extractNumber(const_cast<string&>(b), j);
			if (numA != numB) {
				return numA < numB;
			}
		}
		else {
			if (a[i] != b[j]) {
				return a[i] < b[j];
			}
			++i;
			++j;
		}
	}

	return a.size() < b.size();
}


template<typename T>
void sortProducts(vector<unique_ptr<T>>& products) {
	sort(products.begin(), products.end(),
		[](const unique_ptr<T>& a, const unique_ptr<T>& b) {
			return naturalCompare(a->getName(), b->getName());
		});
	cout << "Продукты отсортированы по имени.\n";
}

static void viewAllUsers() {
	ifstream file("passwords.txt");
	if (!file) {
		throw runtime_error("Не удалось открыть файл для чтения");
	}

	cout << "Список пользователей:\n";
	cout << "======================================================================\n";
	cout << "| Имя пользователя | Хэш пароля                              | Админ |\n";
	cout << "----------------------------------------------------------------------\n";

	string line;
	while (getline(file, line)) {
		size_t pos1 = line.find(',');
		size_t pos2 = line.find(',', pos1 + 1);

		if (pos1 == string::npos || pos2 == string::npos) {
			continue;
		}

		string username = line.substr(0, pos1);
		size_t hash = stoull(line.substr(pos1 + 1, pos2 - pos1 - 1));
		bool isAdmin = stoi(line.substr(pos2 + 1));

		cout << "| " << setw(16) << left << username
			<< "| " << setw(41) << left << hash
			<< "| " << (isAdmin ? "Да " : "Нет") << "   |\n";
	}

	cout << "======================================================================\n";
}

void run() {
	User user;
	vector<unique_ptr<CommonProduct>> products;
	vector<unique_ptr<SpecificProduct>> specificproducts;

	Order order;
	
	bool cheсk1, cheсk2;
	string newName;
	double newPrice;
	char choice;
	bool isUser = false;
	char zn;
	string filename;
	string name;


	while (1) {
		cout << "Выберите опцию: \n";
		preDisplayMenu();
		cin >> choice;
		switch (choice) {
		case '1': {
			string username;
			cout << "Введите имя пользователя: ";
			cin >> username;
			user = loginUser(username, isUser);
			break;
		}
		case '0':{
			cout << "Выход...\n";
			return;
		}
		default:{
			cout << "Некорректная опция. Пожалуйста, попробуйте снова.\n";
			break;
		}
		}
		while (isUser) {
			displayMenu();
			if (user.isAdmin()) { aDisplayMenu(); };
			Color::setTextColor(Color::GREEN);
			cout << "0.Выйти из учетной записи\n";
			Color::setTextColor(Color::DEFAULT);
			cout << "Выберите опцию: ";
			cin >> choice;
			switch (choice) {
			case '1':
				while (1) {
					cout << "Это спец. деталь?(+/-)"; cin >> zn;
					if (zn == '-') { addProduct(products); break; }
					else if (zn == '+') { addProduct(specificproducts); break; }
					else {
						Color::setTextColor(Color::RED);
						cout << "Неверный символ!" << endl;
						Color::setTextColor(Color::DEFAULT);
					}
				}
				break;
			case '2':
				cout << "===================================================" << endl;
				cout << "| Части детали                                    |" << endl;
				cout << "---------------------------------------------------" << endl;
				cout << "| Имя               | Цена          | Специальный |" << endl;
				cout << "---------------------------------------------------" << endl;
				viewItems(products);
				viewItems(specificproducts);
				cout << "===================================================" << endl;
				break;
			case '3':
				cout << "Введите название продукта для редактирования: ";
				cin >> name;
				cout << "Введите новое название продукта: ";
				cin >> newName;
				cout << "Введите новую цену продукта: ";
				cin >> newPrice;
				cheсk1 = redactItem(products, name, newName, newPrice);
				cheсk2 = redactItem(specificproducts, name, newName, newPrice);
				if(cheсk1 || cheсk2){
					cout << "Продукт обновлен, если он существовал.\n";
				}
				break;
			case '4':
				cout << "Введите название продукта для удаления: ";
				cin >> name;
				deleteItem(products, name);
				deleteItem(specificproducts, name);
				cout << "Продукт удален, если он существовал.\n";
				break;
			case '5':
				cout << "Введите имя файла для отчета: ";
				cin >> filename;
				order.setAllProducts(products, specificproducts);
				order.generateReport(filename);
				cout << "Отчет сгенерирован.\n";
				break;
			case '6': {
				cout << "Введите название продукта для поиска: ";
				cin >> name;

				auto it = findItem(products, name, false);

				if (it == products.end()) {
					auto its = findItem(specificproducts, name, true);
				}
				break;
			}

			case '7': {
				sortProducts(products);
				sortProducts(specificproducts);
				break;
			}
			case '8': {
				cout << "Какой вид продукта вывезти спец. или обычный?(+/-) "; cin >> zn;

				cout << "===================================================" << endl;
				cout << "| Детали                                          |" << endl;
				cout << "---------------------------------------------------" << endl;
				cout << "| Имя               | Цена          | Специальный |" << endl;
				cout << "---------------------------------------------------" << endl;
				if (zn == '-') { viewItems(products); }
				else if (zn == '+') { viewItems(specificproducts); }
				else {
					Color::setTextColor(Color::RED);
					cout << "Неверный символ!" << endl;
					Color::setTextColor(Color::DEFAULT);
				}
				cout << "===================================================" << endl;
				break;
			}
			case '9': {
				double totalCostCommon = calculateTotalCost(products);
				double totalCostSpecific = calculateTotalCost(specificproducts);

				cout << "Итоговая стоимость обычных деталей: " << totalCostCommon << " руб.\n";
				cout << "Итоговая стоимость специальных деталей: " << totalCostSpecific << " руб.\n";
				cout << "Общая стоимость: " << (totalCostCommon + totalCostSpecific) << " руб.\n";
				break;
			}
			case 'b': {
				if(user.isAdmin()){
					cout << "Введите имя пользователля - "; cin >> name;
					User::deleteUser(name);
					break;
				}
				else {
					cout << "Это функция для вас недоступна\n";
					break;
				}
			}
			case 'a': {
				User tempuser;
				string password;
				if (user.isAdmin()) {
					cout << "Введите имя пользователля - "; cin >> name;
					cout << "Введите пароль пользователя - "; cin >> password;
					cout << "Выдать ему права админа?(+/-)"; cin >> zn;
					while (1)
					{
						if (zn == '-') { 
							User userr(name, password, false); 
							userr.addUser();
							break;
						}
						else if (zn == '+') { 
							User userr(name, password, true);
							userr.addUser();
							break;
						}
						else {
							Color::setTextColor(Color::RED);
							cout << "Неверный символ!" << endl;
							Color::setTextColor(Color::DEFAULT);
						}
					}
				}
				else {
					cout << "Это функция для вас недоступна\n";
					break;
				}
			}
			case 'c': {
				viewAllUsers();
				break;
			}
			case '0': {
				cout << "Вы вышли из учетной записи!\n";
				isUser = false;
				break; 
			}
			default:
				cout << "Некорректная опция. Пожалуйста, попробуйте снова.\n";
			}
		}
	}
}

int main() {
	SetConsoleOutputCP(1251);
	SetConsoleCP(1251);

	run();
	return 0;
}