#include <stdlib.h>
#include <iostream>
#include <string>
#include "mysql_connection.h"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <ctime>
#include <chrono>
#include <fstream>
#include <algorithm>
#include <vector>

using namespace std;
using namespace sql;

const string server = "tcp://127.0.0.1:3306";
const string username = "cpp";
const string password = "dchosen1";
const string database = "tbpsns";

class Permit {
public:
    //default constructor
    Permit() : NRICPassportNo(""), Name(""), CompanyName(""), VehicleNo(""), ContactNo("") {}
    
    string NRICPassportNo;
    string Name;
    string CompanyName;
    string VehicleNo;
    string ContactNo;

    Permit(string nr, string n, string cn, string vn, string cn2) {
        NRICPassportNo = nr;
        Name = n;
        CompanyName = cn;
        VehicleNo = vn;
        ContactNo = cn2;
    }

    void display() const {
        cout << "NRIC/Passport No: " << NRICPassportNo << endl;
        cout << "Name: " << Name << endl;
        cout << "Company Name: " << CompanyName << endl;
        cout << "Vehicle Number: " << VehicleNo << endl;
        cout << "Contact Number: " << ContactNo << endl;
    }
};

static void displayPermit(Permit permit) {
    cout << "NRIC/Passport No: " << permit.NRICPassportNo << endl;
    cout << "Name: " << permit.Name << endl;
    cout << "Company Name: " << permit.CompanyName << endl;
    cout << "Vehicle Number: " << permit.VehicleNo << endl;
    cout << "Contact Number: " << permit.ContactNo << endl;
}

static void readAllVEP(Connection* conn, bool useOpenMP = false) {
    try {
        Statement* stmt = conn->createStatement();
        ResultSet* rs = stmt->executeQuery("SELECT * FROM vep");

        cout << "==================== VEP Records ====================" << endl;
        //cout << "NRIC/Passport No\tName\tCompany Name" << endl;
        //cout << "=====================================================" << endl;

        if (useOpenMP) {
            #pragma omp parallel for
            for (size_t i = 0; i < rs->rowsCount(); i++) {
                rs->next();
                //cout << rs->getString("NRICPassportNo") << "\t" << rs->getString("Name") << "\t" << rs->getString("CompanyName") << endl;
                Permit permit;
                permit.NRICPassportNo = rs->getString("NRICPassportNo");
                permit.Name = rs->getString("Name");
                permit.CompanyName = rs->getString("CompanyName");
                permit.VehicleNo = rs->getString("VehicleNo");
                permit.ContactNo = rs->getString("ContactNo");

                displayPermit(permit);

                cout << "-----------------------------------------------------" << endl;
            }
        }
        else {
            while (rs->next()) {
                //cout << rs->getString("NRICPassportNo") << "\t" << rs->getString("Name") << "\t" << rs->getString("CompanyName") << endl;
                Permit permit;
                permit.NRICPassportNo = rs->getString("NRICPassportNo");
                permit.Name = rs->getString("Name");
                permit.CompanyName = rs->getString("CompanyName");
                permit.VehicleNo = rs->getString("VehicleNo");
                permit.ContactNo = rs->getString("ContactNo");

                displayPermit(permit);

                cout << "-----------------------------------------------------" << endl;
            }
        }

        cout << "=====================================================" << endl;
        cout << "Total number of records: " << rs->rowsCount() << endl;

        delete rs;
        delete stmt;
    }
    catch (SQLException& e) {
        cout << e.what() << endl;
    }
}

static void createVEP(Connection* conn) {
    try {
        Permit permit;
        //string NRICPassportNo, Name, CompanyName, VehicleNo, ContactNo, sql;

        // USER INPUT
        cout << "Enter NRIC/Passport number: ";
        getline(cin, permit.NRICPassportNo);

        cout << "Enter name: ";
        getline(cin, permit.Name);

        cout << "Enter company name: ";
        getline(cin, permit.CompanyName);

        cout << "Enter vehicle number: ";
        getline(cin, permit.VehicleNo);

        cout << "Enter contact number: ";
        getline(cin, permit.ContactNo);

        string sql = "INSERT INTO vep (NRICPassportNo, Name, CompanyName, VehicleNo, ContactNo) VALUES (?,?,?,?,?)";
        PreparedStatement* pstmt = conn->prepareStatement(sql);

        pstmt->setString(1, permit.NRICPassportNo);
        pstmt->setString(2, permit.Name);
        pstmt->setString(3, permit.CompanyName);
        pstmt->setString(4, permit.VehicleNo);
        pstmt->setString(5, permit.ContactNo);
        pstmt->executeUpdate();

        delete pstmt;
    }
    catch (SQLException& e) {
        cout << e.what() << endl;
    }
}

