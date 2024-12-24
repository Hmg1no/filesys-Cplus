/***********************************************************
 ** // UMBC - CMSC 341 - Fall 2024 - Proj4
 ** File:    filesys.cpp
 ** Project: Fall 2024 - Proj4
 ** Author:  Hazael Magino
 ** Date:    11/26/2026
 ** This file contains the proper implementations for mytest.cpp
 **********************************************************/
#include "filesys.h"
#include <algorithm>
#include <math.h>
#include <random>
#include <vector>

using namespace std;
enum RANDOM { UNIFORMINT, UNIFORMREAL, NORMAL, SHUFFLE };
class Random {
public:
  Random() {}
  Random(int min, int max, RANDOM type = UNIFORMINT, int mean = 50,
         int stdev = 20)
      : m_min(min), m_max(max), m_type(type) {
    if (type == NORMAL) {
      // the case of NORMAL to generate integer numbers with normal distribution
      m_generator = std::mt19937(m_device());
      // the data set will have the mean of 50 (default) and standard deviation
      // of 20 (default) the mean and standard deviation can change by passing
      // new values to constructor
      m_normdist = std::normal_distribution<>(mean, stdev);
    } else if (type == UNIFORMINT) {
      // the case of UNIFORMINT to generate integer numbers
      //  Using a fixed seed value generates always the same sequence
      //  of pseudorandom numbers, e.g. reproducing scientific experiments
      //  here it helps us with testing since the same sequence repeats
      m_generator = std::mt19937(10); // 10 is the fixed seed value
      m_unidist = std::uniform_int_distribution<>(min, max);
    } else if (type == UNIFORMREAL) { // the case of UNIFORMREAL to generate
                                      // real numbers
      m_generator = std::mt19937(10); // 10 is the fixed seed value
      m_uniReal =
          std::uniform_real_distribution<double>((double)min, (double)max);
    } else { // the case of SHUFFLE to generate every number only once
      m_generator = std::mt19937(m_device());
    }
  }
  void setSeed(int seedNum) {
    // we have set a default value for seed in constructor
    // we can change the seed by calling this function after constructor call
    // this gives us more randomness
    m_generator = std::mt19937(seedNum);
  }
  void init(int min, int max) {
    m_min = min;
    m_max = max;
    m_type = UNIFORMINT;
    m_generator = std::mt19937(10); // 10 is the fixed seed value
    m_unidist = std::uniform_int_distribution<>(min, max);
  }
  void getShuffle(vector<int> &array) {
    // this function provides a list of all values between min and max
    // in a random order, this function guarantees the uniqueness
    // of every value in the list
    // the user program creates the vector param and passes here
    // here we populate the vector using m_min and m_max
    for (int i = m_min; i <= m_max; i++) {
      array.push_back(i);
    }
    shuffle(array.begin(), array.end(), m_generator);
  }

  void getShuffle(int array[]) {
    // this function provides a list of all values between min and max
    // in a random order, this function guarantees the uniqueness
    // of every value in the list
    // the param array must be of the size (m_max-m_min+1)
    // the user program creates the array and pass it here
    vector<int> temp;
    for (int i = m_min; i <= m_max; i++) {
      temp.push_back(i);
    }
    std::shuffle(temp.begin(), temp.end(), m_generator);
    vector<int>::iterator it;
    int i = 0;
    for (it = temp.begin(); it != temp.end(); it++) {
      array[i] = *it;
      i++;
    }
  }

  int getRandNum() {
    // this function returns integer numbers
    // the object must have been initialized to generate integers
    int result = 0;
    if (m_type == NORMAL) {
      // returns a random number in a set with normal distribution
      // we limit random numbers by the min and max values
      result = m_min - 1;
      while (result < m_min || result > m_max)
        result = m_normdist(m_generator);
    } else if (m_type == UNIFORMINT) {
      // this will generate a random number between min and max values
      result = m_unidist(m_generator);
    }
    return result;
  }

  double getRealRandNum() {
    // this function returns real numbers
    // the object must have been initialized to generate real numbers
    double result = m_uniReal(m_generator);
    // a trick to return numbers only with two deciaml points
    // for example if result is 15.0378, function returns 15.03
    // to round up we can use ceil function instead of floor
    result = std::floor(result * 100.0) / 100.0;
    return result;
  }

  string getRandString(int size) {
    // the parameter size specifies the length of string we ask for
    // to use ASCII char the number range in constructor must be set to 97 - 122
    // and the Random type must be UNIFORMINT (it is default in constructor)
    string output = "";
    for (int i = 0; i < size; i++) {
      output = output + (char)getRandNum();
    }
    return output;
  }

  int getMin() { return m_min; }
  int getMax() { return m_max; }

private:
  int m_min;
  int m_max;
  RANDOM m_type;
  std::random_device m_device;
  std::mt19937 m_generator;
  std::normal_distribution<> m_normdist;     // normal distribution
  std::uniform_int_distribution<> m_unidist; // integer uniform distribution
  std::uniform_real_distribution<double> m_uniReal; // real uniform distribution
};

unsigned int hashCode(const string str) {
  unsigned int val = 0;
  const unsigned int thirtyThree = 33; // magic number from textbook
  for (unsigned int i = 0; i < str.length(); i++)
    val = val * thirtyThree + str[i];
  return val;
}
enum DataSetType { NAMES_DB, NON_COLLIDE, COLLIDE };

class Tester {
public:
  FileSys generateDataSet(int filesysSize, int numdataPoints, hash_fn hash,
                          prob_t probing, DataSetType dataSetType);
  bool verifyFilePlacement(const FileSys &newSys, const vector<File> &fileList,
                           prob_t method);
  int countLiveData(File **arr, int size);
  int verifyGetNextIndex(int probeIndex, int expectedIndex, int &jump,
                         int fileCap, int hashVal, int table,
                         prob_t method) const;
  bool verifyData(const FileSys &filesys) const;
  bool checkTable(File **table, int tableCap, const File &fileInSys,
                  int genHash, int tableType, prob_t probing);
  bool isFileInCurrentOrOldTable(const FileSys &newSys, const File &fileInSys,
                                 prob_t probing);
  int verifyIndex(int probeIndex, int originalIndex, const int fileCap,
                  int hashVal, int tableNumber, prob_t method,
                  const FileSys &filesys) const;
  bool checkFileAtHash(File **table, const File &fileInSys, int hash, int cap);
  bool testforNormalNonCollide(int filesysSize, int numdataPoints, hash_fn hash,
                               prob_t probing, DataSetType dataSetType);

