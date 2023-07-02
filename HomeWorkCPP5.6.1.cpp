#include <iostream>
#include <ctime>
#include <Windows.h>
#include <Wt/Dbo/Dbo.h>
#include <Wt/Dbo/backend/Postgres.h>

//  | Создание базы  |
//  V                V

class Book;

class Shop;

class Stock;

class Sale;

class Publisher {

public:

    std::string name = "";

    Wt::Dbo::collection<Wt::Dbo::ptr<Book>> book;

    template<class Action> void persist(Action& act) {

        Wt::Dbo::field(act, name, "name");

        Wt::Dbo::hasMany(act, book, Wt::Dbo::ManyToOne, "id_publisher");
    }
};

class Book {

public:

    std::string title = "";

    Wt::Dbo::ptr<Publisher>id_publisher;

    Wt::Dbo::collection<Wt::Dbo::ptr<Stock>> stock;

    template<class Action> void persist(Action& act) {

        Wt::Dbo::field(act, title, "title");
        Wt::Dbo::belongsTo(act, id_publisher, "id_publisher");

        Wt::Dbo::hasMany(act, stock, Wt::Dbo::ManyToOne, "id_book");
    }
};

class Shop {

public:

    std::string name = "";

    Wt::Dbo::collection<Wt::Dbo::ptr<Stock>> stock;

    template<class Action> void persist(Action& act) {

        Wt::Dbo::field(act, name, "name");

        Wt::Dbo::hasMany(act, stock, Wt::Dbo::ManyToOne, "id_shop");
    }
};

class Stock {

public:

    int count = 0;

    Wt::Dbo::ptr<Book> id_book;
    Wt::Dbo::ptr<Shop> id_shop;

    Wt::Dbo::collection<Wt::Dbo::ptr<Sale>> sale;

    template <class Actoin> void persist(Actoin& act) {

        Wt::Dbo::belongsTo(act, id_book, "id_book");
        Wt::Dbo::belongsTo(act, id_shop, "id_shop");
        Wt::Dbo::field(act, count, "count");

        Wt::Dbo::hasMany(act, sale, Wt::Dbo::ManyToOne, "id_stock");
    }
};

class Sale {

public:
    
    int price = 0;
    std::time_t date_sale;
    int count = 0;

    Wt::Dbo::ptr<Stock> id_stock;

    template <class Action> void persist(Action& act) {

        Wt::Dbo::field(act, price, "price");
        Wt::Dbo::field(act, date_sale, "date_sale");
        Wt::Dbo::belongsTo(act, id_stock, "id_stock");
        Wt::Dbo::field(act, count, "count");
    }
};

//  | Заполнение таблиц |
//  V                   V

void record_publisher(Wt::Dbo::Session& session, std::string& rd_name) {

    Wt::Dbo::Transaction transaction{session};

    Wt::Dbo::ptr<Publisher> record = session.find<Publisher>().where("name = ?").bind(rd_name);

    if (!record) {

        auto publisher = std::make_unique<Publisher>();
        publisher->name = rd_name;
        Wt::Dbo::ptr<Publisher>publisherPtr1 = session.add(std::move(publisher));
    }

    transaction.commit();
}

void record_book(Wt::Dbo::Session& session, std::string& rd_title, std::string& pub_name) {

    Wt::Dbo::Transaction transaction{session};

    Wt::Dbo::ptr<Book> record = session.find<Book>().where("title = ?").bind(rd_title);

    if (!record) {

        Wt::Dbo::ptr<Publisher> publisher = session.find<Publisher>().where("name = ?").bind(pub_name);

        auto book = std::make_unique<Book>();
        book->title = rd_title;
        book->id_publisher = publisher;
        Wt::Dbo::ptr<Book>bookPtr = session.add(std::move(book));

        transaction.commit();
    }
}

void record_shop(Wt::Dbo::Session& session, std::string& rd_name) {

    Wt::Dbo::Transaction transaction{session};

    Wt::Dbo::ptr<Shop> record = session.find<Shop>().where("name = ?").bind(rd_name);

    if (!record) {

        auto shop = std::make_unique<Shop>();
        shop->name = rd_name;
        Wt::Dbo::ptr<Shop>shopPtr = session.add(std::move(shop));
    }

    transaction.commit();
}


void record_stock(Wt::Dbo::Session& session,int& number_book, std::string& bk_title, std::string& sh_name) {

    Wt::Dbo::Transaction transaction{session};

    Wt::Dbo::ptr<Book> book = session.find<Book>().where("title = ?").bind(bk_title);
    Wt::Dbo::ptr<Shop> shop = session.find<Shop>().where("name = ?").bind(sh_name);

    Wt::Dbo::ptr<Stock> record = session.find<Stock>().where("id_book_id = ?").bind(book).where("id_shop_id = ?").bind(shop);

    if (!record) {

        auto stock = std::make_unique<Stock>();
        stock->id_book = book;
        stock->id_shop = shop;
        stock->count = number_book;
        Wt::Dbo::ptr<Stock>stockPtr = session.add(std::move(stock));
    }

    transaction.commit();
}

void record_sale(Wt::Dbo::Session& session, int& rd_price, std::time_t& rd_data_sale, int& rd_count, std::string& bk_title, std::string& sh_name) {

    Wt::Dbo::Transaction transaction{session};

    Wt::Dbo::ptr<Book> book = session.find<Book>().where("title = ?").bind(bk_title);
    Wt::Dbo::ptr<Shop> shop = session.find<Shop>().where("name = ?").bind(sh_name);

    Wt::Dbo::ptr<Stock> stock = session.find<Stock>().where("id_book_id = ?").bind(book).where("id_shop_id = ?").bind(shop);

    auto sale = std::make_unique<Sale>();
    sale->price = rd_price;
    sale->date_sale = rd_data_sale;
    sale->id_stock = stock;
    sale->count = rd_count;
    Wt::Dbo::ptr<Sale>salePtr = session.add(std::move(sale));

    transaction.commit();
}