static void updateVEP(Connection* conn) {
    try {
        Permit permit;
        int id;

        cout << "Enter the ID of the record to update: ";
        cin >> id;
        cin.ignore();

        string NRICPassportNo, Name, CompanyName, VehicleNo, ContactNo, sql;

        // USER INPUT
        cout << "Enter NRIC/Passport number: ";
        getline(cin, permit.NRICPassportNo);

        cout << "Enter name: ";
        getline(cin, permit.Name);

        cout << "Enter company name: ";
        getline(cin, permit.CompanyName);

        cout << "Enter vehicle number: ";
        getline(cin, permit.VehicleNo);

        cout << "Enter contact number: ";
        getline(cin, permit.ContactNo);

        sql = "UPDATE vep SET NRICPassportNo=?, Name=?, CompanyName=?, VehicleNo=?, ContactNo=? WHERE id=?";
        PreparedStatement* pstmt = conn->prepareStatement(sql);

        pstmt->setString(1, permit.NRICPassportNo);
        pstmt->setString(2, permit.Name);
        pstmt->setString(3, permit.CompanyName);
        pstmt->setString(4, permit.VehicleNo);
        pstmt->setString(5, permit.ContactNo);
        pstmt->setInt(6, id);
        pstmt->executeUpdate();

        delete pstmt;
    }
    catch (SQLException& e) {
        cout << e.what() << endl;
    }
}

// Sequential search function
static void sequentialSearch(const vector<Permit>& permits, const string& keyword) {
    cout << "==================== Sequential Search Results ====================" << endl;
    for (const auto& permit : permits) {
        if (permit.Name.find(keyword) != string::npos) {
            permit.display();
            cout << "-----------------------------------------------------" << endl;
        }
    }
}

static void searchVEP(Connection* conn) {
    try {
        string keyword;
        cout << "Enter keyword to search: ";
        getline(cin, keyword);

        string sql = "SELECT * FROM vep WHERE Name LIKE ?";
        unique_ptr<PreparedStatement> pstmt(conn->prepareStatement(sql));
        pstmt->setString(1, "%" + keyword + "%");

        unique_ptr<ResultSet> rs(pstmt->executeQuery());

        vector<Permit> permits;

        cout << "==================== Database Search Results ====================" << endl;
        while (rs->next()) {
            Permit permit(
                rs->getString("NRICPassportNo"),
                rs->getString("Name"),
                rs->getString("CompanyName"),
                rs->getString("VehicleNo"),
                rs->getString("ContactNo")
            );
            permits.push_back(permit);
            permit.display();
            cout << "-----------------------------------------------------" << endl;
        }

        cout << "Enter sequential search keyword: ";
        getline(cin, keyword);
        sequentialSearch(permits, keyword);
    }
    catch (SQLException& e) {
        cout << e.what() << endl;
    }
}