  bool testFindError(int filesysSize, int numdataPoints, hash_fn hash,
                     prob_t probing, DataSetType dataSetType);
  bool testFindColllide(int filesysSize, int numdataPoints, hash_fn hash,
                        prob_t probing, DataSetType dataSetType);
  void clearData();
  bool testFindNonCollide(int filesysSize, int numdataPoints, hash_fn hash,
                          prob_t probing, DataSetType dataSetType);
  bool testFindEmptyTable(int filesysSize, int numdataPoints, hash_fn hash,
                          prob_t probing, DataSetType dataSetType);
  bool testRemoveWithNonCollide(int filesysSize, int numdataPoints,
                                hash_fn hash, prob_t probing,
                                DataSetType dataSetType, int removals);
  bool testRemoveError(int filesysSize, int numdataPoints, hash_fn hash,
                       prob_t probing, DataSetType dataSetType, int removals);
  bool testRemoveWithCollideNoRehash(int filesysSize, int numdataPoints,
                                     hash_fn hash, prob_t probing,
                                     DataSetType dataSetType, int removals);
  bool testRehash(int filesysSize, int numdataPoints, hash_fn hash,
                  prob_t probing, DataSetType dataSetType);
  bool testRehashCompletion(int filesysSize, int numdataPoints, hash_fn hash,
                            prob_t probing, DataSetType dataSetType);
  bool testRehashDeletionFactor(int filesysSize, int numdataPoints,
                                hash_fn hash, prob_t probing,
                                DataSetType dataSetType, int removals);
  bool testmidRehash(int filesysSize, int numdataPoints, hash_fn hash,
                     prob_t probing, DataSetType dataSetType);
  bool testmidRehashDeletion(int filesysSize, int numdataPoints, hash_fn hash,
                             prob_t probing, DataSetType dataSetType,
                             int removals);

private:
  vector<File> m_dataList;
  vector<File> m_dataRemoved;
};

// Name: generateDataSet
// Desc: Generates a dataset of files and inserts them into a FileSys object
// based on the specified parameters. Parameters:
//    - filesysSize: the size of the FileSys object to be created.
//    - numdataPoints: the number of data points (files) to be generated and
//    inserted.
//    - hash: the hash function to be used by the FileSys object.
//    - probing: the probing technique to be used by the FileSys object.
//    - dataSetType: the type of dataset to be generated (e.g., NAMES_DB,
//    NON_COLLIDE, COLLIDE).
// Preconditions:
//    - The parameters must be valid and correspond to the expected types and
//    values.
// Postconditions:
//    - A FileSys object is created and populated with the specified number of
//    files.
//    - The files are stored in the m_dataList for later use or verification.
//    - The generated FileSys object is returned.
FileSys Tester::generateDataSet(int filesysSize, int numdataPoints,
                                hash_fn hash, prob_t probing,
                                DataSetType dataSetType) {
  // Initialize random number generator for disk block IDs
  Random RndID(DISKMIN, DISKMAX);
  // Create a FileSys object with the specified size, hash function, and probing
  // technique
  FileSys filesys(filesysSize, hash, probing);

  // Vector to hold the selected dataset based on the specified type
  vector<string> selectedDataSet;

  // Select the dataset based on the specified type
  switch (dataSetType) {
  case NAMES_DB:
    selectedDataSet = {"driver.cpp",  "test.cpp",          "test.h",
                       "info.txt",    "mydocument.docx",   "tempsheet.xlsx",
                       "report.pdf",  "presentation.pptx", "data.csv",
                       "image.png",   "notes.txt",         "summary.doc",
                       "archive.zip", "script.js",         "stylesheet.css",
                       "database.db"};
    break;
  case NON_COLLIDE:
    selectedDataSet = {"test.cpp",          "info.txt",    "script.js",
                       "notes.txt",         "image.png",   "tempsheet.xlsx",
                       "presentation.pptx", "archive.zip", "stylesheet.css",
                       "report.pdf",        "notes.txt"};
    break;
  case COLLIDE:
    selectedDataSet = {"cat", "grault", "hash1", "oscar", "baz", "eve"};
    break;
  default:
    selectedDataSet = {"driver.cpp",  "test.cpp",          "test.h",
                       "info.txt",    "mydocument.docx",   "tempsheet.xlsx",
                       "report.pdf",  "presentation.pptx", "data.csv",
                       "image.png",   "notes.txt",         "summary.doc",
                       "archive.zip", "script.js",         "stylesheet.css",
                       "database.db"};
    return filesys;
  }

  // Initialize random number generator for selecting file names from the
  // dataset
  Random RndName(0, selectedDataSet.size() - 1);

  // Generate the specified number of data points (files)
  for (int i = 0; i < numdataPoints; i++) {
    // Create a file object with a random name from the dataset and a random
    // disk block ID
    File dataObj =
        File(selectedDataSet[RndName.getRandNum()], RndID.getRandNum(), true);
    // Save the file object in the m_dataList for later use
    m_dataList.push_back(dataObj);
    // Insert the file object into the FileSys object
    filesys.insert(dataObj);
  }

  // Return the populated FileSys object
  return filesys;
}

void Tester::clearData() {

  m_dataList.clear();
  m_dataRemoved.clear();
}

// Name: verifyData
// Desc: Verifies the integrity of the data in the file system by checking
//       if the files in m_dataList are present and not deleted, and ensuring
//       that files in m_dataRemoved are not present or are marked as deleted.
// Parameters:
//    - filesys: a constant reference to the FileSys object that contains the
//    file system's state.
// Preconditions:
//    - The data lists (m_dataList and m_dataRemoved) are initialized and may
//    contain file entries.
//    - The provided FileSys object (filesys) is initialized and contains the
//    current state of the file system.
// Postconditions:
//    - Returns true if the verification of data is successful, ensuring the
//    integrity of the data in the file system.
//    - Returns false if any inconsistency is found, such as files in m_dataList
//    being marked as deleted or
//      files in m_dataRemoved being found as active in the file system.
bool Tester::verifyData(const FileSys &filesys) const {
  // Check files in m_dataList
  for (vector<File>::const_iterator it = m_dataList.begin();
       it != m_dataList.end(); ++it) {
    // Retrieve the file from the file system
    File fileInSys = filesys.getFile(it->getName(), it->getDiskBlock());
    // Check if the file is not found or marked as deleted
    if (fileInSys.getName() == "" || !fileInSys.getUsed()) {
      // If the file is not found in m_dataList or is marked as deleted, check
      // m_dataRemoved
      bool foundInRemoved = false;
      for (vector<File>::const_iterator itRemoved = m_dataRemoved.begin();
           itRemoved != m_dataRemoved.end() && !foundInRemoved; ++itRemoved) {
        // Compare the file name and disk block
        if (itRemoved->getName() == it->getName() &&
            itRemoved->getDiskBlock() == it->getDiskBlock()) {
          foundInRemoved = true; // Set flag if found in m_dataRemoved
        }
      }
      if (foundInRemoved) {
        return false; // Return false if the file is found in m_dataRemoved
      }
    }
  }

  // Check files in m_dataRemoved
  for (vector<File>::const_iterator it = m_dataRemoved.begin();
       it != m_dataRemoved.end(); ++it) {
    // Retrieve the file from the file system
    File fileInSys = filesys.getFile(it->getName(), it->getDiskBlock());
    // Check if the file in the system matches the removed file and is still
    // marked as used
    if (fileInSys.getName() == it->getName() && fileInSys.getUsed() != false) {
      return false; // Return false if the file should be removed but is still
                    // marked as used
    }
  }

  // If all checks passed, return true
  return true;
}

