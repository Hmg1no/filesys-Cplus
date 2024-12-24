/***********************************************************
 ** // UMBC - CMSC 341 - Fall 2024 - Proj4
 ** File:    filesys.cpp
 ** Project: Fall 2024 - Proj4
 ** Author:  Hazael Magino
 ** Date:    11/26/2026
 ** This file contains the proper implementations for filesys.cpp
 **********************************************************/
#include "filesys.h"

// Name: FileSys::FileSys
// Desc: Constructor for the FileSys class, initializes the hash table with a
// specified size, hash function, and probing policy parameters:
//    - size: the desired size of the current hash table
//    - hash: function pointer to the hash function
//    - probing: specifies the collision handling policy (defaults to
//    DEFPOLCY)
// Preconditions: Size must be validated and set within the range [MINPRIME,
// MAXPRIME] and adjusted to a prime number if necessary Postconditions:
//    - The hash table is created with the specified or adjusted size
//    - Member variables are initialized, including hash function and collision
//    policy
FileSys::FileSys(int size, hash_fn hash, prob_t probing = DEFPOLCY) {
  // Debug statement: Start of the constructor

  bool checkPrime = isPrime(size);
  int checkSize = size;

  // validate proper size
  if (size < MINPRIME) {
    checkSize = MINPRIME;
  } else if (size > MAXPRIME) {
    checkSize = MAXPRIME;
  } else if (!checkPrime) {
    checkSize = findNextPrime(size);
  }

  // allocate memory for new table
  m_currentTable = new File *[checkSize];

  // intialize 2D-array with nullptr since an array of pointers is being used.
  for (int i = 0; i < checkSize; i++) {
    m_currentTable[i] = nullptr;
  }

  // initialize member variables
  m_currentCap = checkSize;
  m_currentSize = 0;
  m_currNumDeleted = 0;
  m_hash = hash;
  m_currProbing = probing;

  m_newPolicy = probing;

  m_oldTable = nullptr;
  m_oldCap = 0;
  m_oldSize = 0;
  m_oldNumDeleted = 0;
  m_oldProbing = probing;

  m_transferIndex = 0;
}

// Name: FileSys::~FileSys
// Desc: Destructor for the FileSys class, deallocates dynamically allocated
// memory for the hash tables
// parameters: None
// Preconditions: The hash tables (current and old) must be dynamically
// allocated Postconditions:
//    - The dynamically allocated memory for the current hash table is
//    deallocated
//    - If the old hash table exists, its dynamically allocated memory is also
//    deallocated
FileSys::~FileSys() {

  // Cleanup current table
  if (m_currentTable != nullptr) {
    for (int i = 0; i < m_currentCap; ++i) {
      if (m_currentTable[i] != nullptr) {
        delete m_currentTable[i];
      }
    }
    delete[] m_currentTable;
    m_currentTable = nullptr;
  }

  // Cleanup old table
  cleanUpOldTable();
}

// Name: changeProbPolicy
// Desc: Changes the collision handling policy for the next rehash operation
// Parameters:
//    - policy: the new collision handling policy to be used for the next rehash
// Preconditions: None
// Postconditions:
//    - The new policy is stored in m_newPolicy to be used during the next
//    rehash
void FileSys::changeProbPolicy(prob_t policy) { m_newPolicy = policy; }

// Name: getNextIndex()
// Desc: Applies the current probing policy to find the next index in the hash
// table Parameters:
//    - index: the current index calculated by the hash function
//    - originalIndex: the original index calculated by the hash function (used
//    for quadratic and double hashing)
//    - jump: the current step count used in quadratic probing
//    - cap: the capacity of the hash table
//    - name: the name of the file (used for double hashing)
//    - table: indicates which table we are working on
// Preconditions:
//    - The hash table must be properly initialized and have a capacity cap.
//    - The current probing policy (m_currProbing) should be set to LINEAR,
//    QUADRATIC, or DOUBLEHASH.
// Postconditions:
//    - Returns the next index to check using the current probing policy.
//    - Increments the step counter if using quadratic probing.
int FileSys::getNextIndex(int index, int originalIndex, int &jump, int cap,
                          int hashVal, int table) const {

  if (table == 1) { // Indicates we are working on the current table
    switch (m_currProbing) {
    case QUADRATIC:
      return quadraticProbing(originalIndex, jump, cap);
    case DOUBLEHASH:
      return doubleHashing(hashVal, jump, cap);
    case LINEAR:
      return linearProbing(index, cap);
    default:
      // Default case to handle unexpected probing policy
      return 0; // for debugging purposes, should not happen however
    }
  } else if (table == 2) { // Indicates we are working on the old table
    switch (m_oldProbing) {
    case QUADRATIC:
      return quadraticProbing(originalIndex, jump, cap);
    case DOUBLEHASH:
      return doubleHashing(hashVal, jump, cap);
    case LINEAR:
      return linearProbing(index, cap);
    default:
      // Default case to handle unexpected probing policy
      return 0; // for debugging purposes, should not happen however
    }
  }

  return 0; // Return 0 in case neither table condition is met
}