static void deleteVEP(Connection* conn) {
    try {
        //int id;
        string id;
        Permit permit;

        cout << "Enter record ID to delete: ";
        cin >> id;
        cin.ignore();

        string sql = "SELECT * FROM vep WHERE NRICPassportNo = ?";
        PreparedStatement* pstmt = conn->prepareStatement(sql);
        pstmt->setString(1, id);
        ResultSet* rs = pstmt->executeQuery();

        if (rs->next()) {
            permit.NRICPassportNo = rs->getString("NRICPassportNo");
            permit.Name = rs->getString("Name");
            permit.CompanyName = rs->getString("CompanyName");
            permit.VehicleNo = rs->getString("VehicleNo");
            permit.ContactNo = rs->getString("ContactNo");

            cout << "Are you sure you want to delete the record with ID " << id << "? (yes/no): ";
            string response;
            cin >> response;
            cin.ignore();

            if (response == "yes") {
                sql = "DELETE FROM vep WHERE NRICPassportNo = ?";
                pstmt = conn->prepareStatement(sql);
                pstmt->setString(1, id);
                int rowsDeleted = pstmt->executeUpdate();
                cout << rowsDeleted << " record deleted" << endl;
            }
            else {
                cout << "Record not deleted" << endl;
            }
        }
        else {
            cout << "Record not found" << endl;
        }

        delete rs;
        delete pstmt;
    }
    catch (SQLException& e) {
        cout << e.what() << endl;
    }
}

static void exportVEP(Connection* conn) {
    try {
        ofstream file("output.txt");
        if (!file) {
            cout << "Error opening file" << endl;
            return;
        }

        string sql = "SELECT * FROM vep";
        PreparedStatement* pstmt = conn->prepareStatement(sql);
        ResultSet* rs = pstmt->executeQuery();

        while (rs->next()) {
            Permit permit;
            permit.NRICPassportNo = rs->getString("NRICPassportNo");
            permit.Name = rs->getString("Name");
            permit.CompanyName = rs->getString("CompanyName");
            permit.VehicleNo = rs->getString("VehicleNo");
            permit.ContactNo = rs->getString("ContactNo");

            file << "ID: " << rs->getInt("id") << endl;
            file << "Name: " << permit.Name << endl;
            file << "NRIC/Passport No: " << permit.NRICPassportNo << endl;
            file << "Company name: " << permit.CompanyName << endl;
            file << "Vehicle number: " << permit.VehicleNo << endl;
            file << "Contact number: " << permit.ContactNo << endl;
            file << "-----------------------------------------------------" << endl;
        }

        file.close();
    }
    catch (SQLException& e) {
        cout << e.what() << endl;
    }
}

vector<Permit> fetchPermitsFromDatabase() {
    vector<Permit> permits;
    try {
        Driver* driver = get_driver_instance();
        unique_ptr<Connection> conn(driver->connect(server, username, password));
        conn->setSchema(database);

        unique_ptr<Statement> stmt(conn->createStatement());
        unique_ptr<ResultSet> rs(stmt->executeQuery("SELECT NRICPassportNo, Name, CompanyName, VehicleNo, ContactNo FROM vep"));

        while (rs->next()) {
            Permit permit(
                rs->getString("NRICPassportNo"),
                rs->getString("Name"),
                rs->getString("CompanyName"),
                rs->getString("VehicleNo"),
                rs->getString("ContactNo")
            );
            permits.push_back(permit);
        }
    }
    catch (SQLException& e) {
        cout << "Error: " << e.what() << endl;
    }
    return permits;
}

// Bubble sort algorithm
static void bubbleSort(vector<Permit>& permits) {
    int n = permits.size();
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (permits[j].Name > permits[j + 1].Name) {
                swap(permits[j], permits[j + 1]);
            }
        }
    }

    // Print the sorted data
    cout << "Sorted Records:" << endl;
    for (const auto& permit : permits) {
        permit.display();
        cout << endl;
    }
}

////////////////////////////////////////////////////////////////////////////////////
//OpenMP

static void readAllVEPOMP(Connection* conn) {
    try {
        Statement* stmt = conn->createStatement();
        ResultSet* rs = stmt->executeQuery("SELECT * FROM vep");

        cout << "==================== VEP Records ====================" << endl;
        //cout << "NRIC/Passport No\tName\tCompany name" << endl;
        //cout << "=====================================================" << endl;

        #pragma omp parallel for
        for (int i = 0; i < rs->rowsCount(); i++) {
            rs->next();
            //cout << rs->getString("NRICPassportNo") << "\t" << rs->getString("Name") << "\t" << rs->getString("CompanyName") << endl;
            Permit permit;
            permit.NRICPassportNo = rs->getString("NRICPassportNo");
            permit.Name = rs->getString("Name");
            permit.CompanyName = rs->getString("CompanyName");
            permit.VehicleNo = rs->getString("VehicleNo");
            permit.ContactNo = rs->getString("ContactNo");

            displayPermit(permit);

            cout << "-----------------------------------------------------" << endl;
        }

        cout << "=====================================================" << endl;
        cout << "NRIC/Passport No\tName\tCompany name" << endl;
        cout << "====Total number of records: " << rs->rowsCount() << "====" << endl;
        
        delete rs;
        delete stmt;
    }
    catch (SQLException& e) {
        cout << e.what() << endl;
    }
}