// Name: verifyGetNextIndex
// Desc: Determines the next index to probe in the hash table based on the
// current probing method (linear, quadratic, or double hashing). Parameters:
//    - probeIndex: the current index being probed in the hash table.
//    - originalIndex: the original hash index of the item.
//    - jump: a reference to the jump variable, which is used to calculate the
//    next index in quadratic and double hashing methods.
//    - fileCap: the capacity of the hash table.
//    - hashVal: the hash value of the item.
//    - tableNumber: the number indicating which table is being probed (if
//    applicable).
//    - method: the probing method being used (LINEAR, QUADRATIC, DOUBLEHASH).
// Preconditions:
//    - The hash table must be properly initialized and the parameters must be
//    valid.
//    - The method parameter must be a valid probing method (LINEAR, QUADRATIC,
//    DOUBLEHASH).
// Postconditions:
//    - Returns the next index to probe in the hash table based on the specified
//    probing method.
int Tester::verifyGetNextIndex(int probeIndex, int originalIndex, int &jump,
                               int fileCap, int hashVal, int tableNumber,
                               prob_t method) const {
  int nextIndex = -1;

  switch (method) {
  case LINEAR:
    nextIndex = (probeIndex + 1) % fileCap;
    break;
  case QUADRATIC:
    nextIndex = (originalIndex + (jump * jump)) % fileCap;
    break;
  case DOUBLEHASH:
    nextIndex = ((hashVal % fileCap) + jump * (11 - (hashVal % 11))) % fileCap;
    break;
  default:
    break;
  }

  return nextIndex;
}

// Name: verifyIndex
// Desc: Determines the appropriate index in the hash table using the specified
// probing method until an empty slot is found or the maximum capacity is
// reached. Parameters:
//    - probeIndex: the current index being probed in the hash table.
//    - originalIndex: the original hash index of the item.
//    - fileCap: the capacity of the hash table.
//    - hashVal: the hash value of the item.
//    - tableNumber: the number indicating which table is being probed (if
//    applicable).
//    - method: the probing method being used (LINEAR, QUADRATIC, DOUBLEHASH).
//    - filesys: a constant reference to the FileSys object that contains the
//    file system's state.
// Preconditions:
//    - The hash table (filesys) must be properly initialized and the parameters
//    must be valid.
//    - The method parameter must be a valid probing method (LINEAR, QUADRATIC,
//    DOUBLEHASH).
// Postconditions:
//    - Returns the next available index in the hash table where the item can be
//    placed based on the specified probing method.
//    - Ensures that the probing stops if an empty slot is found or the maximum
//    capacity is reached.
int Tester::verifyIndex(int probeIndex, int originalIndex, const int fileCap,
                        int hashVal, int tableNumber, prob_t method,
                        const FileSys &filesys) const {
  int index = probeIndex; // Initialize index with the current probe index
  int jump = 0; // Initialize jump counter to keep track of the number of probes

  // Loop until an empty slot is found or the maximum capacity is reached
  while (filesys.m_currentTable[index] != nullptr && jump < fileCap) {
    // Get the next index to probe based on the current probing method
    index = verifyGetNextIndex(index, originalIndex, jump, fileCap, hashVal,
                               tableNumber, method);
    jump++; // Increment the jump counter
  }

  return index; // Return the found index or the original index if no empty slot
                // is found
}

// Name: verifyFilePlacement
// Desc: Verifies the placement of files in the new hash table (FileSys) by
// checking if each file
//       in the provided file list is correctly placed in the current or old
//       table of the FileSys object.
// Parameters:
//    - newSys: a constant reference to the FileSys object containing the
//    current state of the hash table.
//    - fileList: a constant reference to a vector of File objects that need to
//    be verified in the hash table.
//    - probing: the probing method used in the hash table (LINEAR, QUADRATIC,
//    DOUBLEHASH).
// Preconditions:
//    - The FileSys object (newSys) is initialized and contains the current and
//    possibly old state of the hash table.
//    - The file list (fileList) contains valid File objects that are expected
//    to be in the hash table.
//    - The probing method (probing) must be a valid probing method used for
//    verifying the file placement.
// Postconditions:
//    - Returns true if all files in the fileList are found and correctly placed
//    in the current or old table of the FileSys object.
//    - Returns false if any file in the fileList is not found in the current or
//    old table.
bool Tester::verifyFilePlacement(const FileSys &newSys,
                                 const vector<File> &fileList, prob_t probing) {
  // Iterate through each file in the file list
  for (vector<File>::const_iterator it = fileList.begin(); it != fileList.end();
       ++it) {
    // Retrieve the file from the new system using its name and disk block
    File fileInSys = newSys.getFile(it->getName(), it->getDiskBlock());

    // Check if the file is not found in the new system (i.e., name is empty)
    if (fileInSys.getName() == "") {
      // Verify if the file is in the current or old table
      if (!isFileInCurrentOrOldTable(newSys, *it, probing)) {
        // If the file is not found, return false
        return false;
      }
    }
  }
  // If all files are verified successfully, return true
  return true;
}

// Name: isFileInCurrentOrOldTable
// Desc: Checks if the specified file is present in either the current or old
// hash table of the FileSys object. Parameters:
//    - newSys: a constant reference to the FileSys object containing the hash
//    tables.
//    - fileInSys: a constant reference to the File object to be checked.
//    - probing: the probing method used in the hash table (LINEAR, QUADRATIC,
//    DOUBLEHASH).
// Preconditions:
//    - The FileSys object (newSys) is initialized and contains the current and
//    possibly old state of the hash table.
//    - The file object (fileInSys) is a valid File with a name and disk block.
//    - The probing method (probing) must be a valid probing method used for
//    checking file placement.
// Postconditions:
//    - Returns true if the file is found in either the current or old hash
//    table.
//    - Returns false if the file is not found in both the current and old hash
//    tables.
bool Tester::isFileInCurrentOrOldTable(const FileSys &newSys,
                                       const File &fileInSys, prob_t probing) {
  // Generate the hash value for the file's name and determine the initial index
  // in the current table
  int genHash = hashCode(fileInSys.getName()) % newSys.m_currentCap;

  // Check if the file is in the current table
  bool foundInCurrent = checkTable(newSys.m_currentTable, newSys.m_currentCap,
                                   fileInSys, genHash, 1, probing);
  if (foundInCurrent) {
    return true;
  }

  // Check if the file is in the old table
  bool foundInOld = checkTable(newSys.m_oldTable, newSys.m_oldCap, fileInSys,
                               genHash, 2, probing);
  if (foundInOld) {
    return true;
  }

  // If the file is not found in either table, return false
  return false;
}

// Name: checkTable
// Desc: Checks if the specified file is present in the hash table using the
// given probing method. Parameters:
//    - table: a pointer to the hash table (array of File pointers).
//    - tableCap: the capacity of the hash table.
//    - fileInSys: a constant reference to the File object to be checked.
//    - genHash: the generated hash value for the file's name.
//    - tableType: an integer indicating which table is being checked (current
//    or old).
//    - probing: the probing method used in the hash table (LINEAR, QUADRATIC,
//    DOUBLEHASH).
// Preconditions:
//    - The hash table (table) must be properly initialized and non-null.
//    - The file object (fileInSys) is a valid File with a name and disk block.
//    - The probing method (probing) must be a valid probing method used for
//    checking file placement.
// Postconditions:
//    - Returns true if the file is found in the hash table using the specified
//    probing method.
//    - Returns false if the file is not found in the hash table after probing
//    all possible slots.
bool Tester::checkTable(File **table, int tableCap, const File &fileInSys,
                        int genHash, int tableType, prob_t probing) {

  // Check if the file is at the initial hash index
  if (checkFileAtHash(table, fileInSys, genHash, tableCap)) {
    return true;
  }

  // Initialize the index with the generated hash value
  int index = genHash;
  // Loop to probe through the table using the specified probing method
  for (int i = 1; i < tableCap; ++i) {
    // Get the next index to probe based on the current probing method
    index = verifyGetNextIndex(index, genHash, i, tableCap, genHash, tableType,
                               probing);

    // Check if the index is within bounds and the slot is not null
    if (index >= 0 && index < tableCap && table[index]) {
      // Check if the file is at the probed index
      if (checkFileAtHash(table, fileInSys, index, tableCap)) {
        return true;
      }
    } else {
      // If the index is out of bounds or the slot is null, return false
      return false;
    }
  }
  // Return false if the file is not found after probing all possible slots
  return false;
}