// Name: linearProbing
// Desc: Resolves hash collisions using linear probing strategy
// Parameters:
//    - index: the current index calculated by the hash function
//    - cap: the capacity of the hash table
// Preconditions:
//    - The hash table must be properly initialized and have a capacity cap.
// Postconditions:
//    - Returns the next index to check using linear probing, ensuring it wraps
//    around if it exceeds the table size.
int FileSys::linearProbing(int index, int cap) const {
  // Increment the index by 1 and take modulo with the table capacity to ensure
  // it wraps around if it exceeds the table size
  return ((index + 1) % cap);
}

// Name: quadraticProbing
// Desc: Resolves hash collisions using quadratic probing strategy
// Parameters:
//    - orgIndex: the initial index calculated by the hash function
//    - jump: the current step count, used to calculate the quadratic offset
//    - cap: the capacity of the hash table
// Preconditions:
//    - The hash table must be properly initialized and have a capacity cap.
// Postconditions:
//    - Returns the next index to check using quadratic probing, ensuring it
//    wraps around if it exceeds the table size.
int FileSys::quadraticProbing(int orgIndex, int jump, int cap) const {
  int nextIndex = (orgIndex + (jump * jump)) % cap;
  return nextIndex;
}

// Name: doubleHashing
// Desc: Resolves hash collisions using double hashing strategy with the given
// formula Parameters:
//    - orgIndex: the initial index calculated by the first hash function
//    - hashVal: the hash value calculated by the first hash function
//    - cap: the capacity of the hash table
// Preconditions:
//    - The hash table must be properly initialized and have a capacity.
// Postconditions:
//    - Returns the next index to check using double hashing, ensuring it wraps
//    around if it exceeds the table size.
int FileSys::doubleHashing(int hashVal, int iteration, int cap) const {
  //   index = ((Hash(key) % TableSize) + i x (11-(Hash(key) % 11))) % TableSize
  //  Calculate the step size using the given formula

  int nextIndex = ((hashVal % cap) + iteration * (11 - (hashVal) % 11)) % cap;
  // Calculate the next index to check
  return nextIndex;
}
// Name: insert
// Desc: Inserts a file into the hash table using the current probing strategy
// to resolve collisions. Parameters:
//    - file: the File object to be inserted into the hash table.
// Preconditions:
//    - The file must have valid data including a name and disk block number
//    within the acceptable range.
//    - The hash table must be properly initialized and have sufficient
//    capacity.
// Postconditions:
//    - Inserts the file into the hash table if it doesn't already exist.
//    - Updates the current size of the hash table.
//    - Checks the load factor and initiates rehashing if necessary.
//    - Returns true if the insertion is successful; false if the file already
//    exists or insertion fails.
bool FileSys::insert(File file) {
  if (file.getDiskBlock() < DISKMIN || file.getDiskBlock() > DISKMAX) {
    return false;
  }

  int index = m_hash(file.getName()) % m_currentCap;
  int originalIndex = index;
  int jump = 0;

  // Iterate to find a suitable slot
  while (m_currentTable[index] != nullptr &&
         m_currentTable[index]->getUsed() == true) {
    File fileAtIndex = *m_currentTable[index];
    if (fileAtIndex.getName() == file.getName() &&
        fileAtIndex.getDiskBlock() == file.getDiskBlock()) {
      return false; // Duplicate entry, do not insert
    }
    index = getNextIndex(index, originalIndex, jump, m_currentCap,
                         originalIndex, 1);
    jump++;
  }

  // Check if we need to allocate a new File object or replace the old one
  if (m_currentTable[index] != nullptr) {
    delete m_currentTable[index]; // Delete existing file to prevent memory leak
  }

  // Allocate new File and set it in the hash table
  m_currentTable[index] = new File();
  *m_currentTable[index] = file;
  m_currentTable[index]->setUsed(true);

  if (m_currentTable[index]->getUsed() == false) {
    m_currNumDeleted--;
  } else {
    m_currentSize++;
  }

  float loadFactor = lambda();
  if (loadFactor > 0.5 && m_oldTable == nullptr) {
    int currentNumData = getNumData();
    int newCap = findNextPrime(4 * currentNumData);
    rehash(newCap);
  } else if (m_oldTable != nullptr) {
    transferData();
  }

  return true;
}

