#include <iostream>
#include <string>
#include <fstream>
#include <limits>
#include <climits>
#include <cstdlib>  // For atoi/atof
#include <sstream>   // For stringstream
using namespace std;

// Admin credentials
const string ADMIN_USERNAME = "admin";
const string ADMIN_PASSWORD = "admin123";

// File names
const string MENU_FILE = "menu_data.txt";
const string ORDERS_FILE = "orders_data.txt";
const string FEEDBACK_FILE = "feedback_data.txt";

// Input validation function
int getIntegerInput(int minVal, int maxVal) {
    int value;
    while (true) {
        cin >> value;
        if (cin.fail() || value < minVal || value > maxVal) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input! Please enter a number between " 
                 << minVal << " and " << maxVal << ": ";
        } else {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return value;
        }
    }
}

// Node for menu item (linked list)
struct MenuItem {
    int id;
    string name;
    float price;
    string category;
    MenuItem* next;

    MenuItem(int _id, string _name, float _price, string _category) {
        id = _id;
        name = _name;
        price = _price;
        category = _category;
        next = NULL;
    }
};

// Node for order item
struct OrderItem {
    string itemName;
    float price;
    OrderItem* next;

    OrderItem(string _itemName, float _price) {
        itemName = _itemName;
        price = _price;
        next = NULL;
    }
};

// Node for orders
struct Order {
    int orderId;
    OrderItem* itemHead;
    Order* next;

    Order(int _orderId) {
        orderId = _orderId;
        itemHead = NULL;
        next = NULL;
    }
};

// Node for customer feedback
struct Feedback {
    int itemId;
    string comment;
    int rating;
    Feedback* next;

    Feedback(int _itemId, string _comment, int _rating) {
        itemId = _itemId;
        comment = _comment;
        rating = _rating;
        next = NULL;
    }
};

class Restaurant {
private:
    MenuItem* menuHead;
    Order* orderFront;
    Order* orderRear;
    Feedback* feedbackHead;
    int nextOrderId;

    // File handling methods
    void saveMenuToFile() {
        ofstream outFile(MENU_FILE.c_str());
        if (!outFile) {
            cout << "Error opening menu file for writing.\n";
            return;
        }

        MenuItem* temp = menuHead;
        while (temp) {
            outFile << temp->id << "," << temp->name << "," 
                   << temp->price << "," << temp->category << "\n";
            temp = temp->next;
        }
        outFile.close();
    }

    void loadMenuFromFile() {
        ifstream inFile(MENU_FILE.c_str());
        if (!inFile) {
            cout << "No existing menu file found. Starting with default menu.\n";
            initializeMenu();
            return;
        }

        string line;
        while (getline(inFile, line)) {
            size_t pos1 = line.find(',');
            size_t pos2 = line.find(',', pos1+1);
            size_t pos3 = line.find(',', pos2+1);

            if (pos1 == string::npos || pos2 == string::npos || pos3 == string::npos) {
                continue;
            }

            int id = atoi(line.substr(0, pos1).c_str());
            string name = line.substr(pos1+1, pos2-pos1-1);
            
            string priceStr = line.substr(pos2+1, pos3-pos2-1);
            float price;
            stringstream(priceStr) >> price;
            
            string category = line.substr(pos3+1);

            addMenuItem(id, name, price, category);
        }
        inFile.close();
    }

    void saveOrdersToFile() {
        ofstream outFile(ORDERS_FILE.c_str());
        if (!outFile) {
            cout << "Error opening orders file for writing.\n";
            return;
        }

        Order* tempOrder = orderFront;
        while (tempOrder) {
            outFile << "ORDER:" << tempOrder->orderId << "\n";
            
            OrderItem* tempItem = tempOrder->itemHead;
            while (tempItem) {
                outFile << "ITEM:" << tempItem->itemName << "," << tempItem->price << "\n";
                tempItem = tempItem->next;
            }
            
            tempOrder = tempOrder->next;
        }
        outFile.close();
    }

