#include "TestEnv.h"

int TestEnv::_General::benchmarkStringConversion(int argc, char* argv[]) {
    // Seed the random number generator with the current time
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    // Define the size of the array
    const uint64_t arraySize = 1e8;

    // Create an array of strings to store the random numbers
    // Allocate an array of std::string on the heap
    std::unique_ptr<std::string[]> randomNumbers(new std::string[arraySize]);
    //std::string randomNumbers[arraySize];

    // Generate and store random numbers in the array
    for (uint64_t i = 0; i < arraySize; ++i) {
        int randomNumber = std::rand();
        randomNumbers[i] = std::to_string(randomNumber);
    }

    // Convert ints
    uint64_t start_int = Time::gettime();
    for (uint64_t i = 0; i < arraySize; ++i) {
        int intValue = std::stoi(randomNumbers[i]);
    }
    uint64_t end_int = Time::gettime();

    // Convert floats
    uint64_t start_float = Time::gettime();
    for (uint64_t i = 0; i < arraySize; ++i) {
        float floatValue = std::stof(randomNumbers[i]);
    }
    uint64_t end_float = Time::gettime();

    printf(" Task ints took \t%010i ms for %i instances\n", (int)(end_int - start_int), arraySize);
    printf(" Task floats took\t%010i ms for %i instances\n\n", (int)(end_float - start_float), arraySize);
}
