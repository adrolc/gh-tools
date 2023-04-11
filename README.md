# Documentation

## ProcessMemoryManager

`ProcessMemoryManager` is a class designed for managing the memory of processes running on the Windows operating system. It enables opening processes based on their process identifiers (ID) or names, reading and writing values in the process memory, and performing operations on addresses with offsets.

The main methods of the `ProcessMemoryManager` class include:

* `OpenProcessById` - opens a process based on the provided process ID.

* `OpenProcessByName` - opens a process based on the provided process name.

* `IsProcessHandleValid` - checks if the process handle is valid.

* `ReadValue` - reads a value from the provided memory address.

* `WriteValue` - writes a value to the provided memory address.

* `GetModuleBaseAddress` - retrieves the base address of a module based on the provided module name.

* `ReadValueWithOffsets` - reads a value from an address with offsets.

* `WriteValueWithOffsets` - writes a value to an address with offsets.

* `GetAddressWithOffsets` - calculates the target address, considering the offsets.

Example of use:

```cpp
int main() {
	ProcessMemoryManager procmem;
	uintptr_t moduleBaseAddress;
    
	try {
		procmem.OpenProcessByName(L"example.exe");
		moduleBaseAddress = procmem.GetModuleBaseAddress(L"example.exe");
	}
	catch (const std::runtime_error& e) {
		std::cerr << "Exception: " << e.what() << std::endl;
		return 1;
	}

	// Offsets leading to the gold address in the game's memory
	std::vector<uintptr_t> goldOffsets = { 0x015DC2A0, 0x10, 0xF8, 0x00, 0x08 };

	// Find and read the value of the player's gold in the game
	int goldValue = procmem.ReadValueWithOffsets<int>(moduleBaseAddress, goldOffsets);
	std::cout << "Read value: " << goldValue << std::endl;

	// Set a new value for the player's gold
	int newValue = 100;
	if (procmem.WriteValueWithOffsets<int>(moduleBaseAddress, goldOffsets, newValue)) {
		std::cout << "New value (" << newValue << ") has been wirtten." << std::endl;
	}
	else {
		std::cout << "Failed to wirte new value" << std::endl;
	}

	// Find the address of the player object in the game's memory
	std::vector<uintptr_t> playerBaseOffsets = { 0x024CB1A1, 0x30, 0xA1 };
	uintptr_t playerBaseAddress = procmem.GetAddressWithOffsets(moduleBaseAddress, playerBaseOffsets);

	// Find the player's health address within the player object
	std::vector<uintptr_t> playerHealthOffsets = { 0x32 };
	uintptr_t playerHealthAddress = procmem.GetAddressWithOffsets(playerBaseAddress, playerHealthOffsets);

	// Read the player's current health value
	int healthValue = procmem.ReadValue<int>(playerHealthAddress);
	std::cout << "Read value: " << healthValue << std::endl;

	// Overwrite the player's health value with a new value
	int newValue = 100;
	if (procmem.WriteValue<int>(playerHealthAddress, newValue)) {
		std::cout << "New value (" << newValue << ") has been wirtten." << std::endl;
	}
	else {
		std::cout << "Failed to wirte new value" << std::endl;
	}

	return 0;
}
```