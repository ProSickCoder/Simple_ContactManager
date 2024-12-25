#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <algorithm>
#include <regex>
#include <limits>
#include <unordered_map>
#include <memory>
#include <sstream>

using namespace std;

class Contact {
private:
    string name;
    string phone;
    string email;
    string address;

public:
    Contact(string_view n = "", string_view p = "", string_view e = "", string_view a = "")
        : name(n), phone(p), email(e), address(a) {}

    const string& getName() const noexcept { return name; }
    const string& getPhone() const noexcept { return phone; }
    const string& getEmail() const noexcept { return email; }
    const string& getAddress() const noexcept { return address; }

    void setName(string&& n) noexcept { name = std::move(n); }

    void setPhone(const string& p) {
        static const regex phonePattern(R"(\d{3}-?\d{3}-?\d{4})");
        if (!regex_match(p, phonePattern)) {
            throw invalid_argument("Invalid phone format. Use XXX-XXX-XXXX or XXXXXXXXXX");
        }
        phone = p;  // Fixed assignment
    }

    void setEmail(const string& e) {
        static const regex emailPattern(R"([a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,})");
        if (!regex_match(e, emailPattern)) {
            throw invalid_argument("Invalid email format");
        }
        email = e;
    }

    void setAddress(string&& a) noexcept { address = std::move(a); }

    string serialize() const {  // Fixed return type
        return name + '\n' + phone + '\n' + email + '\n' + address + '\n';
    }

    static Contact deserialize(const string& data) {
        stringstream ss(data);
        string n, p, e, a;
        getline(ss, n);
        getline(ss, p);
        getline(ss, e);
        getline(ss, a);
        return Contact(n, p, e, a);
    }
};

class ContactManager {
private:
    vector<Contact> contacts;
    unordered_map<string, size_t> nameIndex;
    const string filename = "contacts.txt";
    bool modified = false;

    void updateNameIndex() {
        nameIndex.clear();
        for (size_t i = 0; i < contacts.size(); ++i) {
            string nameLower = contacts[i].getName();
            transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);
            nameIndex[nameLower] = i;
        }
    }

    void saveToFile() {
        if (!modified) return;

        ofstream file(filename, ios::binary);
        if (!file) {
            throw runtime_error("Unable to open file for writing");
        }

        string buffer;
        buffer.reserve(contacts.size() * 100);

        for (const auto& contact : contacts) {
            buffer += contact.serialize();
        }

        file.write(buffer.c_str(), buffer.size());
        modified = false;
    }

    void loadFromFile() {
        ifstream file(filename, ios::binary | ios::ate);
        if (!file) return;

        streamsize size = file.tellg();
        file.seekg(0, ios::beg);

        string buffer(size, 0);
        if (file.read(&buffer[0], size)) {
            stringstream ss(buffer);
            string contactData;
            string line;

            contacts.clear();
            contacts.reserve(size / 100);

            while (getline(ss, line)) {
                contactData = line + '\n';  // Fixed concatenation
                for (int i = 0; i < 3 && getline(ss, line); ++i) {
                    contactData += line + '\n';
                }
                contacts.push_back(Contact::deserialize(contactData));  // Fixed variable name
            }

            updateNameIndex();
        }
    }