// Name: checkFileAtHash
// Desc: Checks if the specified file is present at the given hash index in the
// hash table. Parameters:
//    - table: a pointer to the hash table (array of File pointers).
//    - fileInSys: a constant reference to the File object to be checked.
//    - hash: the hash index to check in the hash table.
//    - cap: the capacity of the hash table.
// Preconditions:
//    - The hash table (table) must be properly initialized and non-null.
//    - The file object (fileInSys) is a valid File with a name and disk block.
//    - The hash index (hash) must be within the bounds of the hash table's
//    capacity (0 <= hash < cap).
// Postconditions:
//    - Returns true if the file is found at the specified hash index in the
//    hash table.
//    - Returns false if the file is not found at the specified hash index or
//    the slot is empty.
bool Tester::checkFileAtHash(File **table, const File &fileInSys, int hash,
                             int cap) {
  // Check if the hash index is within bounds and the slot is not null
  if (hash >= 0 && hash < cap && table[hash]) {
    File *filePtr =
        table[hash]; // Get the pointer to the file at the hash index
    const File &theFile = *filePtr; // Dereference the pointer to get the file

    // Check if the file's name and disk block match the given file
    bool nameMatches = theFile.getName() == fileInSys.getName();
    bool diskBlockMatches = theFile.getDiskBlock() == fileInSys.getDiskBlock();
    return nameMatches && diskBlockMatches; // Return true if both match
  }

  return false; // Return false if the hash index is out of bounds or the slot
                // is empty
}

// Name: testforNormalNonCollide
// Desc: Tests the FileSys object to ensure it handles normal non-colliding data
// insertion correctly. Parameters:
//    - filesysSize: the size of the FileSys object to be created.
//    - numdataPoints: the number of data points (files) to be generated and
//    inserted.
//    - hash: the hash function to be used by the FileSys object.
//    - probing: the probing technique to be used by the FileSys object (e.g.,
//    LINEAR, QUADRATIC, DOUBLEHASH).
//    - dataSetType: the type of dataset to be generated and inserted (e.g.,
//    NAMES_DB, NON_COLLIDE, COLLIDE).
// Preconditions:
//    - The parameters must be valid and correspond to the expected types and
//    values.
// Postconditions:
//    - Returns true if the FileSys object correctly handles the insertion of
//    non-colliding data and maintains data integrity.
//    - Returns false if any verification steps fail, indicating an issue with
//    data handling or placement.
bool Tester::testforNormalNonCollide(int filesysSize, int numdataPoints,
                                     hash_fn hash, prob_t probing,
                                     DataSetType dataSetType) {

  // Generate a new FileSys object and populate it with the specified number of
  // data points
  FileSys newSys =
      generateDataSet(filesysSize, numdataPoints, hash, probing, dataSetType);

  // Verify the data in the new FileSys object
  if (!verifyData(newSys)) {
    return false;
  }

  // Verify the placement of files in the new FileSys object
  bool result = verifyFilePlacement(newSys, m_dataList, probing);
  if (!result) {
    return false;
  }

  // Count the number of live data points in the current table
  int liveDataCurrent =
      countLiveData(newSys.m_currentTable, newSys.m_currentCap);

  // Compare the actual number of data points to the expected number
  int actualDataPoints = liveDataCurrent;
  if (actualDataPoints != numdataPoints) {
    return false;
  }

  // Return true if all verification steps pass
  return true;
}

// Name: countLiveData
// Desc: Counts the number of live (active) data points in the given hash table.
// Parameters:
//    - arr: a pointer to the hash table (array of File pointers).
//    - size: the capacity of the hash table.
// Preconditions:
//    - The hash table (arr) must be properly initialized and non-null.
//    - The size parameter must represent the correct capacity of the hash
//    table.
// Postconditions:
//    - Returns the number of live data points in the hash table, where a live
//    data point is one that is non-null and marked as used.

int Tester::countLiveData(File **arr, int size) {
  int count = 0; // Initialize the count of live data points

  // Iterate through each slot in the hash table
  for (int i = 0; i < size; ++i) {
    // Check if the slot is non-null and the file at the slot is marked as used
    if ((arr[i] != nullptr) && (arr[i]->getUsed() != false)) {
      count++; // Increment the count of live data points
    }
  }

  return count; // Return the total count of live data points
}

// Name: testFindError
// Desc: Tests the FileSys object to ensure it handles error cases correctly,
// specifically checking for a file
//       that does not exist in the system.
// Parameters:
//    - filesysSize: the size of the FileSys object to be created.
//    - numdataPoints: the number of data points (files) to be generated and
//    inserted.
//    - hash: the hash function to be used by the FileSys object.
//    - probing: the probing technique to be used by the FileSys object (e.g.,
//    LINEAR, QUADRATIC, DOUBLEHASH).
//    - dataSetType: the type of dataset to be generated and inserted (e.g.,
//    NAMES_DB, NON_COLLIDE, COLLIDE).
// Preconditions:
//    - The parameters must be valid and correspond to the expected types and
//    values.
// Postconditions:
//    - Returns true if the FileSys object correctly handles the error case by
//    returning an empty File object
//      for a file that does not exist in the system.
//    - Returns false if the FileSys object does not handle the error case
//    correctly.

bool Tester::testFindError(int filesysSize, int numdataPoints, hash_fn hash,
                           prob_t probing, DataSetType dataSetType) {
  // Generate a new FileSys object and populate it with the specified number of
  // data points
  FileSys newSys =
      generateDataSet(filesysSize, numdataPoints, hash, probing, dataSetType);

  // Attempt to retrieve a file that does not exist in the system
  File empty = newSys.getFile("_____", 0);

  // Check if the retrieved file is empty and not used
  if (empty.getName() == "" && empty.getDiskBlock() == 0 &&
      empty.getUsed() == false) {
    return true; // Return true if the error case is handled correctly
  }

  return false; // Return false if the error case is not handled correctly
}

// Name: testFindColllide
// Desc: Tests the FileSys object to ensure it handles data insertion and
// collisions correctly. Parameters:
//    - filesysSize: the size of the FileSys object to be created.
//    - numdataPoints: the number of data points (files) to be generated and
//    inserted.
//    - hash: the hash function to be used by the FileSys object.
//    - probing: the probing technique to be used by the FileSys object (e.g.,
//    LINEAR, QUADRATIC, DOUBLEHASH).
//    - dataSetType: the type of dataset to be generated and inserted (e.g.,
//    NAMES_DB, NON_COLLIDE, COLLIDE).
// Preconditions:
//    - The parameters must be valid and correspond to the expected types and
//    values.
// Postconditions:
//    - Returns true if the FileSys object correctly handles data insertion,
//    collision resolution, and maintains data integrity.
//    - Returns false if any verification steps fail, indicating an issue with
//    data handling or placement.
bool Tester::testFindColllide(int filesysSize, int numdataPoints, hash_fn hash,
                              prob_t probing, DataSetType dataSetType) {

  // Generate a new FileSys object and populate it with the specified number of
  // data points
  FileSys newSys =
      generateDataSet(filesysSize, numdataPoints, hash, probing, dataSetType);

  // Verify the placement of files in the new FileSys object
  bool result = verifyFilePlacement(newSys, m_dataList, probing);
  if (!result) {
    return false; // Return false if file placement verification fails
  }

  // Verify the data in the new FileSys object
  result = verifyData(newSys);
  if (!result) {
    return false; // Return false if data verification fails
  }

  // Count the number of live data points in the current table
  int liveDataCurrent =
      countLiveData(newSys.m_currentTable, newSys.m_currentCap);

  // Compare the actual number of data points to the expected number
  int actualDataPoints = liveDataCurrent;
  if (actualDataPoints != numdataPoints) {
    return false; // Return false if the actual data points do not match the
                  // expected number
  }

  return true; // Return true if all verification steps pass
}