// Name: getNumData
// Desc: Returns the number of current data points in the hash table by
// subtracting the number of deleted entries from the total size. Parameters:
// None Preconditions:
//    - The hash table must be properly initialized and have a valid
//    m_currentSize and m_currNumDeleted.
// Postconditions:
//    - Returns the count of live data points (excluding deleted entries) in the
//    hash table.
int FileSys::getNumData() const { return m_currentSize - m_currNumDeleted; }

// Name: rehash
// Desc: Rehashes the hash table to a new capacity, transferring all live data
// nodes from the current table to the new table incrementally. Parameters:
//    - newCap: the new capacity for the hash table, which should be the
//    smallest prime number greater than ((m_currentSize - m_currNumDeleted) *
//    4).
// Preconditions:
//    - The hash table must be properly initialized and have a current capacity
//    m_currentCap.
//    - newCap should be a valid prime number greater than the current capacity
//    adjusted for live data points.
// Postconditions:
//    - Initiates the rehashing process, creating a new table with the specified
//    capacity.
//    - Starts transferring live data from the current table to the new table
//    incrementally.
//    - Updates the current table to the new table once the transfer is complete
//    and deallocates the old table's memory.
void FileSys::rehash(int newCap) {

  // Save the current table and its properties to old variables
  m_oldTable = m_currentTable;
  m_oldCap = m_currentCap;
  m_oldProbing = m_currProbing;
  m_oldSize = m_currentSize;
  m_oldNumDeleted = m_currNumDeleted;

  // If the current probing method is different from the new policy, update it
  if (m_currProbing != m_newPolicy) {
    m_currProbing = m_newPolicy;
  }

  // Update the capacity and create a new table with the new capacity
  m_currentCap = newCap;
  m_currentTable = new File *[m_currentCap];

  // Initialize all elements in the new table to nullptr
  for (int i = 0; i < m_currentCap; ++i) {
    m_currentTable[i] = nullptr;
  }

  // Reset the current size and number of deleted elements
  m_currentSize = 0;
  m_currNumDeleted = 0;
  m_transferIndex = 0;

  // Transfer data from the old table to the new table
  transferData();
}

// Name: transferData
// Desc: Transfers a portion of entries from the old hash table to the new hash
// table. Parameters: None Preconditions:
//    - The old table (m_oldTable) and the new table (m_currentTable) must be
//    properly initialized.
//    - m_transferIndex should be set to the correct starting index for
//    transferring data.
// Postconditions:
//    - Transfers a quarter of the entries from the old table to the new table.
//    - Updates the m_transferIndex to the next index to be transferred.
//    - Decreases m_oldSize by the number of entries transferred.
//    - If all entries have been transferred, it cleans up the old table.
void FileSys::transferData() {

  // Calculate the number of entries to transfer (1/4 of the old table's
  // capacity)
  int entriesToTransfer = floor(m_oldCap / 4);
  int transferred = 0;

  // Transfer entries from the old table to the new table
  while (transferred < entriesToTransfer && m_transferIndex < m_oldCap) {
    transferEntry(m_transferIndex); // Transfer the entry at the current index
    transferred++;     // Increment the count of transferred entries
    m_transferIndex++; // Move to the next index for the next transfer
    m_oldSize--;       // Decrease the size of the old table
  }

  // If all entries have been transferred, clean up the old table
  if (m_transferIndex >= m_oldCap) {
    cleanUpOldTable();
  }
}