    void loadOrdersFromFile() {
        ifstream inFile(ORDERS_FILE.c_str());
        if (!inFile) {
            cout << "No existing orders file found.\n";
            return;
        }

        string line;
        Order* currentOrder = NULL;
        int maxOrderId = 0;

        while (getline(inFile, line)) {
            if (line.find("ORDER:") == 0) {
                int orderId = atoi(line.substr(6).c_str());
                if (orderId > maxOrderId) maxOrderId = orderId;
                
                currentOrder = new Order(orderId);
                
                if (!orderFront) {
                    orderFront = orderRear = currentOrder;
                } else {
                    orderRear->next = currentOrder;
                    orderRear = currentOrder;
                }
            } 
            else if (line.find("ITEM:") == 0 && currentOrder) {
                size_t pos = line.find(',', 5);
                if (pos == string::npos) continue;
                
                string name = line.substr(5, pos-5);
                
                string priceStr = line.substr(pos+1);
                float price;
                stringstream(priceStr) >> price;
                
                OrderItem* newItem = new OrderItem(name, price);
                if (!currentOrder->itemHead) {
                    currentOrder->itemHead = newItem;
                } else {
                    OrderItem* temp = currentOrder->itemHead;
                    while (temp->next) temp = temp->next;
                    temp->next = newItem;
                }
            }
        }
        
        nextOrderId = maxOrderId + 1;
        inFile.close();
    }

    void saveFeedbackToFile() {
        ofstream outFile(FEEDBACK_FILE.c_str());
        if (!outFile) {
            cout << "Error opening feedback file for writing.\n";
            return;
        }

        Feedback* temp = feedbackHead;
        while (temp) {
            outFile << temp->itemId << "," << temp->rating << "," << temp->comment << "\n";
            temp = temp->next;
        }
        outFile.close();
    }

    void loadFeedbackFromFile() {
        ifstream inFile(FEEDBACK_FILE.c_str());
        if (!inFile) {
            cout << "No existing feedback file found.\n";
            return;
        }

        string line;
        while (getline(inFile, line)) {
            size_t pos1 = line.find(',');
            size_t pos2 = line.find(',', pos1+1);

            if (pos1 == string::npos || pos2 == string::npos) {
                continue;
            }

            int itemId = atoi(line.substr(0, pos1).c_str());
            int rating = atoi(line.substr(pos1+1, pos2-pos1-1).c_str());
            string comment = line.substr(pos2+1);

            addFeedback(itemId, comment, rating);
        }
        inFile.close();
    }

    void initializeMenu() {
        addMenuItem(1, "Garlic Bread with Cheese", 6.00, "Starters / Appetizers");
        addMenuItem(2, "Spicy Chicken Wings", 9.00, "Starters / Appetizers");
        addMenuItem(3, "Mozzarella Sticks", 7.50, "Starters / Appetizers");
        addMenuItem(4, "Hummus & Pita Platter", 8.00, "Starters / Appetizers");
        addMenuItem(5, "Stuffed Mushrooms", 7.00, "Starters / Appetizers");
        addMenuItem(6, "Soup of the Day", 5.50, "Starters / Appetizers");
        addMenuItem(7, "Classic Caesar Salad", 10.00, "Salads");
        addMenuItem(8, "Greek Salad", 11.00, "Salads");
        addMenuItem(9, "Grilled Chicken Cobb Salad", 12.50, "Salads");
        addMenuItem(10, "Avocado & Quinoa Bowl", 11.50, "Salads");
        addMenuItem(11, "Grilled Salmon with Lemon Butter Sauce", 18.00, "Main Courses");
        addMenuItem(12, "Spaghetti Carbonara", 14.00, "Main Courses");
        addMenuItem(13, "Chicken Alfredo Pasta", 15.00, "Main Courses");
        addMenuItem(14, "Beef Burger with Fries", 13.00, "Main Courses");
        addMenuItem(15, "Paneer Tikka Masala with Rice (Vegetarian)", 13.50, "Main Courses");
        addMenuItem(16, "Thai Green Curry (Chicken or Veg)", 14.00, "Main Courses");
        addMenuItem(17, "Philly Cheesesteak Sandwich", 12.00, "Sandwiches & Wraps");
        addMenuItem(18, "Grilled Veggie Wrap with Hummus", 10.50, "Sandwiches & Wraps");
        addMenuItem(19, "Club Sandwich with Bacon & Turkey", 11.50, "Sandwiches & Wraps");
        addMenuItem(20, "New York Cheesecake", 6.00, "Desserts");
        addMenuItem(21, "Chocolate Lava Cake", 6.50, "Desserts");
        addMenuItem(22, "Tiramisu", 6.50, "Desserts");
        addMenuItem(23, "Fruit Salad with Mint", 5.00, "Desserts");
        addMenuItem(24, "Ice Cream (Vanilla/Chocolate/Strawberry)", 4.00, "Desserts");
        addMenuItem(25, "Coffee (Hot/Iced)", 3.00, "Beverages");
        addMenuItem(26, "Soft Drinks", 2.50, "Beverages");
        addMenuItem(27, "Fresh Juices (Orange, Apple, Carrot)", 4.00, "Beverages");
        addMenuItem(28, "Lemon Iced Tea", 3.50, "Beverages");
        addMenuItem(29, "Milkshakes (Chocolate, Strawberry, Vanilla)", 5.00, "Beverages");
        saveMenuToFile();
    }

