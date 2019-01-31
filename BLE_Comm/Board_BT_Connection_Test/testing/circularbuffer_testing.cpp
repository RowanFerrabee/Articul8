#include <iostream>
#include "circularbuffer.h"
#include <string>
#include <exception>

void my_assert(bool condition, std::string msg) {

	if (condition != true)
	{
		std::cout << "Error: " << msg << std::endl;
		throw std::exception();
	}
}

void garbage_test() {

	std::cout << "Starting garbage test" << std::endl;

	CircularBuffer<4*PACKET_SIZE> testBuffer;

	char garbage_str[] = "garbage";
	int garbage_len = std::strlen(garbage_str);

	testBuffer.write((unsigned char *)garbage_str, garbage_len);

	my_assert(testBuffer.getSize() == garbage_len, "Wrong buffer size");

	testBuffer.read((unsigned char *)garbage_str, garbage_len);

	my_assert(strcmp(garbage_str, "garbage") == 0, "Garbage not returned");
}

void single_package_test() {

	std::cout << "Starting single package test" << std::endl;

	CircularBuffer<4*PACKET_SIZE> testBuffer;

	unsigned char pkg_data[] = " 000000000000000000000000000000AAAAAAAA ";
	pkg_data[POS_SOP] = (unsigned char)SOP;

	unsigned char checksum = 0;
	for (int i = 1; i < PACKET_SIZE - 1; i++) {
		checksum += (unsigned char)pkg_data[i];
	}

	pkg_data[POS_CHECKSUM] = checksum;

	testBuffer.write(pkg_data, PACKET_SIZE);

	my_assert(testBuffer.findPacket() == BUFFER_SUCCESS, "Couldnt find single packet");

	testBuffer.readPacket(pkg_data);

	my_assert(testBuffer.findPacket() != BUFFER_SUCCESS, "Found packet when shouldnt have");
}

void multi_package_test() {

	std::cout << "Starting multi package test" << std::endl;

	const int max_num_packets = 4;
	CircularBuffer<max_num_packets*PACKET_SIZE> testBuffer;

	for (int num_packets = 3; num_packets <= 5; num_packets++) {

		std::cout << "Testing write " << num_packets << " packets then read all" << std::endl;

		unsigned char pkg_data[] = " 000000000000000000000000000000AAAAAAAA ";
		pkg_data[POS_SOP] = (unsigned char)SOP;

		unsigned char checksum = 0;
		for (int i = 1; i < PACKET_SIZE - 1; i++) {
			checksum += (unsigned char)pkg_data[i];
		}

		pkg_data[POS_CHECKSUM] = checksum;

		for (int i = 0; i < num_packets; i++) {

			testBuffer.write(pkg_data, PACKET_SIZE);
		}

		for (int i = 0; i < num_packets && i < max_num_packets; i++) {

			my_assert(testBuffer.findPacket() == BUFFER_SUCCESS, "Couldnt find packet");
			testBuffer.readPacket(pkg_data);
		}

		my_assert(testBuffer.findPacket() != BUFFER_SUCCESS, "Found packet when shouldnt have");
	}

	for (int num_packets = 3; num_packets <= 5; num_packets++) {

		std::cout << "Testing write then read " << num_packets << " packets" << std::endl;

		for (int i = 0; i < num_packets; i++) {

			unsigned char pkg_data[] = " 000000000000000000000000000000AAAAAAAA ";
			pkg_data[POS_SOP] = (unsigned char)SOP;

			unsigned char checksum = 0;
			for (int i = 1; i < PACKET_SIZE - 1; i++) {
				checksum += (unsigned char)pkg_data[i];
			}

			pkg_data[POS_CHECKSUM] = checksum;

			testBuffer.write(pkg_data, PACKET_SIZE);
			my_assert(testBuffer.findPacket() == BUFFER_SUCCESS, "Couldnt find packet");
			testBuffer.readPacket(pkg_data);
		}

		my_assert(testBuffer.findPacket() != BUFFER_SUCCESS, "Found packet when shouldnt have");
	}
}

void multi_package_test_with_garbage() {

	std::cout << "Starting multi package test with garbage" << std::endl;

	const int max_num_packets = 4;
	CircularBuffer<max_num_packets*PACKET_SIZE> testBuffer;

	for (int i = 0; i < 5; i++) {

		char garbage_str[] = "garbage";
		int garbage_len = std::strlen(garbage_str);

		testBuffer.write((unsigned char *)garbage_str, garbage_len);

		unsigned char pkg_data[] = " 000000000000000000000000000000AAAAAAAA ";
		pkg_data[POS_SOP] = (unsigned char)SOP;

		unsigned char checksum = 0;
		for (int i = 1; i < PACKET_SIZE - 1; i++) {
			checksum += (unsigned char)pkg_data[i];
		}

		pkg_data[POS_CHECKSUM] = checksum;

		testBuffer.write(pkg_data, PACKET_SIZE);
		testBuffer.write(pkg_data, PACKET_SIZE);
		my_assert(testBuffer.findPacket() == BUFFER_SUCCESS, "Couldnt find packet");
		testBuffer.readPacket(pkg_data);
		my_assert(testBuffer.findPacket() == BUFFER_SUCCESS, "Couldnt find packet");
		testBuffer.readPacket(pkg_data);

		my_assert(testBuffer.getSize() == 0, "Buffer not empty after only packet is read");
	}
}

void run_all_tests() {

	garbage_test();
	single_package_test();
	multi_package_test();
	multi_package_test_with_garbage();
}

int main() {

	try {
		run_all_tests();
	}
	catch (const std::exception&)
	{
		std::cout << "Failed to pass all tests" << std::endl;
		return EXIT_FAILURE;
	}

	std::cout << "Passed all tests!" << std::endl;
	return EXIT_SUCCESS;
}