// Name: transferEntry
// Desc: Transfers a single entry from the old hash table to the new hash table
// based on the given transfer index. Parameters:
//    - transferIndex: the index of the entry in the old table to be
//    transferred.
// Preconditions:
//    - The old table (m_oldTable) and the new table (m_currentTable) must be
//    properly initialized.
//    - transferIndex must be within the bounds of the old table’s capacity
//    (m_oldCap).
// Postconditions:
//    - If the entry at transferIndex is valid and used, it is transferred to
//    the new table based on its hash value.
//    - Updates the m_currentSize to reflect the addition in the new table.
//    - Sets the corresponding entry in the old table to nullptr.
void FileSys::transferEntry(int transferIndex) {
  // Check if the transfer index is out of bounds
  if (transferIndex >= m_oldCap) {
    return; // Return early if the index is beyond the old table's capacity
  }

  // Retrieve the file from the old table at the specified index
  File *oldFile = m_oldTable[transferIndex];
  // Check if the file is null or not used
  if (oldFile == nullptr || !oldFile->getUsed()) {
    return; // Return early if there's no file to transfer or it's not used
  }

  // Calculate the hash value for the file's name and determine the initial
  // index in the new table
  int hashValue = m_hash(oldFile->getName()) % m_currentCap;
  int newIndex = hashValue;
  int jump = 0;

  // Use probing to find the next available spot in the new table
  while (m_currentTable[newIndex] != nullptr &&
         m_currentTable[newIndex]->getUsed()) {
    newIndex =
        getNextIndex(newIndex, hashValue, jump, m_currentCap, hashValue, 1);
    jump++;
    // Avoid infinite loops by breaking if jump exceeds the new table capacity
    if (jump >= m_currentCap) {
      return; // Return early to prevent infinite loop
    }
  }

  // Place the file in the new table and clear the entry in the old table
  if (newIndex >= 0 && newIndex < m_currentCap) {
    if (m_currentTable[newIndex] != nullptr) {
      delete m_currentTable[newIndex]; // Prevent memory leak by deleting
                                       // existing file in new table
    }
    m_currentTable[newIndex] = oldFile;  // Move the file to the new table
    m_oldTable[transferIndex] = nullptr; // Clear the old table entry
    m_currentSize++;                     // Increment the current table size
  }
}

// Name: cleanUpOldTable
// Desc: Deallocates and cleans up the old hash table, resetting related
// properties to default values. Parameters: None Preconditions:
//    - The old table (m_oldTable) should be initialized and may contain
//    entries.
// Postconditions:
//    - Deallocates all entries in the old table and the table itself.
//    - Sets the old table pointer (m_oldTable) to nullptr.
//    - Resets transfer index, capacity, probing method, size, and number of
//    deleted entries to default values.
void FileSys::cleanUpOldTable() {

  // Check if the old table is not null
  if (m_oldTable != nullptr) {
    // Iterate through each entry in the old table
    for (int i = 0; i < m_oldCap; i++) {
      // If the entry is not null, delete the entry
      if (m_oldTable[i] != nullptr) {
        delete m_oldTable[i];
      }
    }
    // Delete the array of pointers and set the old table to null
    delete[] m_oldTable;
    m_oldTable = nullptr;
  }

  // Reset old table properties to their default values
  m_transferIndex = 0;
  m_oldCap = 0;
  m_oldProbing = DEFPOLCY; // Assuming DEFPOLCY is a predefined default policy
  m_oldSize = 0;
  m_oldNumDeleted = 0;
}