    void addMenuItem(int id, string name, float price, string category) {
        MenuItem* newItem = new MenuItem(id, name, price, category);
        if (!menuHead) {
            menuHead = newItem;
        } else {
            MenuItem* temp = menuHead;
            while (temp->next) temp = temp->next;
            temp->next = newItem;
        }
    }

    void addFeedback(int itemId, string comment, int rating) {
        Feedback* newFeedback = new Feedback(itemId, comment, rating);
        if (!feedbackHead) {
            feedbackHead = newFeedback;
        } else {
            Feedback* temp = feedbackHead;
            while (temp->next) temp = temp->next;
            temp->next = newFeedback;
        }
    }

public:
    Restaurant() {
        menuHead = NULL;
        orderFront = NULL;
        orderRear = NULL;
        feedbackHead = NULL;
        nextOrderId = 1;
        
        loadMenuFromFile();
        loadOrdersFromFile();
        loadFeedbackFromFile();
    }

    ~Restaurant() {
        while (menuHead) {
            MenuItem* temp = menuHead;
            menuHead = menuHead->next;
            delete temp;
        }

        while (orderFront) {
            Order* temp = orderFront;
            orderFront = orderFront->next;
            OrderItem* item = temp->itemHead;
            while (item) {
                OrderItem* iTemp = item;
                item = item->next;
                delete iTemp;
            }
            delete temp;
        }

        while (feedbackHead) {
            Feedback* temp = feedbackHead;
            feedbackHead = feedbackHead->next;
            delete temp;
        }
    }