//  Поиск идентификатора издателя.

unsigned int search_identifier(Wt::Dbo::Session& session, std::string& sh_publisher) {

    unsigned int identifier = 0;

    Wt::Dbo::Transaction transaction{session};

    Wt::Dbo::ptr<Publisher> publisher = session.find<Publisher>().where("name = ?").bind(sh_publisher);

    identifier = publisher.id();

    transaction.commit();

    return identifier;
}

//  Поиск списка магазинов, в которых продают книги.

void search_shop(Wt::Dbo::Session& session, int& identifier) {

    std::set<std::string> book_shops;

    typedef Wt::Dbo::collection<Wt::Dbo::ptr<Book>> Books;
    typedef Wt::Dbo::collection<Wt::Dbo::ptr<Stock>> Stocks;

    Wt::Dbo::Transaction transaction{session};

    Books record = session.find<Book>().where("id_publisher_id = ?").bind(identifier);

    if (record.begin() != record.end()) {

        Books books = session.find<Book>().where("id_publisher_id = ?").bind(identifier);

        const Wt::Dbo::ptr<Book> book = *books.begin();

        Stocks stocks = session.find<Stock>().where("id_book_id = ?").bind(book.id());

        for (const Wt::Dbo::ptr<Stock>& stock : stocks) {

            Wt::Dbo::ptr<Shop> shop = session.find<Shop>().where("id = ?").bind(stock->id_shop);

            book_shops.insert(shop->name);
        }

        for (const auto& book_shop : book_shops) {

            std::cout << book_shop << std::endl;
        }
    }

    transaction.commit();
}

int main()
{
    setlocale(LC_ALL, "ru_RU.UFT-8");

    std::string connection =
        "host=localhost"
        " port=5432"
        " dbname=BookShop"
        " user=postgres"
        " password=pRD(3eW&@?MY$vzS";

    int identifier = 0;
    std::string publisher = "";

    Wt::Dbo::Session session;

    try {
        
        auto postgres = std::make_unique<Wt::Dbo::backend::Postgres>(connection);

        session.setConnection(std::move(postgres));
    }
    catch (const Wt::Dbo::Exception& exc) {

        std::cout << exc.what() << std::endl;
    }

    session.mapClass<Publisher>("publisher");
    session.mapClass<Book>("book");
    session.mapClass<Shop>("shop");
    session.mapClass<Stock>("stock");
    session.mapClass<Sale>("sale");

    try {

        session.createTables();
    }
    catch (const Wt::Dbo::Exception& exc) {

        std::cout << exc.what() << std::endl;
    }

    std::string publisher1 = "AST";
    std::string publisher2 = "Book";
    std::string publisher3 = "Liters";

    record_publisher(session, publisher1);
    record_publisher(session, publisher2);
    record_publisher(session, publisher3);

    std::string book1 = "Academy (Foundation)";
    std::string book2 = "Dune";
    std::string book3 = "The Dune Messiah";
    std::string book4 = "Children of Dune";
    std::string book5 = "Metro 2033";
    std::string book6 = "Metro 2034";

    record_book(session, book1, publisher1);
    record_book(session, book2, publisher2);
    record_book(session, book3, publisher2);
    record_book(session, book4, publisher2);
    record_book(session, book5, publisher3);
    record_book(session, book6, publisher3);

    std::string shop1 = "Books Moscow";
    std::string shop2 = "Books Rostov";
    std::string shop3 = "Books Voronezh";
    std::string shop4 = "Books Volgograd";

    record_shop(session, shop1);
    record_shop(session, shop2);
    record_shop(session, shop3);
    record_shop(session, shop4);

    int number_book = 26;

    record_stock(session, number_book, book1, shop1);
    record_stock(session, number_book, book1, shop2);
    record_stock(session, number_book, book1, shop3);
    record_stock(session, number_book, book2, shop3);
    record_stock(session, number_book, book2, shop4);
    record_stock(session, number_book, book3, shop1);
    record_stock(session, number_book, book3, shop2);
    record_stock(session, number_book, book3, shop4);
    record_stock(session, number_book, book4, shop1);
    record_stock(session, number_book, book5, shop1);
    record_stock(session, number_book, book5, shop2);
    record_stock(session, number_book, book5, shop3);
    record_stock(session, number_book, book5, shop4);
    record_stock(session, number_book, book6, shop2);
    record_stock(session, number_book, book6, shop3);
    record_stock(session, number_book, book6, shop4);
     
    
    
    do {

        setlocale(LC_ALL, ".1251");

        std::cout << "Введите издателе (publisher), имя или идентификатор или '*' для выхода: ";

        setlocale(LC_ALL, "ru_RU.UFT-8");

        std::cin >> publisher;

        try {

            identifier = std::stoi(publisher);
        }
        catch (std::exception& exc) {

            identifier = search_identifier(session, publisher);
        }

        setlocale(LC_ALL, ".1251");

        std::cout << "Список магазинов, в которых продают книги:" << std::endl;

        setlocale(LC_ALL, "ru_RU.UFT-8");

        search_shop(session, identifier);

    } while (publisher != "*");
}