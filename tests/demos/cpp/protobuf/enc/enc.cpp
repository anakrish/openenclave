// Copyright (c) Open Enclave SDK contributors.
// Licensed under the MIT License.

#include <openenclave/enclave.h>
#include "demo_t.h"

// For mounting host filesystem
#include <openenclave/internal/syscall/device.h>
#include <openenclave/internal/syscall/sys/mount.h>
#include <openenclave/internal/syscall/unistd.h>
#include <openenclave/internal/tests.h>
#include <sys/mount.h>

#include <iostream>
#include <fstream>
#include <string>

#include "addressbook.pb.h"
using namespace std;

// This function fills in a Person message based on user input.
void PromptForAddress(tutorial::Person* person) {
    cout << "Enter person ID number: ";
    int id;
    cin >> id;
    person->set_id(id);
    cin.ignore(256, '\n');

    cout << "Enter name: ";
    getline(cin, *person->mutable_name());

    cout << "Enter email address (blank for none): ";
    string email;
    getline(cin, email);
    if (!email.empty()) {
	person->set_email(email);
    }

    while (true) {
	cout << "Enter a phone number (or leave blank to finish): ";
	string number;
	getline(cin, number);
	if (number.empty()) {
	    break;
	}

	tutorial::Person::PhoneNumber* phone_number = person->add_phones();
	phone_number->set_number(number);

	cout << "Is this a mobile, home, or work phone? ";
	string type;
	getline(cin, type);
	if (type == "mobile") {
	    phone_number->set_type(tutorial::Person::MOBILE);
	} else if (type == "home") {
	    phone_number->set_type(tutorial::Person::HOME);
	} else if (type == "work") {
	    phone_number->set_type(tutorial::Person::WORK);
	} else {
	    cout << "Unknown phone type.  Using default." << endl;
	}
    }
}

int main_fcn_write(int argc, const char** argv)
{
    // Verify that the version of the library that we linked against is
    // compatible with the version of the headers we compiled against.
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    if (argc != 2) {
	cerr << "Usage:  " << argv[0] << " ADDRESS_BOOK_FILE" << endl;
	return -1;
    }

    tutorial::AddressBook address_book;

    {
	// Read the existing address book.
	fstream input(argv[1], ios::in | ios::binary);
	if (!input) {
	    cout << argv[1] << ": File not found.  Creating a new file." << endl;
	} else if (!address_book.ParseFromIstream(&input)) {
	    cerr << "Failed to parse address book." << endl;
	    return -1;
	}
    }

    // Add an address.
    PromptForAddress(address_book.add_people());

    {
	// Write the new address book back to disk.
	fstream output(argv[1], ios::out | ios::trunc | ios::binary);
	if (!output)
	    cerr << "Could not create output stream." << endl;
	
	if (!address_book.SerializeToOstream(&output)) {
	    cerr << "Failed to write address book." << endl;
	    return -1;
	}
    }

    // Optional:  Delete all global objects allocated by libprotobuf.
    google::protobuf::ShutdownProtobufLibrary();

    return 0;
}

// Iterates though all people in the AddressBook and prints info about them.
void ListPeople(const tutorial::AddressBook& address_book) {
    for (int i = 0; i < address_book.people_size(); i++) {
	const tutorial::Person& person = address_book.people(i);

	cout << "Person ID: " << person.id() << endl;
	cout << "  Name: " << person.name() << endl;
	if (person.has_email()) {
	    cout << "  E-mail address: " << person.email() << endl;
	}

	for (int j = 0; j < person.phones_size(); j++) {
	    const tutorial::Person::PhoneNumber& phone_number = person.phones(j);

	    switch (phone_number.type()) {
	    case tutorial::Person::MOBILE:
		cout << "  Mobile phone #: ";
		break;
	    case tutorial::Person::HOME:
		cout << "  Home phone #: ";
		break;
	    case tutorial::Person::WORK:
		cout << "  Work phone #: ";
		break;
	    }
	    cout << phone_number.number() << endl;
	}
    }
}

// Main function:  Reads the entire address book from a file and prints all
//   the information inside.
int main_fcn_read(int argc, const char* argv[]) {
    // Verify that the version of the library that we linked against is
    // compatible with the version of the headers we compiled against.
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    if (argc != 3) {
	cerr << "Usage:  " << argv[0] << " ADDRESS_BOOK_FILE read" << endl;
	return -1;
    }

    tutorial::AddressBook address_book;

    {
	// Read the existing address book.
	fstream input(argv[1], ios::in | ios::binary);
	if (!address_book.ParseFromIstream(&input)) {
	    cerr << "Failed to parse address book." << endl;
	    return -1;
	}
    }

    ListPeople(address_book);

    // Optional:  Delete all global objects allocated by libprotobuf.
    google::protobuf::ShutdownProtobufLibrary();

    return 0;
}

int enc_main(int argc, const char** argv)
{
    int ret = -1;
    bool mounted = false;

    OE_TEST(oe_load_module_host_file_system() == OE_OK);
    OE_TEST(oe_mount("/", "/", OE_DEVICE_NAME_HOST_FILE_SYSTEM, 0, NULL) == 0);
    mounted = true;

    if (argc == 2)
	ret = main_fcn_write(argc, argv);
    else
	ret = main_fcn_read(argc, argv);
	
    if (mounted)
        oe_umount("/");

    return ret;
}

OE_SET_ENCLAVE_SGX(
    1,    /* ProductID */
    1,    /* SecurityVersion */
    true, /* Debug */
    1024, /* NumHeapPages */
    1024, /* NumStackPages */
    2);   /* NumTCS */