// Name: remove
// Desc: Removes the specified file from the hash table. If the file is found,
//       it marks the file as deleted, updates the current size and deleted
//       count, calculates the deletion factor, and triggers rehashing if the
//       deletion factor exceeds 80%. Also, handles incremental data transfer if
//       rehashing is in progress.
// Parameters:
//    - file: The file object to be removed from the hash table.
// Preconditions:
//    - The file system must be properly initialized and have a valid hash
//    table.
//    - The file object must have valid attributes (name, disk block).
// Postconditions:
//    - If the file is found and deleted, returns true.
//    - If the file is not found, returns false.
//    - Updates the current size and deleted count if the file is found.
//    - Triggers rehashing if the deletion factor exceeds 80% and no rehashing
//    is
//      already in progress.
//    - Handles incremental data transfer if rehashing is in progress.
bool FileSys::remove(File file) {

  // Calculate the initial index by applying the hash function to the file name
  int index = m_hash(file.getName()) % m_currentCap;
  int originalIndex = index; // Store the initial index for probing
  int jump = 0;              // Step counter for probing
  string name = file.getName();
  int block = file.getDiskBlock();

  // Search in the current table
  while (index >= 0 && index < m_currentCap &&
         m_currentTable[index] != nullptr &&
         (name != m_currentTable[index]->getName() ||
          block != m_currentTable[index]->getDiskBlock())) {
    // Update the index based on the probing method
    index = getNextIndex(index, originalIndex, jump, m_currentCap,
                         originalIndex, 1);
    jump++;
    if (jump >= m_currentCap) { // Avoid infinite loop
      return false; // Return false if probing exceeds table capacity
    }
  }

  // If file is not found in the current table, search in the old table
  if (m_currentTable[index] == nullptr || jump >= m_currentCap) {
    index = m_hash(file.getName()) %
            m_oldCap; // Recalculate index for the old table
    originalIndex = index;
    jump = 0;

    while (index >= 0 && index < m_oldCap && m_oldTable[index] != nullptr &&
           (name != m_oldTable[index]->getName() ||
            block != m_oldTable[index]->getDiskBlock())) {
      index =
          getNextIndex(index, originalIndex, jump, m_oldCap, originalIndex, 2);
      jump++;
      if (jump >= m_oldCap) { // Avoid infinite loop
        return false; // Return false if probing exceeds old table capacity
      }
    }

    // If file is not found in the old table, return false
    if (m_oldTable[index] == nullptr || jump >= m_oldCap) {
      return false;
    }

    // Mark the file as deleted in the old table
    m_oldTable[index]->setUsed(false);
    m_oldSize--;
    m_oldNumDeleted++;

  } else {
    // Mark the file as deleted in the current table
    m_currentTable[index]->setUsed(false);
    m_currentSize--;
    m_currNumDeleted++;
  }

  // Calculate the deletion factor
  float deletionFactor = deletedRatio();

  // If the deletion factor is too high and rehashing is not already in progress
  if (deletionFactor >= 0.8 && m_oldTable == nullptr) {
    int numDataPoints = getNumData();
    int newCap = findNextPrime(4 * numDataPoints); // Find a new prime capacity
    rehash(newCap);                                // Rehash the table
  }

  // If rehashing is in progress, continue transferring data
  if (m_oldTable != nullptr) {
    transferData();
  }

  return true; // Return true if the file was successfully removed
}

// Name: getFile
// Desc: Retrieves a file from the hash table that matches the given name and
// disk block. Parameters:
//    - name: the name of the file to be retrieved.
//    - block: the disk block number associated with the file.
// Preconditions:
//    - The hash table must be properly initialized and contain valid File
//    objects.
//    - The provided name and block must correspond to a valid entry in the hash
//    table.
// Postconditions:
//    - Returns the File object that matches the given name and disk block if
//    found.
//    - If no matching file is found, empty object is returned
const File FileSys::getFile(string name, int block) const {
  // Calculate the initial index using the hash function
  int index = m_hash(name) % m_currentCap;
  int originalIndex = index;
  int jump = 0;

  // Use probing to search for the file in the hash table
  while (index >= 0 && index < m_currentCap &&
         m_currentTable[index] != nullptr) {
    File *currentFile = m_currentTable[index];
    string slotName = currentFile->getName();
    int currBlock = currentFile->getDiskBlock();

    if (currentFile->getUsed() && slotName == name && currBlock == block) {
      return *currentFile;
    }

    index = getNextIndex(index, originalIndex, jump, m_currentCap,
                         originalIndex, 1);
    jump++;
  }

  // If the file is not found in the current table, check the old table
  if (m_oldTable != nullptr) {
    index = m_hash(name) % m_oldCap;
    originalIndex = index;
    jump = 0;

    while (index >= 0 && index < m_oldCap && m_oldTable[index] != nullptr) {
      File *oldFile = m_oldTable[index];
      string slotName = oldFile->getName();
      int oldBlock = oldFile->getDiskBlock();

      if (oldFile->getUsed() && slotName == name && oldBlock == block) {
        return *oldFile;
      }

      index =
          getNextIndex(index, originalIndex, jump, m_oldCap, originalIndex, 2);
      jump++;
    }
  }

  // Return an empty File object if the file is not found
  return File();
}