static void createVEPOMP(Connection* conn) {
    try {
        //string NRICPassportNo, Name, CompanyName, VehicleNo, ContactNo, sql;
        Permit permit;

        //USER INPUT
        cout << "Enter NRIC/Passport number: ";
        getline(cin, permit.NRICPassportNo);

        cout << "Enter name:";
        getline(cin, permit.Name);

        cout << "Enter company name:";
        getline(cin, permit.CompanyName);

        cout << "Enter vehicle number:";
        getline(cin, permit.VehicleNo);

        cout << "Enter contact number:";
        getline(cin, permit.ContactNo);

        string sql = "INSERT INTO vep (NRICPassportNo, Name, CompanyName, VehicleNo, ContactNo) VALUES (?,?,?,?,?) ";
        PreparedStatement* pstmt = conn->prepareStatement(sql);

        pstmt->setString(1, permit.NRICPassportNo);
        pstmt->setString(2, permit.Name);
        pstmt->setString(3, permit.CompanyName);
        pstmt->setString(4, permit.VehicleNo);
        pstmt->setString(5, permit.ContactNo);
        pstmt->executeUpdate();

        delete pstmt;
    }
    catch (SQLException& e) {
        cout << e.what() << endl;
    }
}

static void updateVEPOMP(Connection* conn) {
    try {
        int id;
        Permit permit;

        cout << "Enter the ID of the record to update:";
        cin >> id;
        cin.ignore();

        //string NRICPassportNo, Name, CompanyName, VehicleNo, ContactNo, sql;

        //USER INPUT
        cout << "Enter NRIC/Passport number: ";
        getline(cin, permit.NRICPassportNo);

        cout << "Enter name:";
        getline(cin, permit.Name);

        cout << "Enter company name:";
        getline(cin, permit.CompanyName);

        cout << "Enter vehicle number:";
        getline(cin, permit.VehicleNo);

        cout << "Enter contact number:";
        getline(cin, permit.ContactNo);

        string sql = "UPDATE vep SET NRICPassportNo=?, Name=?, CompanyName=?, VehicleNo=?, ContactNo=? WHERE id=?";
        PreparedStatement* pstmt = conn->prepareStatement(sql);

        pstmt->setString(1, permit.NRICPassportNo);
        pstmt->setString(2, permit.Name);
        pstmt->setString(3, permit.CompanyName);
        pstmt->setString(4, permit.VehicleNo);
        pstmt->setString(5, permit.ContactNo);
        pstmt->setInt(6, id);
        pstmt->executeUpdate();

        delete pstmt;
    }
    catch (SQLException& e) {
        cout << e.what() << endl;
    }
}

static void searchVEPOMP(Connection* conn) {
    try {
        string keyword, sql;
        cout << "Enter keyword to search: ";
        getline(cin, keyword);

        sql = "SELECT * FROM vep WHERE Name LIKE ?";
        PreparedStatement* pstmt = conn->prepareStatement(sql);
        pstmt->setString(1, "%" + keyword + "%");

        ResultSet* rs = pstmt->executeQuery();

        #pragma omp parallel for
        for (int i = 0; i < rs->rowsCount(); i++) {
        //while (rs->next()) {
            Permit permit;
            rs->next();
            permit.NRICPassportNo = rs->getString("NRICPassportNo");
            permit.Name = rs->getString("Name");
            permit.CompanyName = rs->getString("CompanyName");
            permit.VehicleNo = rs->getString("VehicleNo");
            permit.ContactNo = rs->getString("ContactNo");

            cout << "ID: " << rs->getInt("id") << endl;
            cout << "Name: " << permit.Name << endl;
            cout << "NRIC/Passport No: " << permit.NRICPassportNo << endl;
            cout << "Company name: " << permit.CompanyName << endl;
            cout << "Vehicle number: " << permit.VehicleNo << endl;
            cout << "Contact number: " << permit.ContactNo << endl;
            cout << "-----------------------------------------------------" << endl;
        }

        delete rs;
        delete pstmt;
    }
    catch (SQLException& e) {
        cout << e.what() << endl;
    }
}