public:
    ContactManager() {
        contacts.reserve(100);
        loadFromFile();
    }

    ~ContactManager() {
        try {
            saveToFile();
        } catch (const exception& e) {
            cerr << "Error saving contacts: " << e.what() << endl;
        }
    }

    void addContact() {
        Contact newContact;
        string input;

        cout << "Enter name: ";
        getline(cin, input);
        newContact.setName(move(input));

        while (true) {
            try {
                cout << "Enter phone (XXX-XXX-XXXX): ";
                getline(cin, input);
                newContact.setPhone(input);
                break;
            } catch (const invalid_argument& e) {
                cout << e.what() << endl;
            }
        }

        while (true) {
            try {
                cout << "Enter email: ";
                getline(cin, input);
                newContact.setEmail(input);
                break;
            } catch (const invalid_argument& e) {
                cout << e.what() << endl;
            }
        }

        cout << "Enter address: ";
        getline(cin, input);
        newContact.setAddress(move(input));

        contacts.push_back(move(newContact));
        modified = true;
        updateNameIndex();
        cout << "Contact added successfully!" << endl;
    }

    // Added missing editContact function
    void editContact() {
        if (contacts.empty()) {
            cout << "No contacts to edit." << endl;
            return;
        }

        displayContacts();
        cout << "Enter the number of contact to edit (1-" << contacts.size() << "): ";
        size_t index;
        cin >> index;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        if (index < 1 || index > contacts.size()) {
            cout << "Invalid contact number." << endl;
            return;
        }

        Contact& contact = contacts[index - 1];
        string input;
        cout << "\n1. Name: " << contact.getName()
             << "\n2. Phone: " << contact.getPhone()
             << "\n3. Email: " << contact.getEmail()
             << "\n4. Address: " << contact.getAddress()
             << "\nEnter field to edit (1-4): ";

        char choice;
        cin >> choice;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        switch (choice) {
            case '1':
                cout << "Enter new name: ";
                getline(cin, input);
                contact.setName(move(input));
                break;
            case '2':
                while (true) {
                    try {
                        cout << "Enter new phone: ";
                        getline(cin, input);
                        contact.setPhone(input);
                        break;
                    } catch (const invalid_argument& e) {
                        cout << e.what() << endl;
                    }
                }
                break;
            case '3':
                while (true) {
                    try {
                        cout << "Enter new email: ";
                        getline(cin, input);
                        contact.setEmail(input);
                        break;
                    } catch (const invalid_argument& e) {
                        cout << e.what() << endl;
                    }
                }
                break;
            case '4':
                cout << "Enter new address: ";
                getline(cin, input);
                contact.setAddress(move(input));
                break;
            default:
                cout << "Invalid choice." << endl;
                return;
        }
        modified = true;
        updateNameIndex();
        cout << "Contact updated successfully!" << endl;
    }

    // Added missing deleteContact function
    void deleteContact() {
        if (contacts.empty()) {
            cout << "No contacts to delete." << endl;
            return;
        }

        displayContacts();
        cout << "Enter the number of contact to delete (1-" << contacts.size() << "): ";
        size_t index;
        cin >> index;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        if (index < 1 || index > contacts.size()) {
            cout << "Invalid contact number." << endl;
            return;
        }

        contacts.erase(contacts.begin() + index - 1);
        modified = true;
        updateNameIndex();
        cout << "Contact deleted successfully!" << endl;
    }

    void searchContacts() {
        if (contacts.empty()) {
            cout << "No contacts to search." << endl;
            return;
        }

        string searchTerm;
        cout << "Enter search term: ";
        getline(cin, searchTerm);
        transform(searchTerm.begin(), searchTerm.end(), searchTerm.begin(), ::tolower);

        vector<reference_wrapper<const Contact>> matches;
        matches.reserve(contacts.size());

        for (const auto& contact : contacts) {
            string name = contact.getName();
            transform(name.begin(), name.end(), name.begin(), ::tolower);
            if (name.find(searchTerm) != string::npos) {
                matches.push_back(ref(contact));
            }
        }

        if (matches.empty()) {
            cout << "No matching contacts found." << endl;
            return;
        }

        cout << "\nMatching Contacts:\n";
        for (const auto& contactRef : matches) {
            displayContact(contactRef.get());
            cout << "--------------------\n";
        }
    }

    // Fixed sortContacts function
    void sortContacts() {
        if (contacts.empty()) {
            cout << "No contacts to sort." << endl;
            return;
        }

        stable_sort(contacts.begin(), contacts.end(),
            [](const Contact& a, const Contact& b) {
                return a.getName() < b.getName();
            });

        modified = true;
        updateNameIndex();
        cout << "Contacts sorted by name!" << endl;
        displayContacts();
    }

    // Added missing displayContacts function
    void displayContacts() const {
        if (contacts.empty()) {
            cout << "No contacts to display." << endl;
            return;
        }

        cout << "\nContacts List:\n";
        for (size_t i = 0; i < contacts.size(); ++i) {
            cout << "\n" << (i + 1) << ". ";
            displayContact(contacts[i]);
            cout << "--------------------\n";
        }
    }

private:
    void displayContact(const Contact& contact) const {
        cout << "Name: " << contact.getName() << endl
             << "Phone: " << contact.getPhone() << endl
             << "Email: " << contact.getEmail() << endl
             << "Address: " << contact.getAddress() << endl;
    }
};