// Name: testFindNonCollide
// Desc: Tests the FileSys object to ensure it handles data insertion without
// collisions correctly. Parameters:
//    - filesysSize: the size of the FileSys object to be created.
//    - numdataPoints: the number of data points (files) to be generated and
//    inserted.
//    - hash: the hash function to be used by the FileSys object.
//    - probing: the probing technique to be used by the FileSys object (e.g.,
//    LINEAR, QUADRATIC, DOUBLEHASH).
//    - dataSetType: the type of dataset to be generated and inserted (e.g.,
//    NAMES_DB, NON_COLLIDE, COLLIDE).
// Preconditions:
//    - The parameters must be valid and correspond to the expected types and
//    values.
// Postconditions:
//    - Returns true if the FileSys object correctly handles data insertion
//    without collisions and maintains data integrity.
//    - Returns false if any verification steps fail, indicating an issue with
//    data handling or placement.
bool Tester::testFindNonCollide(int filesysSize, int numdataPoints,
                                hash_fn hash, prob_t probing,
                                DataSetType dataSetType) {

  // Generate a new FileSys object and populate it with the specified number of
  // data points
  FileSys newSys =
      generateDataSet(filesysSize, numdataPoints, hash, probing, dataSetType);

  // Verify the placement of files in the new FileSys object
  bool result = verifyFilePlacement(newSys, m_dataList, probing);
  if (!result) {
    return false; // Return false if file placement verification fails
  }

  // Verify the data in the new FileSys object
  result = verifyData(newSys);
  if (!result) {
    return false; // Return false if data verification fails
  }

  // Count the number of live data points in the current table
  int liveDataCurrent =
      countLiveData(newSys.m_currentTable, newSys.m_currentCap);

  // Compare the actual number of data points to the expected number
  int actualDataPoints = liveDataCurrent;
  if (actualDataPoints != numdataPoints) {
    return false; // Return false if the actual data points do not match the
                  // expected number
  }

  return true; // Return true if all verification steps pass
}

// Name: testRemoveWithNonCollide
// Desc: Tests the FileSys object to ensure it handles data removal without
// collisions correctly. Parameters:
//    - filesysSize: the size of the FileSys object to be created.
//    - numdataPoints: the number of data points (files) to be generated and
//    inserted.
//    - hash: the hash function to be used by the FileSys object.
//    - probing: the probing technique to be used by the FileSys object (e.g.,
//    LINEAR, QUADRATIC, DOUBLEHASH).
//    - dataSetType: the type of dataset to be generated and inserted (e.g.,
//    NAMES_DB, NON_COLLIDE, COLLIDE).
//    - removals: the number of data points (files) to be removed.
// Preconditions:
//    - The parameters must be valid and correspond to the expected types and
//    values.
// Postconditions:
//    - Returns true if the FileSys object correctly handles data removal
//    without collisions and maintains data integrity.
//    - Returns false if any verification steps fail, indicating an issue with
//    data handling or placement.

bool Tester::testRemoveWithNonCollide(int filesysSize, int numdataPoints,
                                      hash_fn hash, prob_t probing,
                                      DataSetType dataSetType, int removals) {
  // Generate a new FileSys object and populate it with the specified number of
  // data points
  FileSys newSys =
      generateDataSet(filesysSize, numdataPoints, hash, probing, dataSetType);

  // Verify the placement of files in the new FileSys object
  bool result = verifyFilePlacement(newSys, m_dataList, probing);
  if (!result) {
    return false; // Return false if file placement verification fails
  }

  // Verify the data in the new FileSys object
  result = verifyData(newSys);
  if (!result) {
    return false; // Return false if data verification fails
  }

  // Prepare the list of files to be removed
  int numRemovals = removals;
  vector<File> filesToRemove;
  for (int i = 0; i < numRemovals; i++) {
    filesToRemove.push_back(m_dataList[i]);
  }

  // Remove the files from the FileSys object
  for (vector<File>::const_iterator it = filesToRemove.begin();
       it != filesToRemove.end(); ++it) {
    File file = *it;

    bool removed = newSys.remove(file);
    if (!removed) {
      return false; // Return false if file removal fails
    }

    // Remove the file from m_dataList and add it to m_dataRemoved
    vector<File>::iterator deletion =
        remove(m_dataList.begin(), m_dataList.end(), file);
    m_dataList.erase(deletion, m_dataList.end());
    m_dataRemoved.push_back(file);
  }

  // Verify the data again in the new FileSys object after removals
  result = verifyData(newSys);
  if (!result) {
    return false; // Return false if data verification fails after removals
  }

  // Count the number of live data points in the current table
  int liveDataCurrent =
      countLiveData(newSys.m_currentTable, newSys.m_currentCap);
  int actualDataPoints = numdataPoints - numRemovals;

  // Compare the actual number of live data points to the expected number after
  // removals
  if (liveDataCurrent != actualDataPoints) {
    return false; // Return false if the actual data points do not match the
                  // expected number after removals
  }

  return true; // Return true if all verification steps pass
}
// Name: testRemoveError
// Desc: Tests the FileSys object to ensure it handles error cases correctly
// during the removal process,
//       specifically checking the removal of nonexistent files and default file
//       objects.
// Parameters:
//    - filesysSize: the size of the FileSys object to be created.
//    - numdataPoints: the number of data points (files) to be generated and
//    inserted.
//    - hash: the hash function to be used by the FileSys object.
//    - probing: the probing technique to be used by the FileSys object (e.g.,
//    LINEAR, QUADRATIC, DOUBLEHASH).
//    - dataSetType: the type of dataset to be generated and inserted (e.g.,
//    NAMES_DB, NON_COLLIDE, COLLIDE).
//    - removals: the number of data points (files) to be removed (not used in
//    this function).
// Preconditions:
//    - The parameters must be valid and correspond to the expected types and
//    values.
// Postconditions:
//    - Returns true if the FileSys object correctly handles the removal error
//    cases, including the removal of nonexistent files.
//    - Returns false if any verification steps fail, indicating an issue with
//    data handling or placement during removal.