static void deleteVEPOMP(Connection* conn) {
    try {
        int id;
        Permit permit;

        cout << "Enter record ID to delete: ";
        cin >> id;
        cin.ignore();

        #pragma omp parallel
        {
            string sql = "SELECT * FROM vep WHERE NRICPassportNo = ?";
            PreparedStatement* pstmt = conn->prepareStatement(sql);
            pstmt->setString(1, to_string(id));
            ResultSet* rs = pstmt->executeQuery();

            if (rs->next()) {
                permit.NRICPassportNo = rs->getString("NRICPassportNo");
                permit.Name = rs->getString("Name");
                permit.CompanyName = rs->getString("CompanyName");
                permit.VehicleNo = rs->getString("VehicleNo");
                permit.ContactNo = rs->getString("ContactNo");

                cout << "Are you sure you want to delete the record with ID " << id << "? (yes/no): ";
                string response;
                cin >> response;
                cin.ignore();

                if (response == "yes") {
                    sql = "DELETE FROM vep WHERE NRICPassportNo = ?";
                    pstmt = conn->prepareStatement(sql);
                    pstmt->setString(1, to_string(id));
                    int rowsDeleted = pstmt->executeUpdate();
                    cout << rowsDeleted << " record deleted" << endl;
                }
                else {
                    cout << "Record not deleted" << endl;
                }
            }
            else {
                cout << "Record not found" << endl;
            }

            delete rs;
            delete pstmt;
        }
    }
    catch (SQLException& e) {
        cout << e.what() << endl;
    }
}

static void mainMenu() {
    cout << "Welcome to Safety and Security system" << endl;
    cout << "=====================================================" << endl;
    cout << "SNS menu" << endl;
    cout << "1. Create new VEP" << endl;
    cout << "2. Update existing record" << endl;
    cout << "3. View all records" << endl;
    cout << "4. Search VEP record" << endl;
    cout << "5. Delete existing record" << endl;
    cout << "=====================================================" << endl;
    cout << "6. Create VEP with OpenMP" << endl;
    cout << "7. Update VEP with OpenMP" << endl;
    cout << "8. View all records with OpenMP" << endl;
    cout << "9. Search VEP with OpenMP" << endl;
    cout << "10.Export data to file" << endl;
    cout << "11.Display bubble sort" << endl;
    cout << "=====================================================" << endl;
    cout << "0. Exit" << endl;
    cout << "=====================================================" << endl;
}

int main() {
    try {
        Driver* driver = get_driver_instance();
        Connection* conn = driver->connect(server, username, password);
        conn->setSchema(database);
        vector<Permit> permits = fetchPermitsFromDatabase();

        int choice;
        do {
            mainMenu();
            cout << "Enter choice: ";
            cin >> choice;
            cin.ignore();

            switch (choice) {
            case 1:
                createVEP(conn);
                break;
            case 2:
                updateVEP(conn);
                break;
            case 3:
                readAllVEP(conn);
                break;
            case 4:
                searchVEP(conn);
                break;
            case 5:
                deleteVEP(conn);
                break;
            case 6:
                createVEPOMP(conn);
                break;
            case 7:
                updateVEPOMP(conn);
                break;
            case 8:
                readAllVEPOMP(conn);
                break;
            case 9:
                searchVEPOMP(conn);
                break;
            case 10:
                exportVEP(conn);
                break;
            case 11:
                bubbleSort(permits);
                break;
            case 0:
                cout << "Exiting..." << endl;
                break;
            default:
                cout << "Invalid choice. Please try again." << endl;
                break;
            }
        } while (choice != 0);

        delete conn;
    }
    catch (SQLException& e) {
        cout << e.what() << endl;
    }
    return 0;
}