// Name: updateDiskBlock
// Desc: Updates the block number of a File object in the database if it exists
// Parameters:
//    - file: the File object to be updated, containing the name and current
//    block number
//    - newBlock: the new block number to be set for the File object
// Preconditions:
//    - The hash table must be properly initialized and contain the provided
//    File object
// Postconditions:
//    - If the File object is found in the database, its block number is updated
//    to newBlock, and the function returns true
//    - If the File object is not found, the function returns false
bool FileSys::updateDiskBlock(File file, int newblock) {

  // Calculate the initial index using the hash function for the current table
  int index = m_hash(file.getName()) % m_currentCap;
  int originalIndex = index;
  int jump = 0;
  File *currFile = m_currentTable[index];
  // Search the current table first

  while (currFile != nullptr) {

    if (m_currentTable[index]->getUsed() && *currFile == file) {

      // File is found, now update block number
      currFile->setDiskBlock(newblock);
      return true;
    }

    index = getNextIndex(index, originalIndex, jump, m_currentCap,
                         originalIndex, 1);
    jump++;
  }

  // If the file is not found in the current table, check the old table
  if (m_oldTable != nullptr) {
    index = m_hash(file.getName()) % m_oldCap;
    originalIndex = index;
    jump = 0;
    File *oldFile = m_oldTable[index];

    while (oldFile != nullptr) {

      if (m_oldTable[index]->getUsed() && *oldFile == file) {
        // File is found, now update block number
        m_oldTable[index]->setDiskBlock(newblock);
        return true;
      }

      index =
          getNextIndex(index, originalIndex, jump, m_oldCap, originalIndex, 2);
      jump++;
    }
  }

  // If the file is not found in either table, return false
  return false;
}

// Name: lambda
// Desc: Calculates the load factor of the hash table
// Parameters: None
// Preconditions:
//    - The hash table must be properly initialized with a defined capacity and
//    current size
// Postconditions:
//    - Returns the load factor as a floating-point number
float FileSys::lambda() const { return ((1.0 * m_currentSize) / m_currentCap); }

// Name: deletedRatio
// Desc: Calculates the ratio of deleted slots to the total number of occupied
// slots in the hash table Parameters: None Preconditions:
//    - The hash table must be properly initialized with counts of current size
//    and deleted slots
// Postconditions:
//    - Returns the deleted ratio as a floating-point number
float FileSys::deletedRatio() const {

  if (m_currentSize + m_currNumDeleted == 0) {
    return 0.0; // To avoid division by zero if there are no occupied slots
  }
  float deleted = m_currNumDeleted; // Declare a float variable
  return (deleted / m_currentSize);
}

void FileSys::dump() const {
  cout << "Dump for the current table: " << endl;
  if (m_currentTable != nullptr)
    for (int i = 0; i < m_currentCap; i++) {
      cout << "[" << i << "] : " << m_currentTable[i] << endl;
    }
  cout << "f for the old table: " << endl;
  if (m_oldTable != nullptr)
    for (int i = 0; i < m_oldCap; i++) {
      cout << "[" << i << "] : " << m_oldTable[i] << endl;
    }
}

bool FileSys::isPrime(int number) {
  bool result = true;
  for (int i = 2; i <= number / 2; ++i) {
    if (number % i == 0) {
      result = false;
      break;
    }
  }
  return result;
}

// Function to find the next prime number greater than or equal to current
int FileSys::findNextPrime(int current) {
  // we always stay within the range [MINPRIME-MAXPRIME]
  // the smallest prime starts at MINPRIME
  if (current < MINPRIME)
    current = MINPRIME - 1;
  for (int i = current; i < MAXPRIME; i++) {
    for (int j = 2; j * j <= i; j++) {
      if (i % j == 0)
        break;
      else if (j + 1 > sqrt(i) && i != current) {
        return i;
      }
    }
  }
  // if a user tries to go over MAXPRIME
  return MAXPRIME;
}