bool Tester::testRemoveError(int filesysSize, int numdataPoints, hash_fn hash,
                             prob_t probing, DataSetType dataSetType,
                             int removals) {
  // Generate a new FileSys object and populate it with the specified number of
  // data points
  FileSys newSys =
      generateDataSet(filesysSize, numdataPoints, hash, probing, dataSetType);

  // Verify the placement of files in the new FileSys object
  bool result = verifyFilePlacement(newSys, m_dataList, probing);
  if (!result) {
    return false; // Return false if file placement verification fails
  }

  // Verify the data in the new FileSys object
  result = verifyData(newSys);
  if (!result) {
    return false; // Return false if data verification fails
  }

  // Attempt to remove a nonexistent file from the FileSys object
  File nonexistentFile("nonexistent_file.txt", 123456);
  bool valid = newSys.remove(nonexistentFile);
  if (valid) {
    return false; // Return false if the removal of a nonexistent file is
                  // incorrectly handled
  }

  // Attempt to remove a default file object from the FileSys object
  valid = newSys.remove(File());
  if (valid) {
    return false; // Return false if the removal of a default file object is
                  // incorrectly handled
  }

  // Verify the data again in the new FileSys object after error removals
  result = verifyData(newSys);
  if (!result) {
    return false; // Return false if data verification fails after error
                  // removals
  }

  // Count the number of live data points in the current table
  int liveDataCurrent =
      countLiveData(newSys.m_currentTable, newSys.m_currentCap);
  int actualDataPoints = numdataPoints;

  // Compare the actual number of live data points to the expected number
  if (liveDataCurrent != actualDataPoints) {
    return false; // Return false if the actual data points do not match the
                  // expected number
  }

  return true; // Return true if all verification steps pass
}

// Name: testRemoveWithCollideNoRehash
// Desc: Tests the FileSys object to ensure it handles data removal correctly in
// the presence of collisions without rehashing. Parameters:
//    - filesysSize: the size of the FileSys object to be created.
//    - numdataPoints: the number of data points (files) to be generated and
//    inserted.
//    - hash: the hash function to be used by the FileSys object.
//    - probing: the probing technique to be used by the FileSys object (e.g.,
//    LINEAR, QUADRATIC, DOUBLEHASH).
//    - dataSetType: the type of dataset to be generated and inserted (e.g.,
//    NAMES_DB, NON_COLLIDE, COLLIDE).
//    - removals: the number of data points (files) to be removed.
// Preconditions:
//    - The parameters must be valid and correspond to the expected types and
//    values.
// Postconditions:
//    - Returns true if the FileSys object correctly handles data removal with
//    collisions and maintains data integrity.
//    - Returns false if any verification steps fail, indicating an issue with
//    data handling or placement during removal.
bool Tester::testRemoveWithCollideNoRehash(int filesysSize, int numdataPoints,
                                           hash_fn hash, prob_t probing,
                                           DataSetType dataSetType,
                                           int removals) {
  // Generate a new FileSys object and populate it with the specified number of
  // data points
  FileSys newSys =
      generateDataSet(filesysSize, numdataPoints, hash, probing, dataSetType);

  // Verify the placement of files in the new FileSys object
  bool result = verifyFilePlacement(newSys, m_dataList, probing);
  if (!result) {
    return false; // Return false if file placement verification fails
  }

  // Verify the data in the new FileSys object
  result = verifyData(newSys);
  if (!result) {
    return false; // Return false if data verification fails
  }

  // Prepare the list of files to be removed
  int numRemovals = removals;
  vector<File> filesToRemove;
  for (int i = 0; i < numRemovals; i++) {
    filesToRemove.push_back(m_dataList[i]);
  }

  // Remove the files from the FileSys object
  for (vector<File>::const_iterator it = filesToRemove.begin();
       it != filesToRemove.end(); ++it) {
    File file = *it;

    bool removed = newSys.remove(file);
    if (!removed) {
      return false; // Return false if file removal fails
    }

    // Remove the file from m_dataList and add it to m_dataRemoved
    vector<File>::iterator deletion =
        remove(m_dataList.begin(), m_dataList.end(), file);
    m_dataList.erase(deletion, m_dataList.end());
    m_dataRemoved.push_back(file);
  }

  // Verify the data again in the new FileSys object after removals
  result = verifyData(newSys);
  if (!result) {
    return false; // Return false if data verification fails after removals
  }

  // Count the number of live data points in the current table
  int liveDataCurrent =
      countLiveData(newSys.m_currentTable, newSys.m_currentCap);
  int actualDataPoints = numdataPoints - numRemovals;

  // Compare the actual number of live data points to the expected number after
  // removals
  if (liveDataCurrent != actualDataPoints) {
    return false; // Return false if the actual data points do not match the
                  // expected number after removals
  }

  return true; // Return true if all verification steps pass
}

// Name: testRehash
// Desc: Tests the FileSys object to ensure it handles rehashing correctly by
// inserting additional data points
//       and verifying the integrity of the data after rehashing.
// Parameters:
//    - filesysSize: the size of the FileSys object to be created.
//    - numdataPoints: the number of data points (files) to be generated and
//    inserted.
//    - hash: the hash function to be used by the FileSys object.
//    - probing: the probing technique to be used by the FileSys object (e.g.,
//    LINEAR, QUADRATIC, DOUBLEHASH).
//    - dataSetType: the type of dataset to be generated and inserted (e.g.,
//    NAMES_DB, NON_COLLIDE, COLLIDE).
// Preconditions:
//    - The parameters must be valid and correspond to the expected types and
//    values.
// Postconditions:
//    - Returns true if the FileSys object correctly handles rehashing and
//    maintains data integrity.
//    - Returns false if any verification steps fail, indicating an issue with
//    data handling or rehashing.

bool Tester::testRehash(int filesysSize, int numdataPoints, hash_fn hash,
                        prob_t probing, DataSetType dataSetType) {
  // Generate a new FileSys object and populate it with the specified number of
  // data points
  FileSys newSys =
      generateDataSet(filesysSize, numdataPoints, hash, probing, dataSetType);

  // Verify the placement of files in the new FileSys object
  bool result = verifyFilePlacement(newSys, m_dataList, probing);
  if (!result) {
    return false; // Return false if file placement verification fails
  }

  // Verify the data in the new FileSys object
  result = verifyData(newSys);
  if (!result) {
    return false; // Return false if data verification fails
  }

  // Prepare the new dataset type and selected dataset for additional inserts
  int newdataSetType = 2;
  vector<string> selectedDataSet;
  switch (newdataSetType) {
  case NAMES_DB:
    selectedDataSet = {"driver.cpp",  "test.cpp",          "test.h",
                       "info.txt",    "mydocument.docx",   "tempsheet.xlsx",
                       "report.pdf",  "presentation.pptx", "data.csv",
                       "image.png",   "notes.txt",         "summary.doc",
                       "archive.zip", "script.js",         "stylesheet.css",
                       "database.db"};
    break;
  case NON_COLLIDE:
    selectedDataSet = {"test.cpp",          "info.txt",    "script.js",
                       "notes.txt",         "image.png",   "tempsheet.xlsx",
                       "presentation.pptx", "archive.zip", "stylesheet.css",
                       "report.pdf",        "notes.txt"};
    break;
  case COLLIDE:
    selectedDataSet = {"cat", "grault", "hash1", "oscar", "baz", "eve"};
    break;
  default:
    selectedDataSet = {"driver.cpp",  "test.cpp",          "test.h",
                       "info.txt",    "mydocument.docx",   "tempsheet.xlsx",
                       "report.pdf",  "presentation.pptx", "data.csv",
                       "image.png",   "notes.txt",         "summary.doc",
                       "archive.zip", "script.js",         "stylesheet.css",
                       "database.db"};
  }

  Random RndID(DISKMIN, DISKMAX); // Random generator for disk block IDs
  Random RndName(0, selectedDataSet.size() -
                        1); // Random generator for file names from the dataset

  int newInserts = numdataPoints / 2; // Number of new inserts
  for (int i = 0; i < newInserts; i++) {
    // Generating random data
    File dataObj =
        File(selectedDataSet[RndName.getRandNum()], RndID.getRandNum(), true);
    // Save data for later use
    m_dataList.push_back(dataObj);
    // Inserting data into the FileSys object
    newSys.insert(dataObj);
  }

  // Verify if rehashing has occurred by checking the new capacity
  if (newSys.m_currentCap <= filesysSize) {
    return false; // Return false if rehashing has not occurred
  }

  // Count the number of live data points in the current table
  int liveDataCurrent =
      countLiveData(newSys.m_currentTable, newSys.m_currentCap);

  // Compare the actual number of data points to the expected number after
  // rehashing
  int actualDataPoints = liveDataCurrent;
  int newNumData = numdataPoints + newInserts;
  if (actualDataPoints != newNumData) {
    return false; // Return false if the actual data points do not match the
                  // expected number after rehashing
  }

  return true; // Return true if all verification steps pass
}