    void editOrder(int orderId) {
        Order* temp = orderFront;
        while (temp && temp->orderId != orderId) temp = temp->next;

        if (!temp) {
            cout << "Order ID not found.\n";
            return;
        }

        char choice;
        cout << "Editing Order #" << orderId << ":\n";
        cout << "Do you want to add more items? (y/n): ";
        cin >> choice;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        while (choice == 'y' || choice == 'Y') {
            displayMenu();
            cout << "Enter Item ID to add: ";
            int itemId = getIntegerInput(1, 29);

            MenuItem* mItem = menuHead;
            while (mItem && mItem->id != itemId) mItem = mItem->next;

            if (!mItem) {
                cout << "Invalid item ID.\n";
            } else {
                OrderItem* newItem = new OrderItem(mItem->name, mItem->price);
                if (!temp->itemHead) {
                    temp->itemHead = newItem;
                } else {
                    OrderItem* last = temp->itemHead;
                    while (last->next) last = last->next;
                    last->next = newItem;
                }
                cout << mItem->name << " added to your order.\n";
            }

            cout << "Add another item? (y/n): ";
            cin >> choice;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
        saveOrdersToFile();
    }

    void cancelOrder(int orderId) {
        if (!orderFront) {
            cout << "No orders to cancel.\n";
            return;
        }

        Order* curr = orderFront;
        Order* prev = NULL;

        while (curr && curr->orderId != orderId) {
            prev = curr;
            curr = curr->next;
        }

        if (!curr) {
            cout << "Order ID not found.\n";
            return;
        }

        if (prev == NULL) {
            orderFront = curr->next;
            if (orderRear == curr) orderRear = NULL;
        } else {
            prev->next = curr->next;
            if (orderRear == curr) orderRear = prev;
        }

        OrderItem* item = curr->itemHead;
        while (item) {
            OrderItem* nextItem = item->next;
            delete item;
            item = nextItem;
        }
        delete curr;

        cout << "Order #" << orderId << " has been cancelled.\n";
        saveOrdersToFile();
    }

    void displayMenu() {
        if (!menuHead) {
            cout << "Menu is empty.\n";
            return;
        }

        cout << "\n--- MENU ---\n";
        string currentCategory = "";
        MenuItem* temp = menuHead;
        while (temp) {
            if (temp->category != currentCategory) {
                currentCategory = temp->category;
                cout << "\n--- " << currentCategory << " ---\n";
            }
            cout << "ID: " << temp->id
                 << " | Name: " << temp->name
                 << " | Price: $" << temp->price << "\n";
            temp = temp->next;
        }
    }

    void placeOrder() {
        char moreItems = 'y';
        Order* newOrder = new Order(nextOrderId++);

        while (moreItems == 'y' || moreItems == 'Y') {
            displayMenu();
            cout << "Enter Item ID to order: ";
            int itemId = getIntegerInput(1, 29);

            MenuItem* temp = menuHead;
            while (temp && temp->id != itemId) temp = temp->next;

            if (!temp) {
                cout << "Invalid Item ID!\n";
            } else {
                OrderItem* newOrderItem = new OrderItem(temp->name, temp->price);
                if (!newOrder->itemHead) {
                    newOrder->itemHead = newOrderItem;
                } else {
                    OrderItem* tempItem = newOrder->itemHead;
                    while (tempItem->next) tempItem = tempItem->next;
                    tempItem->next = newOrderItem;
                }
                cout << "Added " << temp->name << " to your order.\n";
            }

            cout << "Would you like to add another item? (y/n): ";
            cin >> moreItems;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }

        if (!newOrder->itemHead) {
            delete newOrder;
            cout << "No valid items in order. Order discarded.\n";
            return;
        }

        if (!orderFront) {
            orderFront = orderRear = newOrder;
        } else {
            orderRear->next = newOrder;
            orderRear = newOrder;
        }

        cout << "Order placed successfully! Your Order ID is #" << newOrder->orderId << "\n";
        saveOrdersToFile();
    }

    void serveOrder() {
        if (!orderFront) {
            cout << "No orders to serve.\n";
            return;
        }

        Order* temp = orderFront;
        cout << "Serving Order #" << temp->orderId << ":\n";
        OrderItem* itemTemp = temp->itemHead;
        while (itemTemp) {
            cout << "  Item: " << itemTemp->itemName
                 << " | Price: $" << itemTemp->price << "\n";
            itemTemp = itemTemp->next;
        }

        orderFront = orderFront->next;
        if (!orderFront) orderRear = NULL;

        OrderItem* item = temp->itemHead;
        while (item) {
            OrderItem* nextItem = item->next;
            delete item;
            item = nextItem;
        }
        delete temp;
        
        saveOrdersToFile();
    }

    void viewOrders() {
        if (!orderFront) {
            cout << "No current orders.\n";
            return;
        }

        cout << "\n--- Current Orders ---\n";
        Order* temp = orderFront;
        while (temp) {
            cout << "Order #" << temp->orderId << ":\n";
            OrderItem* itemTemp = temp->itemHead;
            while (itemTemp) {
                cout << "  Item: " << itemTemp->itemName
                     << " | Price: $" << itemTemp->price << "\n";
                itemTemp = itemTemp->next;
            }
            temp = temp->next;
        }
    }

    void leaveFeedback(int itemId, string comment, int rating) {
        addFeedback(itemId, comment, rating);
        cout << "Thank you for your feedback on Item #" << itemId << "!\n";
        saveFeedbackToFile();
    }

    void viewFeedback(int itemId) {
        bool found = false;
        cout << "\n--- Feedback for Item #" << itemId << " ---\n";
        Feedback* temp = feedbackHead;
        while (temp) {
            if (temp->itemId == itemId) {
                cout << "Rating: " << temp->rating << "/10 | Comment: " << temp->comment << "\n";
                found = true;
            }
            temp = temp->next;
        }
        if (!found) {
            cout << "No feedback available for this item.\n";
        }
    }

    void generateBill() {
        if (!orderFront) {
            cout << "No orders to generate a bill.\n";
            return;
        }

        float total = 0.0;
        OrderItem* item = orderFront->itemHead;
        cout << "\n--- Bill for Order #" << orderFront->orderId << " ---\n";
        while (item) {
            cout << "Item: " << item->itemName << " | Price: $" << item->price << "\n";
            total += item->price;
            item = item->next;
        }

        cout << "-------------------------\n";
        cout << "Total Amount: $" << total << "\n";

        int paymentChoice;
        string paymentMethod;
        do {
            cout << "\nSelect Payment Method:\n";
            cout << "1. Cash\n";
            cout << "2. Card\n";
            cout << "Enter choice: ";
            paymentChoice = getIntegerInput(1, 2);

            if (paymentChoice == 1) {
                paymentMethod = "Cash";
            } else {
                paymentMethod = "Card";
                cout << "Kindly enter your 4-digit pin on the card machine given by the waiter!\n";
            }
        } while (paymentChoice != 1 && paymentChoice != 2);

        cout << "Payment Method: " << paymentMethod << "\n";
        cout << "Thank you for your payment!\n";
    } 
};

int main() {
    Restaurant r;
    int choice;

    do {
        cout << "\n=== Restaurant Management System ===\n";
        cout << "1. Admin Panel\n";
        cout << "2. Customer Panel\n";
        cout << "3. Exit\n";
        cout << "Enter your choice: ";
        choice = getIntegerInput(1, 3);

        switch (choice) {
            case 1: {
                string username, password;
                cout << "\n--- Admin Login ---\n";
                cout << "Username: ";
                cin >> username;
                cout << "Password: ";
                cin >> password;
                cin.ignore(numeric_limits<streamsize>::max(), '\n');

                if (username == ADMIN_USERNAME && password == ADMIN_PASSWORD) {
                    int adminChoice;
                    do {
                        cout << "\n--- Admin Panel ---\n";
                        cout << "1. View Menu\n";
                        cout << "2. View All Orders\n";
                        cout << "3. Serve Order\n";
                        cout << "4. View Feedback\n";
                        cout << "5. Exit Admin Panel\n";
                        cout << "Enter choice: ";
                        adminChoice = getIntegerInput(1, 5);

                        switch (adminChoice) {
                            case 1: r.displayMenu(); break;
                            case 2: r.viewOrders(); break;
                            case 3: r.serveOrder(); break;
                            case 4: {
                                cout << "Enter Item ID to view feedback: ";
                                int itemId = getIntegerInput(1, 29);
                                r.viewFeedback(itemId);
                                break;
                            }
                            case 5: cout << "Exiting Admin Panel...\n"; break;
                        }
                    } while (adminChoice != 5);
                } else {
                    cout << "Incorrect username or password!\n";
                }
                break;
            }

            case 2: {
                int custChoice;
                do {
                    cout << "\n--- Customer Panel ---\n";
                    cout << "1. View Menu\n";
                    cout << "2. Place Order\n";
                    cout << "3. Leave Feedback\n";
                    cout << "4. Generate Bill\n";
                    cout << "5. Edit Existing Order\n";
                    cout << "6. Cancel Order\n";
                    cout << "7. Exit Customer Panel\n";
                    cout << "Enter your choice: ";
                    custChoice = getIntegerInput(1, 7);

                    switch (custChoice) {
                        case 1: r.displayMenu(); break;
                        case 2: r.placeOrder(); break;
                        case 3: {
                            cout << "Enter Item ID: ";
                            int itemId = getIntegerInput(1, 29);
                            cin.ignore();
                            string comment;
                            cout << "Enter your comment: ";
                            getline(cin, comment);
                            cout << "Enter rating (1-10): ";
                            int rating = getIntegerInput(1, 10);
                            r.leaveFeedback(itemId, comment, rating);
                            break;
                        }
                        case 4: r.generateBill(); break;
                        case 5: {
                            cout << "Enter your Order ID to edit: ";
                            int orderId = getIntegerInput(1, INT_MAX);
                            r.editOrder(orderId);
                            break;
                        }
                        case 6: {
                            cout << "Enter your Order ID to cancel: ";
                            int orderId = getIntegerInput(1, INT_MAX);
                            r.cancelOrder(orderId);
                            break;
                        }
                        case 7: cout << "Exiting Customer Panel...\n"; break;
                    }
                } while (custChoice != 7);
                break;
            }

            case 3: cout << "Exiting system. Goodbye!\n"; break;
        }

    } while (choice != 3);

    return 0;
}