// Name: testRehashDeletionFactor
// Desc: Tests the FileSys object to ensure it handles data removal and
// maintains a proper deletion factor
//       to trigger rehashing based on the specified parameters and conditions.
// Parameters:
//    - filesysSize: the size of the FileSys object to be created.
//    - numdataPoints: the number of data points (files) to be generated and
//    inserted.
//    - hash: the hash function to be used by the FileSys object.
//    - probing: the probing technique to be used by the FileSys object (e.g.,
//    LINEAR, QUADRATIC, DOUBLEHASH).
//    - dataSetType: the type of dataset to be generated and inserted (e.g.,
//    NAMES_DB, NON_COLLIDE, COLLIDE).
//    - removals: the number of data points (files) to be removed to test the
//    deletion factor.
// Preconditions:
//    - The parameters must be valid and correspond to the expected types and
//    values.
// Postconditions:
//    - Returns true if the FileSys object correctly handles data removal and
//    maintains the expected deletion factor.
//    - Returns false if any verification steps fail, indicating an issue with
//    data handling, placement, or deletion factor.

bool Tester::testRehashDeletionFactor(int filesysSize, int numdataPoints,
                                      hash_fn hash, prob_t probing,
                                      DataSetType dataSetType, int removals) {
  // Generate a new FileSys object and populate it with the specified number of
  // data points
  FileSys newSys =
      generateDataSet(filesysSize, numdataPoints, hash, probing, dataSetType);

  // Verify the placement of files in the new FileSys object
  bool result = verifyFilePlacement(newSys, m_dataList, probing);
  if (!result) {
    return false; // Return false if file placement verification fails
  }

  // Verify the data in the new FileSys object
  result = verifyData(newSys);
  if (!result) {
    return false; // Return false if data verification fails
  }

  // Prepare the list of files to be removed
  int numRemovals = removals;
  vector<File> filesToRemove;
  for (int i = 0; i < numRemovals; i++) {
    filesToRemove.push_back(m_dataList[i]);
  }

  // Remove the files from the FileSys object
  for (vector<File>::const_iterator it = filesToRemove.begin();
       it != filesToRemove.end(); ++it) {
    File file = *it;

    bool removed = newSys.remove(file);
    if (!removed) {
      return false; // Return false if file removal fails
    }

    // Remove the file from m_dataList and add it to m_dataRemoved
    vector<File>::iterator deletion =
        remove(m_dataList.begin(), m_dataList.end(), file);
    m_dataList.erase(deletion, m_dataList.end());
    m_dataRemoved.push_back(file);
  }

  // Verify the data again in the new FileSys object after removals
  result = verifyData(newSys);
  if (!result) {
    return false; // Return false if data verification fails after removals
  }

  // Count the number of live data points in the current table
  int liveDataCurrent =
      countLiveData(newSys.m_currentTable, newSys.m_currentCap);
  int actualDataPoints = numdataPoints - numRemovals;

  // Compare the actual number of live data points to the expected number after
  // removals
  if (liveDataCurrent != actualDataPoints) {
    return false; // Return false if the actual data points do not match the
                  // expected number after removals
  }

  return true; // Return true if all verification steps pass
}

// Name: testmidRehash
// Desc: Tests the FileSys object to ensure it can handle rehashing correctly by
// inserting additional data points
//       and verifying the integrity of the data after rehashing has been
//       triggered.
// Parameters:
//    - filesysSize: the size of the FileSys object to be created.
//    - numdataPoints: the number of data points (files) to be generated and
//    inserted.
//    - hash: the hash function to be used by the FileSys object.
//    - probing: the probing technique to be used by the FileSys object (e.g.,
//    LINEAR, QUADRATIC, DOUBLEHASH).
//    - dataSetType: the type of dataset to be generated and inserted (e.g.,
//    NAMES_DB, NON_COLLIDE, COLLIDE).
// Preconditions:
//    - The parameters must be valid and correspond to the expected types and
//    values.
// Postconditions:
//    - Returns true if the FileSys object correctly handles rehashing and
//    maintains data integrity.
//    - Returns false if any verification steps fail, indicating an issue with
//    data handling, placement, or rehashing.

bool Tester::testmidRehash(int filesysSize, int numdataPoints, hash_fn hash,
                           prob_t probing, DataSetType dataSetType) {
  // Generate a new FileSys object and populate it with the specified number of
  // data points
  FileSys newSys =
      generateDataSet(filesysSize, numdataPoints, hash, probing, dataSetType);

  // Verify the placement of files in the new FileSys object
  bool result = verifyFilePlacement(newSys, m_dataList, probing);
  if (!result) {
    return false; // Return false if file placement verification fails
  }

  // Verify the data in the new FileSys object
  result = verifyData(newSys);
  if (!result) {
    return false; // Return false if data verification fails
  }

  // Prepare the new dataset type and selected dataset for additional inserts
  int newdataSetType = 2;
  vector<string> selectedDataSet;
  switch (newdataSetType) {
  case NAMES_DB:
    selectedDataSet = {"driver.cpp",  "test.cpp",          "test.h",
                       "info.txt",    "mydocument.docx",   "tempsheet.xlsx",
                       "report.pdf",  "presentation.pptx", "data.csv",
                       "image.png",   "notes.txt",         "summary.doc",
                       "archive.zip", "script.js",         "stylesheet.css",
                       "database.db"};
    break;
  case NON_COLLIDE:
    selectedDataSet = {"test.cpp",          "info.txt",    "script.js",
                       "notes.txt",         "image.png",   "tempsheet.xlsx",
                       "presentation.pptx", "archive.zip", "stylesheet.css",
                       "report.pdf",        "notes.txt"};
    break;
  case COLLIDE:
    selectedDataSet = {"cat", "grault", "hash1", "oscar", "baz", "eve"};
    break;
  default:
    selectedDataSet = {"driver.cpp",  "test.cpp",          "test.h",
                       "info.txt",    "mydocument.docx",   "tempsheet.xlsx",
                       "report.pdf",  "presentation.pptx", "data.csv",
                       "image.png",   "notes.txt",         "summary.doc",
                       "archive.zip", "script.js",         "stylesheet.css",
                       "database.db"};
  }

  Random RndID(DISKMIN, DISKMAX); // Random generator for disk block IDs
  Random RndName(0, selectedDataSet.size() -
                        1); // Random generator for file names from the dataset

  int newInserts = 2; // Number of new inserts to trigger rehashing
  for (int i = 0; i < newInserts; i++) {
    // Generating random data
    File dataObj =
        File(selectedDataSet[RndName.getRandNum()], RndID.getRandNum(), true);
    // Save data for later use
    m_dataList.push_back(dataObj);
    // Inserting data into the FileSys object
    newSys.insert(dataObj);
  }

  // Verify if rehashing has occurred by checking if the old table is not null
  if (newSys.m_oldTable != nullptr) {
    return true; // Return true if rehashing has occurred
  }

  return false; // Return false if rehashing has not occurred
}

// Name: testmidRehashDeletion
// Desc: Tests the FileSys object to ensure it handles rehashing correctly
// during the deletion process
//       by removing a specified number of data points and verifying the
//       integrity of the data after rehashing.
// Parameters:
//    - filesysSize: the size of the FileSys object to be created.
//    - numdataPoints: the number of data points (files) to be generated and
//    inserted.
//    - hash: the hash function to be used by the FileSys object.
//    - probing: the probing technique to be used by the FileSys object (e.g.,
//    LINEAR, QUADRATIC, DOUBLEHASH).
//    - dataSetType: the type of dataset to be generated and inserted (e.g.,
//    NAMES_DB, NON_COLLIDE, COLLIDE).
//    - removals: the number of data points (files) to be removed.
// Preconditions:
//    - The parameters must be valid and correspond to the expected types and
//    values.
// Postconditions:
//    - Returns true if the FileSys object correctly handles rehashing and
//    maintains data integrity during the deletion process.
//    - Returns false if any verification steps fail, indicating an issue with
//    data handling, placement, or rehashing.

bool Tester::testmidRehashDeletion(int filesysSize, int numdataPoints,
                                   hash_fn hash, prob_t probing,
                                   DataSetType dataSetType, int removals) {
  // Generate a new FileSys object and populate it with the specified number of
  // data points
  FileSys newSys =
      generateDataSet(filesysSize, numdataPoints, hash, probing, dataSetType);

  // Verify the placement of files in the new FileSys object
  bool result = verifyFilePlacement(newSys, m_dataList, probing);
  if (!result) {
    return false; // Return false if file placement verification fails
  }

  // Verify the data in the new FileSys object
  result = verifyData(newSys);
  if (!result) {
    return false; // Return false if data verification fails
  }

  // Prepare the list of files to be removed
  int numRemovals = removals;
  vector<File> filesToRemove;
  for (int i = 0; i < numRemovals; i++) {
    filesToRemove.push_back(m_dataList[i]);
  }

  // Remove the files from the FileSys object
  for (vector<File>::const_iterator it = filesToRemove.begin();
       it != filesToRemove.end(); ++it) {
    File file = *it;

    bool removed = newSys.remove(file);
    if (!removed) {
      return false; // Return false if file removal fails
    }

    // Remove the file from m_dataList and add it to m_dataRemoved
    vector<File>::iterator deletion =
        remove(m_dataList.begin(), m_dataList.end(), file);
    m_dataList.erase(deletion, m_dataList.end());
    m_dataRemoved.push_back(file);
  }

  // Verify the data again in the new FileSys object after removals
  result = verifyData(newSys);
  if (!result) {
    return false; // Return false if data verification fails after removals
  }

  // Check if the old table still exists
  if (newSys.m_oldTable != nullptr) {
    return true; // Return true if the old table still exists (indicating
                 // rehashing has occurred)
  }

  return false; // Return false if the old table does not exist (indicating
                // rehashing has not occurred)
}

int main() {
  Tester aTester;

  cout << "Testing for Normal Non Collision Handling" << endl;

  if (aTester.testforNormalNonCollide(70, 25, hashCode, QUADRATIC,
                                      NON_COLLIDE)) {
    cout << "Testing test for Non Collision case passed!" << endl;

  } else {
    cout << "Testing for Non Collision Handling case failed!" << endl;
  }
  aTester.clearData();
  cout << "Testing an Error case for getFile method Handling" << endl;

  if (aTester.testFindError(200, 50, hashCode, DOUBLEHASH, NAMES_DB)) {
    cout << "Testing an Error case for getFile method Handling passed !"
         << endl;
  } else {
    cout << "Testing an Error case for getFile method case failed!" << endl;
  }
  aTester.clearData();
  cout << "Testing a Normal case for getFile method Handling (Non-Collide)"
       << endl;
  if (aTester.testFindNonCollide(200, 50, hashCode, LINEAR, NON_COLLIDE)) {
    cout << "Testing a Normal case for getFile method Handling passed !"
         << endl;
  } else {
    cout << "Testing a Normal case for getFile method case failed!" << endl;
  }
  aTester.clearData();
  cout << "Testing a Normal case for getFile method Handling (Collide)" << endl;

  if (aTester.testFindColllide(101, 20, hashCode, QUADRATIC, COLLIDE)) {
    cout << "Testing a Normal case for getFile method Handling passed !"
         << endl;
  } else {
    cout << "Testing a Normal case for getFile method case failed!" << endl;
  }

  aTester.clearData();
  cout << "Testing a Normal case for remove method Handling (Non-colliding)"
       << endl;

  if (aTester.testRemoveWithNonCollide(101, 30, hashCode, DOUBLEHASH,
                                       NON_COLLIDE, 10)) {
    cout << "Testing a Normal case for remove method Handling passed !" << endl;
  } else {
    cout << "Testing a Normal case for remove method Handling failed!" << endl;
  }
  aTester.clearData();
  cout << "Testing error case for remove method " << endl;

  if (aTester.testRemoveWithNonCollide(400, 100, hashCode, LINEAR, NAMES_DB,
                                       10)) {
    cout << "Testing error case passed !" << endl;
  } else {
    cout << "Testing error case failed!" << endl;
  }
  aTester.clearData();
  cout << "Testing Normal case for remove method (Collide)" << endl;

  if (aTester.testRemoveWithCollideNoRehash(400, 100, hashCode, LINEAR, COLLIDE,
                                            10)) {
    cout << "Testing Normal case for remove passed !" << endl;
  } else {
    cout << "Testing Normal case for remove failed!" << endl;
  }

  aTester.clearData();
  cout << "Testing Normal case of triggering rehash method  " << endl;

  if (aTester.testmidRehash(101, 49, hashCode, QUADRATIC, NAMES_DB)) {
    cout << "Testing Normal case of  triggering rehash method passed !" << endl;
  } else {
    cout << "Testing Normal case of triggering rehash method failed!" << endl;
  }
  aTester.clearData();
  cout << "Testing Normal case for rehash method with Completion" << endl;

  if (aTester.testRehash(101, 55, hashCode, QUADRATIC, NAMES_DB)) {
    cout << "Testing Normal case for rehash method passed !" << endl;
  } else {
    cout << "Testing Normal case for rehash method failed!" << endl;
  }

  aTester.clearData();
  cout << "Testing Normal case of triggering rehash method with Deletion "
          "factor > 0.8 "
       << endl;
  if (aTester.testmidRehashDeletion(101, 48, hashCode, LINEAR, COLLIDE, 35)) {
    cout << "Testing Normal case for rehash method passed !" << endl;
  } else {
    cout << "Testing Normal case for rehash method failed!" << endl;
  }
  aTester.clearData();

  cout << "Testing Normal case of rehash method with Deletion factor > 0.8 "
          "with Completion "
       << endl;
  if (aTester.testRehashDeletionFactor(101, 47, hashCode, QUADRATIC, NAMES_DB,
                                       39)) {
    cout << "Testing Normal case for rehash method passed !" << endl;
  } else {
    cout << "Testing Normal case for rehash method failed!" << endl;
  }
  return 0;
}
